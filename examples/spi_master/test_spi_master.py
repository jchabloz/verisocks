from verisocks.verisocks import Verisocks
import subprocess
import os.path
import time
import shutil
import logging
import pytest
import socket


def find_free_port():
    with socket.socket() as s:
        s.bind(('', 0))
        return s.getsockname()[1]


# Parameters
HOST = socket.gethostbyname("localhost")
PORT = find_free_port()
LIBVPI = "../../build/verisocks.vpi"  # Relative path to this file!
CONNECT_DELAY = 0.01
VS_TIMEOUT = 10


def get_abspath(relpath):
    """Builds an absolute path from a path which is relative to the current
    file

    Args:
        * relpath (str): Relative path

    Returns:
        * abspath (str): Absolute path
    """
    return os.path.join(os.path.dirname(__file__), relpath)


def setup_iverilog(vvp_name, *src_files):
    """Elaborate and run the verilog testbench file provided as an argument

    Args:
        * src_file (str): Path to source file

    Returns:
        * pop: Popen instance for spawned process
    """
    src_file_paths = []
    for src_file in src_files:
        src_file_path = get_abspath(src_file)
        if not os.path.isfile(src_file_path):
            raise FileNotFoundError
        src_file_paths.append(src_file_path)
    vvp_file_path = get_abspath(vvp_name)
    cmd = [
        shutil.which("iverilog"),
        "-o", vvp_file_path,
        "-Wall",
        f"-DVS_NUM_PORT={PORT}",
        f"-DVS_TIMEOUT={VS_TIMEOUT}",
        *src_file_paths,
    ]
    subprocess.check_call(cmd)
    libvpi_path = get_abspath(LIBVPI)
    cmd = [shutil.which("vvp"), "-lvvp.log", "-m", libvpi_path,
           vvp_file_path, "-fst"]
    pop = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print(f"Launched Icarus with PID {pop.pid}")

    # Some delay is required for Icarus to launch the Verisocks server before
    # being able to connect - Please adjust CONNECT_DELAY if required.
    time.sleep(CONNECT_DELAY)

    return pop


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
           path="spi_master_tb.i_spi_master.end_transaction")

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
    pop = setup_iverilog("spi_master_tb",
                         "spi_master.v",
                         "spi_slave.v",
                         "spi_master_tb.v")
    _vs = Verisocks(HOST, PORT)
    _vs.connect()
    yield _vs
    # Teardown
    _vs.finish()
    _vs.close()
    pop.wait(timeout=10)


def test_spi_master_simple(vs):

    assert vs._connected

    # Let the simulator run for 10 us
    answer = vs.run(cb="for_time", time=10, time_unit="us")
    assert answer["type"] == "ack"

    # Trigger an SPI transaction
    tx = [12, 34, 56, 78, 90, 12, 34]
    rx, counter = send_spi(vs, tx)
    assert counter == 0

    vs.run(cb="for_time", time=10, time_unit="us")
    tx_previous = tx
    tx = [1, 3, 5, 7, 11, 13, 17]
    rx, counter = send_spi(vs, tx)
    assert rx[1:] == tx_previous
    assert counter == 1

    vs.run(cb="for_time", time=10, time_unit="us")
    tx_previous = tx
    tx = [19, 23, 29, 31, 37, 41, 43]
    rx, counter = send_spi(vs, tx)
    assert rx[1:] == tx_previous
    assert counter == 2
