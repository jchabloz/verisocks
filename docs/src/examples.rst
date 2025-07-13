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

This example shows how to 


