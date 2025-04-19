.. sectionauthor:: Jérémie Chabloz

.. _sec_vsl_api:

Verisocks C++ API
#################

The integration of Verisocks with Verilator articulates itself around a C++ API
offering classes and methods that very easily allow to create top-level
testbench code.

.. note:: 

    All classes and functions are declared in the namespace :cpp:any:`vsl`.


.. cpp:namespace:: vsl

Verisocks integration class
***************************

.. cpp:class:: template<typename T> VslInteg

    :tparam T: Type (class name) of the verilated model. Thus, it has to derive
        from :cpp:class:`VerilatedModel`, which is checked by a static
        assertion in the class constructor

    Any C++ testbench targeting to use Verisocks shall instantiate this class
    template using the Verilated model type as template parameter.

Public methods
==============

The following functions are public methods for the class
:cpp:class:`vsl::VslInteg` and shall be used from the top-level C++ testbench
code.

.. cpp:namespace:: template<typename T> vsl::VslInteg

.. cpp:function:: VslInteg(T* p_model, const int port=5100, \
                           const int timeout=120)

    :tparam T: Type for the verilated model
    :param p_model: Pointer to the verilated model instance
    :param port: Port number for the Verisocks server (default is 5100)
    :param timeout: Timeout in seconds for the Verisocks server (default is
        120)

    Constructor to instantiate the class :cpp:class:`vsl::VslInteg`. It has to
    be called from the top C++ bench source after having created an instance
    for the DUT verilated model and registered it into a Verilator context.

.. cpp:function:: const T* model()

    :returns: Pointer to the registered verilated model instance

.. cpp:function:: const VerilatedContext* context()

    :returns: Pointer to the registered verilated model's context

.. cpp:function:: int run()

    :returns: Exit code. A normal execution results in an exit code value of
        `0`.

    This is the main function to be used from the top-level C++ testbench code.
    It will launch the Verisocks finite-state machine and will expect a client
    to connect to the exposed socket. Once connected, it will be expecting to
    receive Verisocks commands to control the Verilated testbench simulation.

The methods :cpp:func:`vsl::VslInteg::register_scalar`,
:cpp:func:`vsl::VslInteg::register_param`,
:cpp:func:`vsl::VslInteg::register_array` and
:cpp:func:`vsl::VslInteg::register_event` described below are all useful to
declare internal specific variables to be accessible with Verisocks commands
such as :keyword:`get <sec_tcp_cmd_get>`, :keyword:`set <sec_tcp_cmd_set>` or
:keyword:`run <sec_tcp_cmd_run>`. In order to declare such variables, they need
to be accessible in the verilated code. This can be ensured by declaring them
as *Verilator public variables*, either directly in the verilog sources or by
using a dedicated verilator configuration file.

.. cpp:function:: void register_scalar(const char* namep, std::any datap, \
                                  VerilatedVarType vltype, size_t width)

    :param namep: Name of the variable as registered by Verisocks and how it
        will be used as *path* within Verisocks commands
    :param datap: Pointer to the variable in the verilated C++ code
    :param vltype: Variable verilated type
    :param width: Variable width (should be consistent with :cpp:any:`vltype`)

    This function shall be used from within the top-level C++ testbench code
    scope in order to register a scalar variable to be accessible with
    Verisocks commands.

.. cpp:function:: void register_param(const char* namep, std::any datap, \
                                  VerilatedVarType vltype, size_t width)

    :param namep: Name of the parameter as registered by Verisocks and how it
        will be used as *path* within Verisocks commands
    :param datap: Pointer to the parameter variable in the verilated C++ code
    :param vltype: Variable verilated type
    :param width: Variable width (should be consistent with :cpp:any:`vltype`)

    This function shall be used from within the top-level C++ testbench code
    scope in order to register a scalar parameter to be accessible (*read
    only*) with Verisocks commands.

.. cpp:function:: void register_array(const char* namep, std::any datap, \
                                  VerilatedVarType vltype, size_t width, \
                                  size_t depth)

    :param namep: Name of the array variable as registered by Verisocks and how
        it will be used as *path* within Verisocks commands
    :param datap: Pointer to the array variable in the verilated C++ code
    :param vltype: Variable verilated type
    :param width: Array items width (should be consistent with
        :cpp:any:`vltype`)
    :param depth: Array depth

    This function shall be used from within the top-level C++ testbench code
    scope in order to register an array variable (only 1-dimensional arrays are
    currently supported) to be accessible with Verisocks commands.

.. cpp:function:: void register_event(const char* namep, VlEvent* eventp)

    :param namep: Name of the event variable as registered by Verisocks and how
        it will be used as *path* within Verisocks commands
    :param eventp: Pointer to the event variable in the verilated C++ code

    This function shall be used from within the top-level C++ testbench code
    scope in order to register a named event variable to be accessible with
    Verisocks commands.

