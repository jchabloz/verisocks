.. _sec_python_client:

Python client
#############

A reference Python client is provided with the source repository of Verisocks.
It provides an easy way to connect to the Verisocks server and to format and
send requests to it in accordance with the defined :ref:`TCP protocol
<sec_tcp_protocol>`.

The entire client is encapsulated into the :py:class:`Verisocks
<verisocks.verisocks.Verisocks>` class, which can easily be instantiated as in
the following example:

.. code-block:: python

    from verisocks.verisocks import Verisocks

    vs = Verisocks("127.0.0.1", 5100)
    vs.connect()
    vs.run("for_time", time=3.2, time_unit="us")
    vs.close()


The :py:class:`Verisocks <verisocks.verisocks.Verisocks>` class comes with
proper context manager methods, allowing a safer, more pythonic, way to
instantiate it. The context manager will automatically connect to the socket
and close it when done. The code of the previous example can thus be
equivalently written as:

.. code-block:: python

    from verisocks.verisocks import Verisocks

    with Verisocks("127.0.0.1", 5100) as vs:
        vs.run("for_time", time=3.2, time_unit="us")


Python client API documentation
*******************************

.. automodule:: verisocks.verisocks
    :members:


Miscellaneous utilitaries
*************************

The module :py:mod:`verisocks.utils` is a collection of miscellaneous
utilitaries and helper functions to ease setting up simulations using
Verisocks.

.. automodule:: verisocks.utils
    :members:
