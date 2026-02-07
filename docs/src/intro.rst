.. sectionauthor:: Jérémie Chabloz
.. _sec_introduction:

Introduction
############

.. _sec_quick_start:

Quick start
***********

Install Verisocks from sources
------------------------------

Make sure that the following pre-requisites are available on your system prior
to trying and compile Verisocks:

* Depending on the desired use case (using either Icarus or Verilator):

  * Valid Icarus Verilog installation (version >= 11.0). Follow the `Icarus
    reference installation guide
    <https://steveicarus.github.io/iverilog/usage/qinstallation.html>`_ or use
    the packaged version provided with your favorite distro.
  * Valid `Verilator <https://www.veripool.org/verilator/>`_ installation
    (version >= 5.0). Follow the latest `installation guide
    <https://verilator.org/guide/latest/install.html>`_.

* GCC C/C++ compiler (LLVM should of course also do the trick, even though I
  have not tested this extensively)

.. note::

    Older GCC versions will most likely complain about the variadic macros used
    for logging purposes. These warnings can normally be safely ignored...

The easiest way to get a full copy of the latest code is to clone the git
repository:

.. code-block:: sh

    git clone https://github.com/jchabloz/verisocks.git <path to your verisocks folder>

Run the ``configure`` script and use ``make`` in order to compile the VPI
module.

.. code-block:: sh

    ./configure
    make


If the ``configure`` script fails to find the path to the installed Icarus VPI
header file :file:`vpi_user.h` make sure to provide it as an argument. The same
applies for the path to the shared library :file:`libvpi.a` to be provided as a
linker argument:

.. code-block:: sh

    ./configure CFLAGS=-I<path to your vpi_user.h> LDFLAGS=-L<path to your libvpi.a>


Install the reference Python client
-----------------------------------

In order to use the provided reference Python client, you can directly and
easily install it using :code:`pip`. Remember that it is considered best
practice to use `virtual environments
<https://docs.python.org/3/glossary.html#term-virtual-environment>`_.

.. code-block:: sh

	python -m venv <path to your virtual environment folder>
	source <path to your virtual environment folder>/bin/activate
	pip install verisocks

Alternatively, if you cloned the full git repository, you can directly install
it from the downloaded source:

.. code-block:: sh

    pip install <path to your verisocks folder>/python

.. note::

  The main advantage of Verisock's socket interface is its versatility. The
  Python client provided with the code can serve as a reference implementation
  that can easily be replicated in any language with an API for TCP sockets.

Run the examples
----------------

The `verisocks repository
<https://github.com/jchabloz/verisocks/tree/main/examples>`_ contains an
:file:`examples` folder. Please refer to the relevant :file:`README.md` files.


Write your own verification code
--------------------------------

You are now ready to use verisocks to write your own verification code!

.. _sec_motivation:

Motivation
**********

Why Verisocks? I have been using the `Icarus Verilog
<https://steveicarus.github.io/iverilog/>`_ simulator for quite some time now.
It is a quite complete and performant tool that enables to perform verification
of simple verilog modules but also of complete systems. However, there are a
few things that are still potentially missing in the picture:

* How to **efficiently organize test cases and test suites**, ideally
  interfacing with existing test frameworks so as not to reinvent the wheel?
* How to define and perform **regression tests**?
* How to establish **traceability**, typically with a set of requirements
* Etc...

And to top all of these considerations, I would like to be able to use Python
to do all of it... No justification, just my preferred tool of the moment.
However, a good solution shall be easy to interface with any other standard
scripting/programming language.

.. note::

  As I was looking for solutions, I found out that `cocotb
  <https://docs.cocotb.org/en/stable/>`_ proposes a nice approach and could
  definitely fit the bill. While it is definitely a rising star in the
  verification world, I was not completely satisfied, though. I could make a
  detailed list of whys, but it is not the goal to position Verisocks against
  cocotb; it just approaches the same needs in different ways.


All in all, I decided to try and implement my own solution (it's more fun
anyway); re-write from scratch a simple-to-use (emphasis on *simple*) interface
using the standardized `Verilog Procedural Interface (VPI)
<https://en.wikipedia.org/wiki/Verilog_Procedural_Interface>`_ in order to make
it possible to control externally an Icarus simulation.

.. highlights::

  The main idea is to make the Icarus simulation instance behave as a *server*
  to which it would be possible to submit *requests* via a *client*.


The requests to be submitted to the *Verisocks simulation server* would then
have to typically be:

* **get values** of simulation variables,
* **set values** for simulation variables,
* **get simulation time**,
* **run** the simulation **for** a certain amount of (simulation) time
* **run** the simulation **until** a certain (simulation) time,
* **run** the simulation **until** a certain event (e.g. a given simulation
  variable rising edge),
* etc...

.. _sec_alternative_simulators:

Alternative simulators
**********************

While the Verisocks PLI application has been developed targeting specifically
Icarus as a verilog simulator, there is no known reason that it would not be
working as well with any other simulator that is supporting the VPI normative
interface (as defined in `IEEE Std 1364
<https://ieeexplore.ieee.org/document/1620780>`_ and `IEEE Std 1800
<https://ieeexplore.ieee.org/document/10458102>`_), including mainstream
commercial simulators.

.. note::
    I will gladly accept any contribution to test Verisocks with other
    simulators.
    As of now, I have only successfully tested it with Cadence's XCelium 64
    29.03. As soon as I get more material, I will make a short tutorial for it.
    My next target will be Tachyon's CVC. If anybody is able to test it with
    QuestaSim...
