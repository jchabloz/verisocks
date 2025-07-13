from verisocks.verisocks import Verisocks
from verisocks.utils import setup_sim_run, find_free_port
import socket
import pytest
import logging
from os.path import join, dirname, abspath, relpath


HOST = socket.gethostbyname("localhost")
TIMEOUT = 10
cwd = relpath(dirname(abspath(__file__)))


def setup_test(port=5100, timeout=10):
    elab_cmd = ["make", "-C", cwd]
    sim_cmd = [
        join(cwd, "hello_world"),
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


def test_hello_world(vs):

    assert vs._connected
    answer = vs.get("sim_info")
    assert answer['type'] == "result"
    print(f"Simulator: {answer['product']}")
    print(f"Version: {answer['version']}")

    answer = vs.info("Hello World!")
    assert answer['type'] == "ack"


if __name__ == "__main__":

    logging.info("Executing standalone Python testcase")
    port = find_free_port()
    setup_test(port, TIMEOUT)
    with Verisocks(HOST, port) as vs_cli:
        test_hello_world(vs_cli)
