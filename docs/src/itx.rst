.. sectionauthor:: Jérémie Chabloz
.. role:: json(code)
    :language: json


.. _sec_itx:

.. warning::

  THE CONTENT OF THIS SECTION IS A PROPOSAL - IT HAS NOT BEEN IMPLEMENTED YET


Interrupts/events
#################

On top of the callback events that are automatically registered by various
types of :ref:`run <sec_tcp_cmd_run>` commands, it is possible to register a
certain number of *interrupts* (or *events*). When one of these interrupts is
triggered, the server sends an *interrupt return message* to the client and
pauses the simulation (for *blocking* interrupts) or continues it immediately
(for *non-blocking* interrupts).

Interrupts properties
*********************

Interrupts are defined with 

- Name
- Type
- Options

Types
-----

The following types of interrupts can be created:

- **In time (in_time)** - The interrupt is triggered after a certain time,
  starting from the current simulation time. This interrupt can be optionally
  flagged as recurrent, in which case it becomes a periodic interrupt, until
  disabled. The properties ``time`` and ``time_unit`` have to be defined.
- **At time (at_time)** - The interrupt is triggered at a certain simulation
  time. This interrupt obviously cannot be recurrent. The properties ``time``
  and ``time_unit`` have to be defined.
- **On change (on_change)** - The interrupt is triggered when a defined
  variable transitions to a certain value. If flagged as recurrent, it will
  only trigger again if the watched variable has reverted to another value
  first. The properties ``path`` and ``value`` have to be defined.


Options
-------

- **Blocking** - If this option is set, when the interrupt is triggered, the
  running simulation is interrupted and the focus is given back to the
  Verisocks execution thread, exactly as if the registered callback condition
  had been met. The current running simulation transaction can be resumed with
  the a ``run(sel="resume")`` command until the corresponding, registered
  callback condition occurs. Alternatively, the registered callback can be
  cancelled with a ``run(sel="cancel")`` command.

- **Recurrent** - If this option is set, the interrupt is defined to be
  *recurrent*. For *in time* interrupts, it means that he interrupt is
  re-activated with the same time period. For *on change* interrupts, the
  interrupt is re-activated with the same variable path and same value, however
  only when the watched variable has changed back to another value first. Once
  set, recurrent interrupts have to be expressly disabled to be removed.


Interrupts-related commands
***************************

This section documents the few commands which are related to the interrupts
feature of Verisocks.

Register an interrupt
---------------------

* JSON payload fields:
  * :json:`"command": "set"`
  * :json:`"sel": "itx"`
  * :json:`"name":` (text) Reference name/ID for the interrupt

The :json:`"name"` field has to be unique.
The field :json:`"type"` defines which is the type of interrupt to be
registered and comes together with a few expected different extra fields, as
follows

* JSON payload fields for *in time* interrupts:
  
  * :json:`"type": "in_time"`
  * :json:`"time":` (number) Time period in w
  * :json:`"time_unit":`

* JSON payload fields for *at time* interrupts: 

  * :json:`"type": "at_time"`
  * :json:`"time":` (number) Simulation time at which the interrupt shall be
    triggered
  * :json:`"time_unit":` (text) Time unit

* JSON payload fiels for *on change* interrupts:

  * :json:`"type": "on_change"`
  * :json:`"path":` (text) Indicates the path to the variable to watch
  * :json:`"value":` (number) Defines for the transition to which value the
    interrupt shall be triggered

Some extra (optional) fields can be used to determine the interrupt's
properties:

* Extra JSON payload fields:

  * :json:`"blocking":` (bool, optional) Indicates if the interrupt shall be
    blocking (true) or non-blocking (false, default)
  * :json:`"recurrent":` (bool, optional) Indicates if the interrupt shall be
    recurrent (true) or unique (false, default)

Remove an interrupt
-------------------

* :json:`"command": "set"`
* :json:`"sel": "itx_remove"`
* :json:`"name":` (text) Name of the interrupt to be removed


Resume running simulation
-------------------------

* :json:`"command": "run"`
* :json:`"sel": "resume"`

This command allows to restart the simulation (give the execution focus back to
the simulator) until the currently registered callback condition has been met
after a blocking interrupt. The expected return message for this command shall
be the return message of the initial *run* command that was interrupted.

.. note::

  This command is only valid and considered when the running simulation has
  been paused by a blocking interrupt.


Cancel running simulation
-------------------------

* :json:`"command": "run"`
* :json:`"sel": "cancel"`

This command allows to cancel the normal registered callback after a blocking
interrupt has been triggered.

.. note::

  This command is only valid and considered when the running simulation has
  been paused by a blocking interrupt.
