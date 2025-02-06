from verisocks.verisocks import Verisocks
import socket

# Parameters
HOST = socket.gethostbyname("localhost")
PORT = 5100


def get_sim_info(vs):
    answer = vs_cli.get("sim_info")
    print(f"Simulator: {answer['product']}")
    print(f"Version: {answer['version']}")
    print(f"Model name: {answer['model_name']}")


def get_sim_time(vs):
    answer = vs_cli.get("sim_time")
    print(f"Simulation time: {answer['time']}")


def get_variable_value(vs, path):
    answer = vs.get("value", path)
    print(f"Value for variable {path}: {answer['value']}")


with Verisocks(HOST, PORT) as vs_cli:
    answer = vs_cli.info("Verilator integration test")
    print(answer)

    get_sim_info(vs_cli)
    get_sim_time(vs_cli)
    get_variable_value(vs_cli, "spi_master_tb.miso")
    get_variable_value(vs_cli, "spi_master_tb.tutu")
    get_variable_value(vs_cli, "spi_master_tb.toto")
    get_variable_value(vs_cli, "spi_master_tb.tata")

    answer = vs_cli.send_cmd("finish")
    # answer = vs_cli.send_cmd("exit")
    print(answer)

# EOF
