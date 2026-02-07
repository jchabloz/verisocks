from verisocks.verisocks import Verisocks
from verisocks.utils import setup_sim_run, find_free_port
import logging
import pytest
import socket
import random
from os.path import join, dirname, abspath, relpath
from math import floor, ceil

# Parameters
HOST = socket.gethostbyname("localhost")
TIMEOUT = 10
cwd = relpath(dirname(abspath(__file__)))


def setup_test(port=5100, timeout=10):
    elab_cmd = ["make", "-C", cwd]
    sim_cmd = [
        join(cwd, "Vcounter"),
        f"{port}",
        f"{timeout}"
    ]
    pop = setup_sim_run(elab_cmd, sim_cmd, capture_output=True)
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


def test_counter(vs):

	# Run the simulation for some time and verify that the returned answer is
	# an acknowledgement
	t0_us = 1.1
	answer = vs.run("for_time", time=t0_us, time_unit="us")
	assert(answer["type"] == "ack")
	sim_time = t0_us

	# Set the active-low async reset signal
	answer = vs.set("arst_b", value=1)
	assert(answer["type"] == "ack")

	# Run the simulation for some time and verify that the simulation time is
	# consistent
	t1_us = 100
	answer = vs.run("for_time", time=t1_us, time_unit="us")
	assert(answer["type"] == "ack")
	answer = vs.get("sim_time")
	assert(answer["type"] == "result")
	sim_time += t1_us
	assert(answer["time"] == pytest.approx(sim_time*1.0e-6))

	# Verify the counter value
	answer = vs.get("value", path="count")
	assert(answer["type"] == "result")
	counter_value = answer["value"]
	tclk_us = 1.4 # Clock period according to config.yaml
	assert(counter_value == pytest.approx(t1_us/tclk_us, abs=1))

	# Disable the clock signal
	answer = vs.disable_clock("clk")
	assert(answer["type"] == "ack")

	t2_us = 15.3
	answer = vs.run("for_time", time=t2_us, time_unit="us")
	assert(answer["type"] == "ack")

	# Check that the simulation time has advanced as expected
	answer = vs.get("sim_time")
	assert(answer["type"] == "result")
	sim_time += t2_us
	assert(answer["time"] == pytest.approx(sim_time*1.0e-6))

	# Check that the counter value has not been incremented (since the clock is
	# disabled)
	answer = vs.get("value", path="count")
	assert(answer["type"] == "result")
	assert(answer["value"] == counter_value)

	# Re-configure the clock and re-enable it
	answer = vs.configure_clock("clk", 2.0, "us", 0.33)
	assert(answer["type"] == "ack")
	answer = vs.enable_clock("clk")
	assert(answer["type"] == "ack")

	# Run for some time
	t3_us = 100
	answer = vs.run("for_time", time=t3_us, time_unit="us")
	assert(answer["type"] == "ack")

	# Check that the simulation time has advanced as expected
	answer = vs.get("sim_time")
	assert(answer["type"] == "result")
	sim_time += t3_us
	assert(answer["time"] == pytest.approx(sim_time*1.0e-6))

	# Check that the counter value has been incremented as expected
	answer = vs.get("value", path="count")
	assert(answer["type"] == "result")
	assert(answer["value"] == counter_value + 50)


if __name__ == "__main__":
    port = find_free_port()
    setup_test(port, TIMEOUT)
    with Verisocks(HOST, port) as vs_cli:
        test_counter(vs_cli)

# EOF
