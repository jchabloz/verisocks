from verisocks.verisocks import Verisocks
import socket

# Parameters
HOST = socket.gethostbyname("localhost")
PORT = 5100

with Verisocks(HOST, PORT) as vs_cli:
    answer = vs_cli.info("Verilator integration test")
    print(answer)

    answer = vs_cli.get("sim_info")
    print(answer)

    answer = vs_cli.get("sim_time")
    print(answer)

    answer = vs_cli.get("value", "spi_master_tb.miso")
    print(answer)
    answer = vs_cli.get("value", "spi_master_tb.tutu")
    print(answer)
    answer = vs_cli.get("value", "spi_master_tb.toto")
    print(answer)
    answer = vs_cli.get("value", "spi_master_tb.i_spi_master.tx_buffer")
    print(answer)

    answer = vs_cli.send_cmd("finish")
    # answer = vs_cli.send_cmd("exit")
    print(answer)

# EOF
