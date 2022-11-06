from verisocks.verisocks import Verisocks
import subprocess
import os.path
import time
import shutil
import logging

# Parameters
HOST = "127.0.0.1"
PORT = 5100
LIBVPI = "../../build/libvpi.so"  # Relative path to this file!
CONNECT_DELAY = 0.01


def get_abspath(relpath):
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
        *src_file_paths,
    ]
    subprocess.check_call(cmd)
    libvpi_path = get_abspath(LIBVPI)
    cmd = [shutil.which("vvp"), "-m", libvpi_path, vvp_file_path, "-fst"]
    pop = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print(f"Launched Icarus with PID {pop.pid}")
    # Some delay is required for Icarus to launch the Verisocks server before
    # being able to connect - Please adjust CONNECT_DELAY if required.

    time.sleep(CONNECT_DELAY)
    return pop


def send_spi(vs, tx_buffer):
    vs.set(path="spi_master_tb.i_spi_master.tx_buffer",
           value=tx_buffer)
    answer = vs.get(sel="value",
                    path="spi_master_tb.i_spi_master.transaction_counter")
    counter = answer['value']
    vs.set(path="spi_master_tb.i_spi_master.start_transaction")
    vs.run(cb="until_change",
           path="spi_master_tb.i_spi_master.end_transaction")
    answer = vs.get(sel="value",
                    path="spi_master_tb.i_spi_master.rx_buffer")
    rx_buffer = answer['value']
    return rx_buffer, counter


if __name__ == "__main__":
    pop = setup_iverilog("spi_master_tb",
                         "spi_master.v",
                         "spi_slave.v",
                         "spi_master_tb.v")
    with Verisocks(HOST, PORT) as vs:
        assert vs._connected

        vs.run(cb="for_time", time=10, time_unit="us")

        tx = [12, 34, 56, 78, 90, 12, 34]
        rx, counter = send_spi(vs, tx)
        logging.info(f"Content of TX buffer: {tx}")
        logging.info(f"Content of RX buffer: {rx}")
        logging.info(f"Transaction counter: {counter}")

        vs.run(cb="for_time", time=10, time_unit="us")

        tx = [1, 3, 5, 7, 11, 13, 17]
        rx, counter = send_spi(vs, tx)
        logging.info(f"Content of TX buffer: {tx}")
        logging.info(f"Content of RX buffer: {rx}")
        logging.info(f"Transaction counter: {counter}")

        vs.run(cb="for_time", time=10, time_unit="us")

        vs.finish()
    pop.wait(timeout=10)
