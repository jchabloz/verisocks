# Verisocks - *A generic socket interface for Icarus*

[Github](https://github.com/jchabloz/verisocks)

## Introduction

When using an HDL simulator such as [Icarus](http://iverilog.icarus.com) which
provides only a limited possible support for advanced verification
methodologies, the following challenges have to be faced:

* How to define and perform regression tests?
* How to define and execute simple or complex pass/fail criteria?
* How to establish traceability between testcases and requirements?
* etc.

While these issues could be adressed with advanced verification methodologies
and frameworks such as
[UVM](https://en.wikipedia.org/wiki/Universal_Verification_Methodology), it is
relatively difficult to put into place with Icarus.

The idea that I propose to explore here comes out of my trying to use the
[cocotb](https://docs.cocotb.org) environment to meet some of the needs listed
above. The *cocotb* tool, is described as a *Python verification framework* and
uses extensively asynchronous I/O techniques (hence the name, which stands for
*coroutine based cosimulation testbench*). I made mine the statement that can
be found on the [cocotb documentation main
page](https://docs.cocotb.org/en/stable/):

> All verification is done using Python which has various advantages over using
> SystemVerilog or VHDL for verification:
> 
> * Writing Python is fast - it’s a very productive language.
> * It’s easy to interface to other languages from Python.
> * Python has a huge library of existing code to re-use.
> * Python is interpreted - tests can be edited and re-run without having to
>   recompile the design.
> * Python is popular - far more engineers know Python than SystemVerilog or
>   VHDL.

I soon realized that it was indeed really nice to be able to define and execute
tests using Python! I was however a bit disappointed by the so-called
*regression manager* implementation proposed by cocotb and started to wonder
why redo what is already done so well (and much more completely) by tests
frameworks such as [pytest](https://docs.pytest.org) or
[robot](https://robotframework.org)?

Some workaround can be found, such as for example
[cocotb-test](https://github.com/themperek/cocotb-test) which proposes a
solution to use pytest with cocotb, but I am not entirely convinced.

I prefer trying another solution (it's more fun anyway); re-write from scratch
a simple-to-use interface using the standardized *Verilog Procedural Interface
(VPI)* to make it possible to control an Icarus simulation (this could normally
easily be extended to any Verilog simulator later) from Python.

## Requirements

Let's try and draft some requirements for the desired solution I have in mind.

* It shall be possible to control the HDL simulator with Python, either
  interactively or with scripts.
* It shall be possible to easily interface the HDL simulator with any higher
  level testing and/or automation frameworks (e.g. pytest, robot, etc.)
* Defining tests (stimuli, configuration, etc.) should be done as much as
  possible from the chosen high-level framework, without requiring any tweaking
  of the HDL description.
* Assessing test pass/fail criteria (assertions) shall be performed from the
  chosen high-level framework.

Bonus:

* It should be easy to add support for any other high-level (scripting)
  language.
* The solution should be compatible with parallel testing.

## Proposed architecture

TODO

## TCP protocol

### Message format

The TCP message format follows the proposal for a message format in the
RealPython tutorial on sockets programming as it seems to be quite reasonable
and generic. Indeed, it allows to deal easily with variable-length messages
while making sure that we can easily verify the that the full message has been
received and/or is not overlapping with the next message.

1. Fixed-length pre-header
   * Type: Integer value
   * Encoding: Big endian byte ordering
   * Length: 2 bytes
2. Variable-length header
   * Type: Unicode text
   * Encoding: UTF-8
   * Length: As specified by the integer value in the pre-header
3. Variable-length payload
   * Type: As specified in the header
   * Encoding: As specified in the header
   * Length: As specified in the header

## References

1. Ultralightweight JSON parser in ANSI C: https://github.com/DaveGamble/cJSON#readme