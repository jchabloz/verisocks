from verisocks.verisocks import Verisocks, VerisocksError
import subprocess
import os.path
import time
import shutil
import pytest

# Parameters
HOST = "127.0.0.1"
PORT = 5100
LIBVPI = "../../build/libvpi.so"  # Relative path to this file!
CONNECT_DELAY = 0.01

# Expectations
sim_info_product = "Icarus Verilog"
sim_info_version = "11.0 (stable)"


# @pytest.fixture
# def pop():
#     _pop = setup_iverilog("test_0.v")
#     yield _pop
#     _pop.wait(timeout=120)


# @pytest.fixture
# def vs(pop):
#     assert pop.poll() is None
#     _vs = Verisocks(HOST, PORT)
#     _vs.connect()
#     yield _vs
#     _vs.finish()def test_run_for_time():
#     _vs.close()


def get_abspath(relpath):
    return os.path.join(os.path.dirname(__file__), relpath)


def setup_iverilog(src_file):
    """Elaborate and run the verilog testbench file provided as an argument

    Args:
        * src_file (str): Path to source file

    Returns:
        * pop: Popen instance for spawned process
    """
    src_file_path = get_abspath(src_file)
    if not os.path.isfile(src_file_path):
        raise FileNotFoundError
    src_name = os.path.splitext(os.path.basename(src_file))[0]
    vvp_file_path = get_abspath(f"{src_name}.vvp")
    cmd = [
        shutil.which("iverilog"),
        "-o", vvp_file_path,
        "-Wall",
        src_file_path
    ]
    subprocess.check_call(cmd)
    libvpi_path = get_abspath(LIBVPI)
    cmd = [shutil.which("vvp"), "-m", libvpi_path, vvp_file_path]
    pop = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print(f"Launched Icarus with PID {pop.pid}")
    # Some delay is required for Icarus to launch the Verisocks server before
    # being able to connect - Please adjust CONNECT_DELAY if required.

    time.sleep(CONNECT_DELAY)
    return pop


def test_connect():
    """Tests simple connection to Icarus simulator with Verisocks server"""
    pop = setup_iverilog("test_0.v")
    assert pop.poll() is None
    with Verisocks(HOST, PORT) as vs:
        assert vs._connected
        answer = vs.finish()
        assert answer["type"] == "ack"
    retcode = pop.wait(timeout=120)
    assert retcode == 0

    # Verify error case - Here Icarus is not running anymore
    with pytest.raises(ConnectionRefusedError):
        with Verisocks(HOST, PORT) as vs:
            pass


def test_get_siminfo():
    """Tests Verisocks get(sel=sim_info) function"""
    pop = setup_iverilog("test_0.v")
    with Verisocks(HOST, PORT) as vs:
        answer = vs.get(sel="sim_info")
        assert answer["type"] == "result"
        assert answer["product"] == sim_info_product
        assert answer["version"] == sim_info_version
        answer = vs.finish()
        assert answer["type"] == "ack"
    retcode = pop.wait(timeout=120)
    assert retcode == 0


def test_get_sim_time():
    """Tests Verisocks get(sel=sim_time) function"""
    # Setup
    pop = setup_iverilog("test_0.v")
    with Verisocks(HOST, PORT) as vs:

        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] == 0.0
        answer = vs.run(cb="until_time", time=101.3, time_unit="us")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] == 101.3e-6

        # Teardown
        answer = vs.finish()
        assert answer["type"] == "ack"
    retcode = pop.wait(timeout=120)
    assert retcode == 0


def test_get_value():
    """Tests Verisocks get(sel="value") function"""

    # Setup
    pop = setup_iverilog("test_0.v")
    with Verisocks(HOST, PORT) as vs:

        # Get an integer parameter value
        answer = vs.get(sel="value", path="main.num_port")
        assert answer["type"] == "result"
        assert answer["value"] == 5100

        # Get a real parameter value
        answer = vs.get(sel="value", path="main.fclk")
        assert answer["type"] == "result"
        assert answer["value"] == 1.01

        # Get a reg value
        answer = vs.run(cb="for_time", time=100, time_unit="us")
        assert answer["type"] == "ack"
        answer = vs.get(sel="value", path="main.clk")
        assert answer["type"] == "result"
        assert answer["value"] == 1

        # Get a reg[] value
        answer = vs.get(sel="value", path="main.count")
        assert answer["type"] == "result"
        assert answer["value"] == 101

        # Get a memory array value
        answer = vs.get(sel="value", path="main.count_memory")
        assert answer["type"] == "result"
        assert answer["value"] == [
            96, 97, 98, 99, 100, 85, 86, 87,
            88, 89, 90, 91, 92, 93, 94, 95]

        # Error case: wrong path
        with pytest.raises(VerisocksError):
            answer = vs.get(sel="value", path="wrong_path")
            assert answer["type"] == "error"

        # Teardown
        answer = vs.finish()
        assert answer["type"] == "ack"
    retcode = pop.wait(timeout=120)
    assert retcode == 0


def test_run_for_time():
    # Setup
    pop = setup_iverilog("test_0.v")
    with Verisocks(HOST, PORT) as vs:

        # Get initial time
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        prev_sim_time = answer["time"]

        # For time in ps
        answer = vs.run(cb="for_time", time=6, time_unit="ps")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] - prev_sim_time == pytest.approx(6e-12)
        prev_sim_time = answer["time"]

        # For time in ns
        answer = vs.run(cb="for_time", time=7, time_unit="ns")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] - prev_sim_time == pytest.approx(7e-9)
        prev_sim_time = answer["time"]

        # For time in us
        answer = vs.run(cb="for_time", time=8, time_unit="us")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] - prev_sim_time == pytest.approx(8e-6)
        prev_sim_time = answer["time"]

        # For time in ms
        answer = vs.run(cb="for_time", time=0.23, time_unit="ms")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] - prev_sim_time == pytest.approx(0.23e-3)
        prev_sim_time = answer["time"]

        # For time in s
        answer = vs.run(cb="for_time", time=0.00017, time_unit="s")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] - prev_sim_time == pytest.approx(0.17e-3)
        prev_sim_time = answer["time"]

        # Teardown
        answer = vs.finish()
        assert answer["type"] == "ack"
    retcode = pop.wait(timeout=120)
    assert retcode == 0


def test_run_until_time():
    # Setup
    pop = setup_iverilog("test_0.v")
    with Verisocks(HOST, PORT) as vs:

        # Until time in ps
        answer = vs.run(cb="until_time", time=174, time_unit="ps")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] == pytest.approx(174e-12)

        # Until time in ns
        answer = vs.run(cb="until_time", time=215.4, time_unit="ns")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] == pytest.approx(215.4e-9)

        # Until time in us
        answer = vs.run(cb="until_time", time=1.593, time_unit="us")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] == pytest.approx(1.593e-6)

        # Until time in ms
        answer = vs.run(cb="until_time", time=0.456, time_unit="ms")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] == pytest.approx(0.456e-3)

        # Until time in s
        answer = vs.run(cb="until_time", time=0.6e-3, time_unit="s")
        assert answer["type"] == "ack"
        answer = vs.get(sel="sim_time")
        assert answer["type"] == "result"
        assert answer["time"] == pytest.approx(0.6e-3)

        # Error: until time in the past
        with pytest.raises(VerisocksError):
            answer = vs.run(cb="until_time", time=0.4e-3, time_unit="s")
            assert answer["type"] == "error"

        # Teardown
        answer = vs.finish()
        assert answer["type"] == "ack"
    retcode = pop.wait(timeout=120)
    assert retcode == 0
