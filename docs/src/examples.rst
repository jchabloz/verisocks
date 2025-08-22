.. sectionauthor:: Jérémie Chabloz

.. _sec_examples:

Examples
########

.. _sec_example_hello_world:

Hello World
***********

This example shows how to use Verisocks with a minimalistic test case. The test
does nothing except executing the :code:`$verisocks_init()` system task and
requesting from Verisocks the simulator name and version and then send an
:code:`Hello World!` string to the simulator using the :ref:`info <cmd_info>`
command of the TCP protocol.

The Python helper function :py:meth:`setup_sim() <verisocks.utils.setup_sim>`
is used in order to easily set up the simulation with the Icarus simulator in
the Python test function.

All the sources and corresponding README file can be found in the `example
repository folder
<https://github.com/jchabloz/verisocks/blob/main/examples/hello_world>`_.


SPI master
**********

This example shows how to use Verisocks with a simple SPI master model in order
to drive a full-duplex SPI bus with arbitrary content as well as read the
results sent by an SPI slave on the same bus.  For the purpose of the example,
a simplistic SPI slave sends back the same content that was sent by the master
in the previous transaction.

All the sources and corresponding README file can be found in the `spi master
example repository folder
<https://github.com/jchabloz/verisocks/blob/main/examples/spi_master>`_.

