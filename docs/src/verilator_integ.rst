.. sectionauthor:: Jérémie Chabloz

.. _sec_verilator_integration:

Verilator integration
#####################

This section describes how to use Verisocks with Verilator to produce a compact
and efficient model for your hardware.

For now, it is assumed that the verilation has been performed using the
`--timing` option.


Verisocks C++ API
*****************

.. note:: 

    All classes and functions are declared in the namespace :cpp:any:`vsl`.


.. cpp:namespace:: vsl

Verisocks integration class
===========================

.. cpp:class:: template<typename T> VslInteg

    :tparam T: Type (class name) of the verilated model. Thus, it has to derive
        from :cpp:class:`VerilatedModel`, which is checked by a static
        assertion in the class constructor.

    Any C++ testbench targeting to use Verisocks shall instantiate this class
    template using the Verilated model type as template parameter.

Public methods
--------------

The following functions are public methods for the class
:cpp:class:`vsl::VslInteg` and shall be used from the top-level C++ testbench
code.

.. cpp:namespace:: template<typename T> vsl::VslInteg

.. cpp:function:: VslInteg(T* p_model, const int port=5100, \
                           const int timeout=120)

    :tparam T: Type for the verilated model.
    :param p_model: Pointer to the verilated model instance.
    :param port: Port number for the Verisocks server (default is 5100).
    :param timeout: Timeout in seconds for the Verisocks server (default is 120).

    Constructor to instantiate the class :cpp:class:`vsl::VslInteg`. It has to
    be called from the top C++ bench source after having created an instance
    for the DUT verilated model and registered it into a Verilator context.

.. cpp:function:: const T* model()

    :returns: Pointer to the registered verilated model instance.

.. cpp:function:: const VerilatedContext* context()

    :returns: Pointer to the registered verilated model's context.

.. cpp:function:: int run()

    :returns: Exit code. A normal execution results in an exit code value of
    `0`.

    This is the main function to be used from the top-level C++ testbench code.
    It will launch the Verisocks finite-state machine and will expect a client
    to connect to the exposed socket. Once connected, it will be expecting to
    receive Verisocks commands to control the Verilated testbench simulation.

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

.. cpp:function:: register_param(const char* namep, std::any datap, \
                                  VerilatedVarType vltype, size_t width)









Configuration file and wizard script
************************************


