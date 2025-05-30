.. sectionauthor:: Jérémie Chabloz

.. _sec_vsl_wizard:

Configuration file and wizard script
####################################

In order to facilitate the usage of Verisocks with Verilator, a command line
wizard script :program:`vsl-wizard` is provided with the :py:mod:`verisocks`
python package. This script generates automatically a Makefile, a C++ top
testbench source file and a Verilator configuration file to declare the
necessary public variables for using Verilator while integrating Verisocks. The
files are generated based on entries which have to be provided by the user in a
:option:`YAML configuration file <vsl-wizard config>`.

Wizard-script usage
===================

If properly installed with :program:`pip`, an entry point is added in the path
such that the wizard script can simply be evoked as follows:

.. code:: text

    usage: vsl-wizard [-h] [--templates-dir TEMPLATES_DIR]
        [--makefile MAKEFILE] [--testbench-file TESTBENCH_FILE]
        [--variables-file VARIABLES_FILE] config

.. program:: vsl-wizard

.. option:: config

    Path to YAML configuration file

The content and structure of the YAML configuration file shall be as described
below. Unless specifically mentioned, all fields are mandatory. Entries which
are paths can be relative to the configuration file location.

.. code-block:: yaml

    config:
      prefix: <text>            # Prefix name to be used
      top: <text>               # Name of top module
      verilog_src_files:        # List of Verilog source files
      - <path>
      # ...
      cpp_src_files:            # (optional) List of C++ extra files
      - <path>
      # ...
      verisocks_root: <path>    # Path to Verisocks root directory
      verilator_path: <path>    # Path to the verilator binary
      verilator_root: <path>    # Path to Verilator root
      use_tracing: <bool>       # If true, tracing is enabled
      use_fst: <bool>           # (optional if use_tracing is false) If true,
                                # the FST format is used for the traces file
    variables:                  # (optional) Public variables
      scalars:                  # (optional) List of scalar variables
      - path: <text>            # Name/alias to be used for the variable
        module: <text>          # Name of the module in which is the variable
        type: <text>            # Variable type [uint8, uint16, uint32, uint64, real]
        width: <number>         # Width of the variable
      # ...
      arrays:                   # (optional) List of array variables
      - path: <text>            # Name/alias to be used for the variable
        module: <text>          # Name of the module in which is the variable
        type: <text>            # Variable type [uint8, uint16, uint32, uint64, real]
        width: <number>         # Width of the variable
        depth: <number>         # Depth of the array
      # ...
      params:                   # (optional) List of parameter variables
      - path: <text>            # Name/alias to be used for the variable
        module: <text>          # Name of the module in which is the variable
        type: <text>            # Variable type [uint8, uint16, uint32, uint64, real]
        width: <number>         # Width of the variable
      # ...
      events:                   # (optional) List of events
      - path: <text>            # Name/alias to be used for the event
        module: <text>          # Name of the module in which is the event
      # ...

Optional arguments
------------------

.. option:: -h, --help

    Displays help content

.. option:: --templates-dir <TEMPLATES_DIR>, -t <TEMPLATES_DIR>

    Path to templates directory if alternatives templates shall be used instead
    of the default ones

.. option:: --makefile <MAKEFILE>

    Rendered makefile name (default: :code:`Makefile`)

.. option:: --testbench-file <TESTBENCH_FILE>

    Rendered C++ testbench file (default: :code:`test_main.cpp`)

.. option:: --variables-file <VARIABLES_FILE>

    Rendered Verilator configuration file for public variables (default:
    :code:`variables.vlt`)

