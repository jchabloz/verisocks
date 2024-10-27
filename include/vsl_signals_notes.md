# Notes on how to access signals/variables

## Variables

For each model/context, Verilator defines *scopes* for public variables, which
seem to correspond to the design hierachical levels in which some variables are
declared as being public. The existing scopes for a given verilated design can
be found by name using the ``VerilatedContext::scopeFind()`` function.

The name of the top-level scope corresponds to the name returned by the
``VerilatedModel::hierName()`` function (to be confirmed).

Variables which are public in a given scope can then be found using the
``VerilatedScope::varFind()`` function. The ``varFind()`` and ``scopeFind()``
functions are declared as public in their respective classes, however
considered as reserved for "internal use" and thus not as being part of the
public API. Using them for our needs should thus be done with caution and
careful monitoring to ensure back-compatibility in future Verilator's releases.

Note that a public function which runs an iterative loop on all scopes and
variables in a given context exists, called
``VerilatedContext::internalDumps()``, which will dump them on stdout. This can
be used to verify exact path names and which variables are available in a given
context.

The class ``VerilatedVar`` is used to store information about variables and is
derived from the base class ``VerilatedVarProps``. The function ``varFind()``
returns a pointer to an instance of this class.

### Public methods for VerilatedVar:

- ``void* datap()`` - Returns a pointer to the actual variable. Note that the
  current implementation of Verilator uses void pointers, which is not fully
  type safe! A much better choice would probably have been to use
  ``std::variant`` (or ``std::any``) objects with appropriate casting. Since
  supporting the ``--timing`` requires anyway a compiler that supports C++20,
  we might as well use these features (part of the std lib from C++17).
- ``const char* name()``
- ``bool isParam()``

#### Methods inherited from VerilatedVarProps base class:

- ``VerilatedVarType vltype()``
- ``VerilatedVarFlags vldir()``
- ``uint32_t entSize()``
- ``bool isPublicRW()``
- ``int udims()``
- ``int dims()`` (= pdims + udims)
- ``const VerilatedRange& packed()``
- ``const VerilatedRange& unpacked()``
- ``int left(int dim)``
- ``int right(int dim)``
- ``int low(int dim)``
- ``int high(int dim)``
- ``int increment(int dim)``
- ``int elements(int dim)``
- ``size_t totalSize()``
- ``void* datapAdjustIndex(void* datap, int dim, int indx)``

### Notion of dimensions

The property ``dims`` allows to distinguish the following arrangements:
- 0: Single-bit variable
- 1: Multiple-bit variable
- 2 or more: Array

Further more, the so-called *packed* and *unpacked* dimensions are defined as
follows:
- *packed* dimensions (``pdims``): First level dimension (1- or n-bit variable)
- *unpacked* dimensions (``udims``): Arrays dimensions (higher-levels)
- the packed and unpacked ranges are also aware of the order in which the
  elements should be accessed (variables ``x[6:0]`` vs ``x[0:6]``).

