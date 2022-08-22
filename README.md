# Verisocks - A generic verification interface

[Github](https://github.com/jchabloz/verisocks)

## TODO

- [ ] Learn how sockets are working
- [ ] Learn VPI and how to use it with Icarus verilog


## TCP protocol

### Message format

The TCP message format follows the proposal for a message format in the
RealPython tutorial as it seems to be quite reasonable and generic. Indeed, it
allows to deal easily with variable-length messages while making sure that we
can easily verify the that the full message has been received and/or is not
overlapping with the next message.

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

1. A tutorial on sockets in C: https://www.binarytides.com/socket-programming-c-linux-tutorial/
2. RealPython guide on sockets programming: https://realpython.com/python-sockets/
3. GNU libc manual: https://www.gnu.org/software/libc/manual/html_mono/libc.html
4. GNU C reference manual: https://www.gnu.org/software/gnu-c-manual/gnu-c-manual.html
5. Ultralightweight JSON parser in ANSI C: https://github.com/DaveGamble/cJSON#readme