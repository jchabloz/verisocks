
.. _sec_release_notes:

Release notes
#############

Releases of documentation and code are using the same version numbers. The
version numbering system follows the `semantic versioning
<https://semver.org/>`_ principles.

1.1.1 - Ongoing
***************

* Modified :py:class:`Verisocks<verisocks.verisocks.Verisocks>` constructor and
  :py:meth:`Verisocks.connect() <verisocks.verisocks.Verisocks.connect>` method
  to include arguments for multiple, delayed connection trials. Examples and
  test have been simplified accordingly.
* Added correct management of system call interrupts while waiting on client
  connection in the server code (see
  https://www.gnu.org/software/libc/manual/html_node/Interrupted-Primitives.html
  for details).
* Added section :ref:`sec_alternative_simulators`.

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
