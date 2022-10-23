# Testing - Some notes

## Unit test framework

CUnit: http://cunit.sourceforge.net

I am using the system-installed version (Ubuntu).

```bash
sudo apt install libcunit1-dev
```

Related note: [How to view CUnit-generated XML output files](HOWTO_CUnit_xml.md)

## Mock functions framework

Fake Function Framework: https://github.com/meekrosoft/fff

Only the file `fff.h` is copied from the repository


## Other tools

* `valgrind`
* Coverage analysis: `lcov`
