from verisocks.verisocks import Verisocks
import subprocess
import os.path
import time
import shutil

# Parameters
HOST = "127.0.0.1"
PORT = 5100
LIBVPI = "../../build/libvpi.so"
CONNECT_DELAY = 0.01

# Expectations
sim_info_product = "Icarus Verilog"
sim_info_version = "11.0 (stable)"


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


def test_get_siminfo():
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
        answer = vs.finish()
        assert answer["type"] == "ack"
    retcode = pop.wait(timeout=120)
    assert retcode == 0
