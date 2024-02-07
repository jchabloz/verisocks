# Verisocks example - Hello World

## Introduction

This example shows how to use Verisocks with a minimalistic test case. The test
does nothing except asking Verisocks for the simulator name and version and
then send an "Hello World!" string to the simulator using the `info` command of
the TCP protocol.

This example uses the helper function `setup_sim()` in order to easily set up
the simulation.

## Files

The example folder contains the following files:

* [hello_world_tb.v](hello_world_tb.v): Top verilog testbench
* [test_hello_world.py](test_hello_world.py): Verisocks Python testbench file

## Running the example

This example can be run by directly executing the Python file or by using
[`pytest`](https://docs.pytest.org).

### Standalone execution

Simply run the test script:
```sh
python test_hello_world.py
```

The results of the test script execution can be checked from the content of the
`vvp.log` file, which should look like this:

```log
INFO [Verisocks]: Server address: 127.0.0.1
INFO [Verisocks]: Port: 44041
INFO [Verisocks]: Connected to localhost
INFO [Verisocks]: Command "get(sel=sim_info)" received.
INFO [Verisocks]: Command "info" received.
INFO [Verisocks]: Hello World!
INFO [Verisocks]: Command "finish" received. Terminating simulation...
INFO [Verisocks]: Returning control to simulator
```

### Using pytest

If you already have it installed, simply run `pytest` from within the SPI
master example directory or from a parent directory.
Otherwise, follow [installation instruction](https://docs.pytest.org/en/latest/getting-started.html#install-pytest). 
