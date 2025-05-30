from verisocks.verisocks import Verisocks
from verisocks.utils import setup_sim, find_free_port
import os.path
import logging
import pytest
import socket
import random


# Parameters
HOST = socket.gethostbyname("localhost")
PORT = find_free_port()
LIBVPI = "../../build/verisocks.vpi"  # Relative path to this file!
VS_TIMEOUT = 10
SRC = ["spi_master.v", "spi_slave.v", "spi_master_tb.v"]

cwd = os.path.dirname(__file__)
dump_file = os.path.join(cwd, "spi_master_tb.fst")

def setup_test():
    setup_sim(
        LIBVPI,
        *SRC,
        cwd=cwd,
        vvp_filepath="spi_master_tb",
        ivl_args=[
            f"-DVS_NUM_PORT={PORT}",
            f"-DVS_TIMEOUT={VS_TIMEOUT}",
            f"-DDUMP_FILE=\"{dump_file}\""
        ]
    )


def send_spi(vs, tx_buffer):
    """Triggers an SPI transaction

    Args:
        * vs: Verisocks instance, connected
        * tx_buffer: Array of byte values (7 bytes). Note that the SPI
                     transaction occurs with the most-significant byte
                     sent first.
    Returns:
        * rx_buffer: Content of the SPI slave RX buffer that was received
                     during the transaction.
    """
    logging.info("Sending SPI transaction")
    # Set the TX buffer simulation variable with the desired content
    vs.set(path="spi_master_tb.i_spi_master.tx_buffer",
           value=tx_buffer)
    tx_buffer_log = tx_buffer.copy()
    tx_buffer_log.reverse()
    logging.info(f"Content of TX buffer (MSB first): {tx_buffer_log}")

    # Get the transaction counter simulation variable value
    answer = vs.get(sel="value",
                    path="spi_master_tb.i_spi_master.transaction_counter")
    counter = answer['value']

    # Trigger the SPI transaction by using the named event "start_transaction"
    # as a hook to execute the corresponding verilog task
    vs.set(path="spi_master_tb.i_spi_master.start_transaction")

    # Let the simulator run until the named event "end_transaction" has been
    # triggered.
    vs.run(cb="until_change",
           path="spi_master_tb.i_spi_master.end_transaction",
           timeout=10.0)

    # Get the RX buffer simulation variable value
    answer = vs.get(sel="value",
                    path="spi_master_tb.i_spi_master.rx_buffer")
    rx_buffer = answer['value']
    rx_buffer_log = rx_buffer.copy()
    rx_buffer_log.reverse()
    logging.info(f"Content of RX buffer (MSB first): {rx_buffer_log}")
    logging.info(f"Transaction counter: {counter}")

    return rx_buffer, counter


@pytest.fixture
def vs():
    # Set up Icarus simulation and launch it as a separate process
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


def get_random_tx_buffer():
    return [random.randint(0, 255) for i in range(7)]


def test_spi_master_simple(vs):

    random.seed(57383)
    assert vs._connected

    # Let the simulator run for 10 us
    answer = vs.run(cb="for_time", time=10, time_unit="us")
    assert answer["type"] == "ack"

    # Trigger an SPI transaction
    tx = get_random_tx_buffer()
    counter_expected = 0
    rx, counter = send_spi(vs, tx)
    assert counter == counter_expected

    for i in range(200):
        vs.run(cb="for_time", time=10, time_unit="us")
        tx_expected = tx
        counter_expected += 1
        tx = get_random_tx_buffer()
        rx, counter = send_spi(vs, tx)
        assert rx[1:] == tx_expected
        assert counter == counter_expected


if __name__ == "__main__":

    setup_test()
    with Verisocks(HOST, PORT) as vs_cli:
        test_spi_master_simple(vs_cli)
