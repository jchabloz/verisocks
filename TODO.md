# Verisocks TODO list

* [x] Add get type
* [x] Add command to read memory arrays
* [x] Split `vs_vpi.c` in several files for easier readability.
* [x] Manage end of simulation when a message is still expected to be returned.
  For example, this could happen due to an error or because the simulation has
  finished while a registered callback condition with the run command has not
  yet been reached.
* [x] Add command *set* to be able to force values, supporting the same object
  types as for the command *get*.
* [ ] Improve unit test coverage, using appropriately mock/fake functions
  (using `fff.h`?).
* [x] Add timeout as an (optional) parameter for $verisocks_init()
* [x] Python packaging config
* [ ] ~~Move Python Verisocks class back to `__init__.py`~~
* [x] Add testing for Python module
* [x] Use autotools make compilation better portable
* [x] Change makefile and source to cope with a different, more generic
  installation path for `vpi_user.h`
* [ ] Create a few examples (WIP)
* [ ] Documentation (WIP)
* [ ] Attempt to compile Verisocks and run it with Cadence Xcelium (WIP)
* [ ] Add support for callbacks on multiple events/value changes in // and also
  for the possibility to keep the callback enabled after it has triggered.
* [ ] Port for usage with Verilator. This won't be such an easy feat...
* [ ] Add a `"context"` field to the JSON message which is sent by
  `vs_vpi_return()` function.
