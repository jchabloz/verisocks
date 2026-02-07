.. sectionauthor:: Jérémie Chabloz

.. _sec_verilator_integration:

Verilator integration
#####################

This section describes how to use Verisocks with Verilator to produce a compact
and efficient model for your hardware that can be interfaced using the
Verisocks commands.

With the versions 1.4 and below of the provided API, the following
assumptions are made:

* the verilog sources contain all the necessary stimuli to generate all
  required clocks and other timed events for the simulation to proceed
* the verilation is performed using the :code:`--timing` option of verilator

From version 1.5 and above, the API has been completed such that:

* if possible, the verilation can be (read *should be*) performed *without* the
  :code:`--timing` option,
* the required clock signals can be declared in the C++ testbench using the
  :cpp:func:`vsl::VslInteg::register_clock` method. These clock signals can
  then be controlled (enabled, disabled, configured) using the :ref:`set
  <sec_tcp_cmd_set>` Verisocks command using the appropriate fields.


.. toctree::
    :maxdepth: 1

    vsl_api
    vsl_wizard

