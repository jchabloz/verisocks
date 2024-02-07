from verisocks.verisocks import Verisocks
from verisocks.utils import setup_sim, find_free_port
import socket
import time
import pytest
import logging
import os.path


HOST = socket.gethostbyname("localhost")
PORT = find_free_port()
VS_TIMEOUT = 10
LIBVPI = "../../build/verisocks.vpi"
CONNECT_DELAY = 0.1


def setup_test():
    setup_sim(
        LIBVPI,
        "hello_world_tb.v",
        cwd=os.path.dirname(__file__),
        ivl_args=[
            f"-DVS_NUM_PORT={PORT}",
            f"-DVS_TIMEOUT={VS_TIMEOUT}"
        ]
    )
    time.sleep(CONNECT_DELAY)


@pytest.fixture
def vs():
    setup_test()
    _vs = Verisocks(HOST, PORT)
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

    setup_test()
    with Verisocks(HOST, PORT) as vs_cli:
        test_hello_world(vs_cli)
