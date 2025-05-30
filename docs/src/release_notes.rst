
.. _sec_release_notes:

Release notes
#############

Releases of documentation and code are using the same version numbers. The
version numbering system (tries and) follows the `semantic versioning
<https://semver.org/>`_ principles:

.. note::

  Given a version number ``MAJOR.MINOR.PATCH``, increment the:

    1. ``MAJOR`` version when you make incompatible API changes
    2. ``MINOR`` version when you add functionality in a backward compatible manner
    3. ``PATCH`` version when you make backward compatible bug fixes

1.3.0 - 2025-04-19
******************

* Added an :ref:`API <sec_vsl_api>` to support integrating Verisocks with
  Verilator, including full documentation
* Included a :ref:`CLI wizard script <sec_vsl_wizard>` to Python package in
  order to facilitate the creation of top-level ``C++`` code to use Verisocks
  with Verilated code
* Modified the command :ref:`get(sim_info) <sec_tcp_cmd_get>` to return
  ``time_unit`` and ``time_precision`` information

1.2.0 - 2024-03-16
******************

* Modified :py:class:`Verisocks<verisocks.verisocks.Verisocks>` constructor and
  :py:meth:`Verisocks.connect() <verisocks.verisocks.Verisocks.connect>` method
  to include arguments for multiple, delayed connection trials. Examples and
  test have been simplified accordingly.
* Added correct management of system call interrupts while waiting on client
  connection in the server code (see
  https://www.gnu.org/software/libc/manual/html_node/Interrupted-Primitives.html
  for details).
* Added section :ref:`sec_alternative_simulators`.
* Added method :py:meth:`verisocks.utils.setup_sim_run` to simplify foreseen
  support for alternative simulators.

1.1.0 - 2024-02-07
******************

* Added :py:mod:`verisocks.utils` Python utilitary functions, including
  documentation.
* Added :py:meth:`Verisocks.info() <verisocks.verisocks.Verisocks.info>` method
  as a shortcut to implement the TCP protocol :keyword:`info
  <sec_tcp_cmd_info>` command.
* Corrected *SPI master* example for standalone execution.
* Added a minimalistic *Hello world* example, working both for standalone
  execution or with pytest.


1.0.0 - 2024-01-04
******************

* First released, working version
