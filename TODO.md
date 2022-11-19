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
* [ ] Improve unit test coverage, using appropriately mock/fake functions.
* [x] Add timeout as an (optional) parameter for $verisocks_init()
* [x] Python packaging config
* [ ] Move Python Verisocks class back to `__init__.py` (?)
* [x] Add testing for Python module
* [x] Use autotools make compilation better portable
* [x] Change makefile and source to cope with a different, more generic
  installation path for `vpi_user.h`
* [ ] Create a few examples (WIP)
* [ ] Documentation
