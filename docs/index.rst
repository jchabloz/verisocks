.. Verisocks documentation master file, created by
   sphinx-quickstart on Tue Nov  8 02:34:11 2022.

.. |verisocks_logo| image::  _static/vs_logo_icon.png
    :height: 40px

#####################################
|verisocks_logo| Welcome to Verisocks
#####################################

You are reading the documentation for **Verisocks**, a generic TCP socket
server interface over VPI, originally intended to be used as an interface for
the `Icarus verilog simulator <https://steveicarus.github.io/iverilog/#>`_.

.. |github_logo| image:: _static/github-mark.png
    :height: 25px
    :target: https://github.com/jchabloz/verisocks

|github_logo| https://github.com/jchabloz/verisocks


Features
========

* **Easy to integrate** within any existing verilog testbench (only :ref:`one
  system task <sec_verisocks_init>` to be called within the top-level
  ``initial`` statement)
* Verisocks interfaces with the simulator simply via a standard **TCP socket**
  with a :ref:`JSON-based protocol <sec_tcp_protocol>`
* Using this interface, Verisocks allows to **control the simulation** and
  probe its status by:

  * instructing the simulator to run for a given amount of time,
  * instructing the simulator to run until a given simulation time,
  * instructing to run the simulator until a certain event,
  * reading or writing simulator variables,
  * getting the current simulation time,
  * etc...


.. toctree::
  :maxdepth: 2
  :caption: Contents

  src/intro
  src/architecture
  src/tcp_protocol
  src/python_client


.. toctree::
  :titlesonly:

  src/release_notes


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
