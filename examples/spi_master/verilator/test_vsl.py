from verisocks.verisocks import Verisocks
from verisocks.utils import setup_sim_run
import socket
import pytest
import logging
import numpy as np


# Parameters
HOST = socket.gethostbyname("localhost")
PORT = 5100


def setup_sim(capture_output=True):
    elab_cmd = ["make"]
    sim_cmd = ["./Vspi_master_tb"]
    pop = setup_sim_run(elab_cmd, sim_cmd, capture_output=capture_output)
    return pop


@pytest.fixture
def vs():
    setup_sim()
    _vs = Verisocks(HOST, PORT)
    _vs.connect()
    yield _vs
    try:
        _vs.finish()
    except ConnectionError:
        logging.warning("Connection error - Finish command not possible")
    _vs.close()


def test_get_sim_info(vs):
    logging.info("Verifying get(sim_info)")
    assert vs._connected
    answer = vs.get("sim_info")
    assert answer['type'] == "result"
    assert answer['product'] == "Verilator"
    assert 'version' in answer
    assert 'model_name' in answer


def test_get_sim_time(vs):
    logging.info("Verifiying get(sim_time)")
    assert vs._connected
    answer = vs.get("sim_time")
    assert answer['type'] == "result"


def get_variable_value(vs, path):
    answer = vs.get("value", path)
    assert answer['type'] == "result"
    return answer['value']


def test_variable_access(vs):
    logging.info("Verifying accessing variables")
    assert vs._connected

    get_variable_value(vs, "spi_master_tb.miso")
    get_variable_value(vs, "spi_master_tb.tutu")
    get_variable_value(vs, "spi_master_tb.toto")
    get_variable_value(vs, "spi_master_tb.tata")
    get_variable_value(vs, "spi_master_tb.i_spi_master.start_transaction")
    get_variable_value(vs, "spi_master_tb.i_spi_master.transaction_counter")

    answer = vs.set("spi_master_tb.tutu", value=237)
    assert answer['type'] == 'ack'
    x = get_variable_value(vs, "spi_master_tb.tutu")
    assert x == 237

    answer = vs.set("spi_master_tb.tata", value=[45]*12)
    assert answer['type'] == 'ack'
    x = get_variable_value(vs, "spi_master_tb.tata")
    assert np.all(x == [45]*12)


def test_run(vs):
    logging.info("Verifying run commands")
    assert vs._connected

    answer = vs.run("for_time", time=14.32, time_unit="us", timeout=60)
    assert answer['type'] == 'ack'
    answer = vs.get("sim_time")
    assert answer['type'] == "result"
    assert answer['time'] == 14.32e-6

    answer = vs.run("until_time", time=114, time_unit="us")
    assert answer['type'] == 'ack'
    answer = vs.get("sim_time")
    assert answer['type'] == "result"
    assert answer['time'] == 114.e-6


if __name__ == "__main__":

    pop = setup_sim(False)
    with Verisocks(HOST, PORT) as vs:
        test_get_sim_info(vs)
        test_get_sim_time(vs)
        test_variable_access(vs)
        test_run(vs)
        vs.finish()
    pop.communicate(timeout=10)

