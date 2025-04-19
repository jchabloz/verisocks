.. sectionauthor:: Jérémie Chabloz

.. _sec_verilator_integration:

Verilator integration
#####################

This section describes how to use Verisocks with Verilator to produce a compact
and efficient model for your hardware that can be interfaced using the
Verisocks commands.

With the current version (|version|) of the provided API, the following
assumptions are made:

* the verilog sources contain all the necessary stimuli to generate all
  required clocks and other timed events for the simulation to proceed
* the verilation is performed using the :code:`--timing` option of verilator


.. toctree::
    :maxdepth: 1

    vsl_api
    vsl_wizard

