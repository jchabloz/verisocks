.. sectionauthor:: Jérémie Chabloz
.. role:: json(code)
    :language: json

.. _sec_tcp_protocol:

TCP protocol
############

This section describes the chosen and implemented protocol for the frames sent
and received to/from the Verisocks server TCP socket by any client that wishes
to connect to it.


.. note::

  A `reference Python client implementation
  <https://github.com/jchabloz/verisocks/blob/main/python/verisocks/verisocks.py>`_
  can be found in the Verisocks github repository.

  A plethora of tools and languages on many platforms support socket
  programming with convenient built-in libraries and methods (C, Ruby, Perl,
  Tcl, Labview, Matlab, Java, etc.). Any could be used to implement a client
  that could connect to the Verisocks server.


.. _sec_tcp_frame_format:

Frames format
*************

.. note::

  The TCP message format follows the proposal for a message format in the
  `RealPython's guide on sockets programming
  <https://realpython.com/python-sockets/>`_. It allows to deal easily with
  variable-length messages while making sure that it is possible to verify that
  the full message has been received and that it is not overlapping with the
  next message. Moreover, while it is not really foreseen at this time to be
  used in this project, it also allows to cope with the transmission of binary
  data.

The chosen frame format contains in order:

1. **Fixed-length pre-header**: Indicates the length in bytes of the following
   variable-length header.

  * *Type*: Integer value
  * *Encoding*: `Big endian <https://en.wikipedia.org/wiki/Endianness>`_ byte
    ordering (most significant byte sent first)
  * *Length*: 2 bytes -- This limits the size of the following variable-length
    header to a maximum of 65535 bytes. This is more than enough...

2. **Variable-length header**: Gives information on the message payload's
   content.

  * *Type*: Unicode text, JSON format
  * *Encoding*: UTF-8
  * *Length*: As specified by the pre-header integer value
  * *Content*: Follows partially :rfc:`9110` (HTTP semantics); only the
    following fields are being used:

      * ``content-type``: Type of content for the variable-length payload. For
        Verisocks current implementation, only :mimetype:`application/json` is
        currently supported.
      * ``content-encoding``: Encoding for the variable-length payload. Only
        ``UTF-8`` currently supported (supporting encodings other than system's
        default is a nightmare with the GNU standard C library).
      * ``content-length``: Length in bytes of the variable-length payload.

  For example, the following constitutes a valid Verisocks header (assuming
  that the following payload is indeed 110-byte long):

  .. code-block:: json

    {
      "content-type": "application/json",
      "content-encoding": "UTF-8",
      "content-length": 110
    }

3. **Variable-length payload**

  * *Type*: As specified in the header's ``content-type`` field.
  * *Encoding*: As specified in the header's ``content-encoding`` field.
  * *Length*: As specified in the header's ``content-length`` field.
  * *Content*: At the moment, the only supported content correspond to a
  * *request* (or *command*) from the client to the Verisocks server,
    respectively a *response* (or *return message*) from the Verisocks server
    to the client, formatted as a JSON object.


Full message example
--------------------

As an example of a valid Verisocks full message, you can consider the following
(taken from the `SPI master example
<https://github.com/jchabloz/verisocks/tree/main/examples/spi_master>`_
provided in the verisocks github repository):

* *Pre-header*: value 88, first byte = ``0x00``, second byte = ``0x58``
* *Header*:

  .. code-block:: json

    {
      "content-type": "application/json",
      "content-encoding": "utf-8",
      "content-length": 110
    }


* *Payload*:

  .. code-block:: json

    {
      "command": "set",
      "path": "spi_master_tb.i_spi_master.tx_buffer",
      "value": [132, 201, 31, 71, 178, 192, 137]
    }


.. _sec_tcp_commands:

Commands
********

Commands are messages sent from a client to the Verisocks server intending to
trigger specific actions for the simulator or the server.
The supported commands are exhaustively listed below.


.. _sec_tcp_cmd_info:

Information frame (**info**)
----------------------------

Provides an arbitary information text from the client to the server. This
arbitrary text is then printed out to the VPI standard output.

* JSON payload fields:

  * :json:`"command": "info"`
  * :json:`"value":` Arbitrary text content to be printed out to VPI stdout

* Returned frame (normal case):

  * :json:`"type": "ack"` (acknowledgement)
  * :json:`"value": "command info received"`

With the provided Python client reference implementation, the method
:py:meth:`Verisocks.info() <verisocks.verisocks.Verisocks.info>`
corresponds to this command.

.. _sec_tcp_cmd_finish:

Finish simulation (**finish**)
------------------------------

Finishes the simulation (equivalent to a verilog :verilog:`$finish()`
statement). This will also result in the simulator process being terminated as
well as the Verisocks server. The socket used will also be closed. After this
command, the :ref:`execution focus <sec_architecture_focus>` goes to
Verisocks.

* JSON payload fields:

  * :json:`"command": "finish"`

* Returned frame (normal case):

  * :json:`"type": "ack"` (acknowledgement)
  * :json:`"value": "Processing finish command - Terminating simulation."`

With the provided Python client reference implementation, the method
:py:meth:`Verisocks.finish() <verisocks.verisocks.Verisocks.finish>`
corresponds to this command.

.. _sec_tcp_cmd_stop:

Stop simulation (**stop**)
--------------------------

Stops the simulation (equivalent to the verilog :verilog:`$stop()` statement).
Contrary to the :ref:`finish <sec_tcp_cmd_finish>` command, the socket is not
closed. After this command, the :ref:`execution focus <sec_architecture_focus>`
goes to Verisocks.

* JSON payload fields:

  * :json:`"command": "stop"`

* Returned frame (normal case):

  * :json:`"type": "ack"` (acknowledgement)
  * :json:`"value": "Processing stop command - Stopping simulation."`

With the provided Python client reference implementation, the method
:py:meth:`Verisocks.stop() <verisocks.verisocks.Verisocks.stop>`
corresponds to this command.

.. _sec_tcp_cmd_exit:

Exit (**exit**)
---------------

Exits Verisocks and closes the socket. After this command, the :ref:`execution
focus <sec_architecture_focus>` goes to the simulator and the simulation
continues its course until it ends.

* JSON payload fields:

  * :json:`"command": "exit"`

* Returned frame (normal case):

  * :json:`"type": "ack"`  (acknowledgement)
  * :json:`"value": "Processing exit command - Quitting Verisocks."`

With the provided Python client reference implementation, the method
:py:meth:`Verisocks.exit() <verisocks.verisocks.Verisocks.exit>`
corresponds to this command.

.. _sec_tcp_cmd_run:

Run simulation (**run**)
------------------------

Transfers the :ref:`execution focus <sec_architecture_focus>` to the simulator
and runs it until a *callback event* which is specified by the command
arguments (see description below). After the callback is reached, the focus
gets back to Verisocks.

* JSON payload fields:

  * :json:`"command": "run"` Command name
  * A callback needs to be defined in order to specify when the **run**
    command has to yield the execution focus back to Verisocks. The different
    available possibilities are:

    * :json:`"cb": "for_time"` - The simulation shall run for a certain
      amount of time,
    * :json:`"cb": "until_time"` - The simulation shall run until a certain
      (simulator) time,
    * :json:`"cb": "until_change"` - The simulation shall run until a certain
      simulator variable changes to a given value,
    * :json:`"cb": "to_next"` - The simulation shall run until the next
      simulation time step.

  If the ``"cb"`` field is ``"for_time"`` or ``"until_time"``, the following
  fields are further expected in the command frame:

  * :json:`"time":` (number): Time value (either as a time difference or as an
    absolute simulation time). For example :json:`"time": 3.2`
  * :json:`"time_unit":` (string): Time unit (``"s"``, ``"ms"``, ``"us"``,
    ``"ns"``, ``"ps"`` or ``"fs"``) which applies to the ``"time"`` field
    value. Be aware that depending on the simulator time resolution, the
    provided time value can be truncated.

  If the ``"cb"`` field is ``"until_change"``, the following fields are further
  expected in the command frame:

  * :json:`"path":` (string): Path to verilog object used for the callback
  * :json:`"value":` (number): Condition on the verilog object's value for the
    callback to be executed. This argument is not required if the path
    corresponds to a named event.

  If the ``"cb"`` field is ``"to_next"``, no further fields are required.

* Returned frame (normal case):

  * :json:`"type": "ack"` (acknowledgement)
  * :json:`"value": "Reached callback - Getting back to Verisocks main loop"`

With the provided Python client reference implementation, the method
:py:meth:`Verisocks.run() <verisocks.verisocks.Verisocks.run>`
corresponds to this command.

.. _sec_tcp_cmd_get:

Get information from simulation (**get**)
-----------------------------------------

This command can be used to get pieces of information from the simulator.

* JSON payload fields:

  * :json:`"command": "get"` Command name
  * It is possible to select which shall be the returned information, using the
    following possibilities:

    * :json:`"sel": "sim_info"` - The simulator information (name and version)
      is returned,
    * :json:`"sel": "sim_time"` - The simulator (absolute) time is returned,
    * :json:`"sel": "value"` - The value of a simulator variable is returned,
    * :json:`"sel": "type"` - The VPI type of a simulator variable is returned.

  If the ``"sel"`` field is ``"value"`` or ``"type"``, the following field is
  required in the command frame:

    * :json:`"path":` (string): Path to the verilog variable

* Returned frame (for :json:`"sel": "sim_info"`):

  * :json:`"type": "result"`
  * :json:`"product":` (string): Simulator product name
  * :json:`"version":` (string): Simulator version

* Returned frame (for :json:`"sel": "sim_time"`):

  * :json:`"type": "result"`
  * :json:`"time":` (number): Simulator absolute time, in seconds

* Returned frame (for :json:`"sel": "value"`):

  * :json:`"type": "result"`
  * :json:`"value":` (number or array): Value for the queried variable

* Returned frame (for :json:`"sel": "type"`):

  * :json:`"type": "result"`
  * :json:`"vpi_type":` (number): Value for the queried variable's VPI type
    (see VPI objects types definitions in IEEE 1364-2001)

With the provided Python client reference implementation, the method
:py:meth:`Verisocks.get() <verisocks.verisocks.Verisocks.get>`
corresponds to this command.

.. _sec_tcp_cmd_set:

Set variables in simulation (**set**)
-------------------------------------

This command can be used to forcefully set the value of a simulator variable.

* JSON payload fields:

  * :json:`"command": "set"` Command name
  * :json:`"path":` (string): Path to the simulator variable to be set
  * :json:`"value":` (number or array): Value to be set. If the path
    corresponds to a verilog named event, this argument is not required. If the
    path corresponds to a memory array, this argument needs to be provided as
    an array of the same length.

* Returned frame (normal case):

  * :json:`"type": "ack"` (acknowledgement)
  * :json:`"value": "Processed command set"`

With the provided Python client reference implementation, the method
:py:meth:`Verisocks.set() <verisocks.verisocks.Verisocks.set>`
corresponds to this command.
