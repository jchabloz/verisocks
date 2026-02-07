DOXYGEN      ?= doxygen
DOXYGEN_OPTS ?=

docs: Doxyfile
	$(DOXYGEN) $(DOXYGEN_OPTS) $<

.PHONY: docs

