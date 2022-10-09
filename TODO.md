# Verisocks TODO list

* [x] Add get type
* [x] Add command to read memory arrays
* [x] Split `vs_vpi.c` in several files for easier readability.
* [x] Manage end of simulation when a message is still expected to be returned.
  For example, this could happen due to an error or because the simulation has
  finished while a registered callback condition with the run command has not
  yet been reached.
* [ ] Add command *set* to be able to force values, supporting the same object
  types as for the command *get*.
* [ ] Improve unit test coverage, using mock functions.
* [ ] Move Python Verisocks class back to `__init__.py`
* [ ] Add unit tests for Python module
* [ ] Create a few examples
* [ ] Documentation