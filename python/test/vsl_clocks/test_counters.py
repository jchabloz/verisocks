from verisocks.verisocks import Verisocks
from verisocks.utils import setup_sim_run, find_free_port
import logging
import pytest
import socket
from os.path import join, dirname, abspath, relpath

# Parameters
HOST = socket.gethostbyname("localhost")
TIMEOUT = 10
cwd = relpath(dirname(abspath(__file__)))


def setup_test(port=5100, timeout=10, capture_output=True):
    elab_cmd = ["make", "-C", cwd]
    sim_cmd = [
        join(cwd, "build", "Vcounters"),
        "-p", f"{port}",
        "-t", f"{timeout}"
    ]
    pop = setup_sim_run(elab_cmd, sim_cmd, capture_output=capture_output)
    return pop


@pytest.fixture
def vs():
    # Set up simulation and launch it as a separate process
    port = find_free_port()
    setup_test(port, TIMEOUT)
    _vs = Verisocks(HOST, port)
    _vs.connect()
    yield _vs
    # Teardown
    try:
        _vs.finish()
    except ConnectionError:
        logging.warning("Connection error - Finish command not possible")
    _vs.close()


def reset(vs, wait_us=10):
    vs.set("arstb", value=0)
    vs.run("for_time", time=1, time_unit="us")
    vs.set("arstb", value=1)
    vs.run("for_time", time=wait_us, time_unit="us")


def get_value(vs, path):
    answer = vs.get(sel="value", path=path)
    assert answer['type'] == "result"
    return answer['value']


def test_get_clocks(vs):

    answer = vs.get(sel="clocks")
    assert answer['type'] == "result"
    clocks = answer['value']
    assert len(clocks) == 2
    assert clocks[0]['name'] == "clk1"
    assert clocks[0]['period'] == 1_400_000
    assert clocks[0]['duty_cycle'] == 0.4
    assert clocks[1]['name'] == "clk2"
    assert clocks[1]['period'] == 20_000_000
    assert clocks[1]['duty_cycle'] == 0.6


def test_clk_config(vs):

    reset(vs, 100)

    # Check configured initial values for clk1
    answer = vs.get(sel="value", path="clk1_period")
    assert answer['value'] == 1400
    answer = vs.get(sel="value", path="clk1_dc")
    assert answer['value'] == 0.4

    # Check configured initial values for clk2
    answer = vs.get(sel="value", path="clk2_period")
    assert answer['value'] == 20000
    answer = vs.get(sel="value", path="clk2_dc")
    assert answer['value'] == 0.6

    # Test configuring clock
    vs.configure_clock("clk1", 1.6, "us", 0.56)
    vs.run("for_time", time=10, time_unit="us")
    answer = vs.get(sel="value", path="clk1_period")
    assert answer['value'] == 1600
    answer = vs.get(sel="value", path="clk1_dc")
    assert answer['value'] == 0.56


def test_clk_enable_disable(vs):

    reset(vs, 100)
    # Both clocks are supposed to be enabled at the start
    # This has already been somehow verified in the previous test_clk_config
    c1 = get_value(vs, "clk1_count")
    c2 = get_value(vs, "clk2_count")

    vs.disable_clock("clk1")
    vs.run("for_time", time=40, time_unit="us")
    c1_new = get_value(vs, "clk1_count")
    c2_new = get_value(vs, "clk2_count")
    assert c1_new == c1
    assert c2_new > c2
    c1 = c1_new
    c2 = c2_new

    vs.disable_clock("clk2")
    vs.run("for_time", time=40, time_unit="us")
    c1_new = get_value(vs, "clk1_count")
    c2_new = get_value(vs, "clk2_count")
    assert c1_new == c1
    assert c2_new == c2
    c1 = c1_new
    c2 = c2_new

    vs.enable_clock("clk1")
    vs.run("for_time", time=40, time_unit="us")
    c1_new = get_value(vs, "clk1_count")
    c2_new = get_value(vs, "clk2_count")
    assert c1_new > c1
    assert c2_new == c2
    c1 = c1_new
    c2 = c2_new

    vs.enable_clock("clk2")
    vs.run("for_time", time=40, time_unit="us")
    c1_new = get_value(vs, "clk1_count")
    c2_new = get_value(vs, "clk2_count")
    assert c1_new > c1
    assert c2_new > c2


if __name__ == "__main__":
    port = find_free_port()
    setup_test(port, TIMEOUT, False)

    with Verisocks(HOST, port) as vs_cli:
        test_get_clocks(vs_cli)
        test_clk_config(vs_cli)
        test_clk_enable_disable(vs_cli)
        vs_cli.finish()

# EOF
