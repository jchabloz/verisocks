from verisocks.verisocks import Verisocks, VerisocksError
from verisocks.utils import setup_sim_run, find_free_port
import os.path
import pytest
import logging
import re
import socket

# Note
# To run with coverage, use
# coverage run --source=../verisocks --branch -m pytest
# coverage report or coverage html


# Parameters
HOST = socket.gethostbyname("localhost")
VS_TIMEOUT = 10


# Expectations
sim_info_product = "Verilator"
sim_info_version = r"[0-9]+\.[0-9]+ 20[0-9]{2}-[0-1][0-9]-[0-3][0-9]"


cwd = os.path.dirname(__file__)


def setup_test(port, timeout, capture_output=True,
               capture_logfile=None):
    elab_cmd = ["make", "-C", cwd]
    sim_cmd = [
        os.path.join(cwd, "Vmain"),
        f"{port}",
        f"{timeout}"
    ]
    pop = setup_sim_run(
        elab_cmd,
        sim_cmd,
        capture_output=capture_output,
        capture_logfile=capture_logfile
    )
    return pop


@pytest.fixture
def vs():
    # Setup
    port = find_free_port()
    #flog = open("vsl.log", "w")
    #pop = setup_test(port, VS_TIMEOUT, False, flog)
    pop = setup_test(port, VS_TIMEOUT)
    _vs = Verisocks(HOST, port)
    _vs.connect()
    yield _vs
    # Teardown
    try:
        _vs.finish()
    except ConnectionError:
        logging.warning("Connection error - Cannot send finish command")
    _vs.close()
    pop.communicate(timeout=10)
    #flog.close()


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
    port = find_free_port()
    with pytest.raises(ConnectionError):
        vs = Verisocks(HOST, port)
        vs.connect(trials=1)
    vs.close()


def test_info(vs):
    """Tests the info command"""
    answer = vs.info("This is a test")
    assert answer["type"] == "ack"


def test_get_sim_info(vs):
    answer = vs.get("sim_info")
    assert answer["type"] == "result"
    assert answer["product"] == sim_info_product
    assert re.match(sim_info_version, answer["version"])


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
    answer = vs.get("value", path="main.int_param")
    assert answer["type"] == "result"
    assert answer["value"] == 598402

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


#def test_get_type(vs):
#    """Tests Verisocks get(sel="type") function"""
#
#    # Get an integer parameter type
#    answer = vs.get("type", path="main.int_param")
#    assert answer["type"] == "result"
#    assert answer["vpi_type"] == 41  # vpiParameter (see vpi_user.h)
#
#    # Get a real parameter type
#    answer = vs.get(sel="type", path="main.fclk")
#    assert answer["type"] == "result"
#    assert answer["vpi_type"] == 41  # vpiParameter (see vpi_user.h)
#
#    # Get a reg type
#    answer = vs.get(sel="type", path="main.clk")
#    assert answer["type"] == "result"
#    assert answer["vpi_type"] == 48  # vpiReg (see vpi_user.h)
#
#    # Get a reg[] type
#    answer = vs.get(sel="type", path="main.count")
#    assert answer["type"] == "result"
#    assert answer["vpi_type"] == 48  # vpiReg (see vpi_user.h)
#
#    # Get a memory array type
#    answer = vs.get(sel="type", path="main.count_memory")
#    assert answer["type"] == "result"
#    assert answer["vpi_type"] == 29  # vpiMemory (see vpi_user.h)
#
#    # Error case: wrong path
#    with pytest.raises(VerisocksError):
#        answer = vs.get(sel="type", path="wrong_path")
#        assert answer["type"] == "error"


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

#    answer = vs.set(path="main.count_memory[6]", value=37)
#    assert answer["type"] == "ack"
#    answer = vs.get(sel="value", path="main.count_memory")
#    assert answer["type"] == "result"
#    assert answer["value"][6] == 37

    # Set an event
    answer = vs.set(path="main.counter_end")
    assert answer["type"] == "ack"


def test_read_not_expected(vs):
    """Tests what happens when a read function is requested while there are no
    messages expected.
    """
    assert vs.read() is False


#def test_sim_finishes(vs):
#    """Tests the case where the simulation runs out before the specified
#    callback event occurs
#    """
#    with pytest.raises(VerisocksError):
#        _ = vs.run(cb="until_time", time=1200, time_unit="us")


def test_exit():
    port = find_free_port()
    setup_test(port, VS_TIMEOUT)
    with Verisocks(HOST, port) as vs:
        answer = vs.exit()
        assert answer["type"] == "ack"


def test_context_manager():
    port = find_free_port()
    pop = setup_test(port, VS_TIMEOUT)
    with Verisocks(HOST, port) as vs:
        assert vs._connected
        answer = vs.finish()
        assert answer["type"] == "ack"

    pop.wait(timeout=10)
