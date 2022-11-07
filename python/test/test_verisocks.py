from verisocks.verisocks import Verisocks, VerisocksError
import subprocess
import os.path
import time
import shutil
import pytest
import logging

# Note
# To run with coverage, use
# coverage run --source=../verisocks --branch -m pytest
# coverage report or coverage html

# Parameters
HOST = "127.0.0.1"
PORT = 5100
LIBVPI = "../../build/verisocks.vpi"  # Relative path to this file!
CONNECT_DELAY = 0.01

# Expectations
sim_info_product = "Icarus Verilog"
sim_info_version = "11.0 (stable)"


@pytest.fixture
def vs():
    # Setup
    pop = setup_iverilog("test_0.v")
    _vs = Verisocks(HOST, PORT)
    _vs.connect()
    yield _vs
    # Teardown
    _vs.finish()
    _vs.close()
    pop.wait(timeout=10)


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


def test_connect(vs):
    """Tests simple connection to Icarus simulator with Verisocks server"""
    assert vs._connected


def test_already_connected(vs, caplog):
    caplog.set_level(logging.INFO)
    caplog.clear()
    vs.connect()
    log_record = caplog.record_tuples[0]
    assert log_record[1] == logging.INFO
    assert log_record[2] == "Socket already connected"


def test_connect_error():
    """Tests trying to connect to a non-running server"""
    with pytest.raises(ConnectionRefusedError):
        vs = Verisocks(HOST, PORT)
        vs.connect()
    vs.close()


def test_get_sim_info(vs):
    answer = vs.get("sim_info")
    assert answer["type"] == "result"
    assert answer["product"] == sim_info_product
    assert answer["version"] == sim_info_version


def test_get_sim_time(vs):
    """Tests Verisocks get(sel=sim_time) function"""
    answer = vs.get("sim_time")
    assert answer["type"] == "result"
    assert answer["time"] == 0.0

    answer = vs.run("until_time", time=101.3, time_unit="us")
    assert answer["type"] == "ack"

    answer = vs.get("sim_time")
    assert answer["type"] == "result"
    assert answer["time"] == 101.3e-6


def test_get_value(vs):
    """Tests Verisocks get(sel="value") function"""

    # Get an integer parameter value
    answer = vs.get("value", path="main.num_port")
    assert answer["type"] == "result"
    assert answer["value"] == 5100

    # Get a real parameter value
    answer = vs.get(sel="value", path="main.fclk")
    assert answer["type"] == "result"
    assert answer["value"] == 1.01

    # Get a reg value
    answer = vs.run("for_time", time=100, time_unit="us")
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


def test_run_for_time(vs):
    """Tests Verisocks run(cb="for_time") function"""

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


def test_run_until_time(vs):
    """Tests Verisocks run(cb="until_time") function"""

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


def test_run_until_change(vs):
    """Tests Verisocks run(cb="until_change") function"""

    answer = vs.run(cb="until_change", path="main.count", value=137)
    assert answer["type"] == "ack"
    answer = vs.get(sel="value", path="main.count")
    assert answer["type"] == "result"
    assert answer["value"] == 137

    answer = vs.run(cb="until_change", path="main.counter_end")
    assert answer["type"] == "ack"
    answer = vs.get(sel="value", path="main.count")
    assert answer["type"] == "result"
    assert answer["value"] == 255


def test_set(vs):
    """Tests Verisocks set() function"""
    # Set a reg
    answer = vs.run(cb="until_time", time=10, time_unit="us")
    assert answer["type"] == "ack"
    answer = vs.set(path="main.count", value=125)
    assert answer["type"] == "ack"
    answer = vs.get(sel="value", path="main.count")
    assert answer["type"] == "result"
    assert answer["value"] == 125

    # Set a memory array
    answer = vs.set(path="main.count_memory", value=list(range(16)))
    assert answer["type"] == "ack"
    answer = vs.get(sel="value", path="main.count_memory")
    assert answer["type"] == "result"
    assert answer["value"] == list(range(16))

    answer = vs.set(path="main.count_memory[6]", value=37)
    assert answer["type"] == "ack"
    answer = vs.get(sel="value", path="main.count_memory")
    assert answer["type"] == "result"
    assert answer["value"][6] == 37

    # Set an event
    answer = vs.set(path="main.counter_end")
    assert answer["type"] == "ack"


def test_read_not_expected(vs):
    # No data expected
    assert vs.read() is False


def test_context_manager():
    pop = setup_iverilog("test_0.v")
    with Verisocks(HOST, PORT) as vs:
        assert vs._connected
        answer = vs.finish()
        assert answer["type"] == "ack"

    pop.wait(timeout=10)
