from verisocks.verisocks import Verisocks
from verisocks.utils import setup_sim_run
import socket

# Parameters
HOST = socket.gethostbyname("localhost")
PORT = 5100


def setup_sim():
    elab_cmd = ["make"]
    sim_cmd = ["./Vspi_master_tb"]
    pop = setup_sim_run(elab_cmd, sim_cmd, capture_output=False)
    return pop


def get_sim_info(vs):
    answer = vs.get("sim_info")
    print(f"Simulator: {answer['product']}")
    print(f"Version: {answer['version']}")
    print(f"Model name: {answer['model_name']}")


def get_sim_time(vs):
    answer = vs.get("sim_time")
    print(f"Simulation time: {answer['time']}")


def get_variable_value(vs, path):
    answer = vs.get("value", path)
    print(f"Value for variable {path}: {answer['value']}")


pop = setup_sim()
with Verisocks(HOST, PORT) as vs_cli:
    answer = vs_cli.info("Verilator integration test")
    print(answer)

    get_sim_info(vs_cli)
    get_sim_time(vs_cli)
    get_variable_value(vs_cli, "spi_master_tb.miso")
    get_variable_value(vs_cli, "spi_master_tb.tutu")
    get_variable_value(vs_cli, "spi_master_tb.toto")
    get_variable_value(vs_cli, "spi_master_tb.tata")
    # get_variable_value(vs_cli, "spi_master_tb.start_transaction")

    answer = vs_cli.set("spi_master_tb.tutu", value=237)
    print(answer)
    get_variable_value(vs_cli, "spi_master_tb.tutu")

    answer = vs_cli.set("spi_master_tb.tata", value=[45]*12)
    print(answer)
    get_variable_value(vs_cli, "spi_master_tb.tata")

    answer = vs_cli.send_cmd("finish")
    # answer = vs_cli.send_cmd("exit")
    print(answer)

pop.communicate(timeout=10)
# EOF
