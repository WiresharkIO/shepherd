## Cython Compiled Lib

This directory contains setup for compiling the virtual converter / harvester / source as a shared library and use it in python to:
- compare it to reference implementation
- enable unittesting of sub-functions

### Install prerequisites

choose what is needed

```Shell
cd ./software/firmware/pru0_cython_module

pip3 install pipenv

pipenv shell
```

**Note**: a local shepherd-installation is needed. Pipenv is installing it. Otherwise run:

```Shell
cd ./software/python-package
pip3 install ./
```

### Compile and install Module

```Shell
python3 setup.py build_ext
# or just
pip3 install ./
```

### Run the testing-scratchpad

```Shell
python3 testing.py
```

### To Run Pytest

Since pytest doesn't recognize Cython(.pyx) directly, before importing (.pyx) in test_module.py :

Install - Pytest
```shell
pip install pytest-cython
```

To Run - Pytest
```Shell
pytest-3 test_module.py
```

### Cleanup

This should be the correct command, but it fails to clean up some pieces.

```Shell
python setup.py clean --all
```

### TODO

done:
- replaced distutils by setuptools (as distutils are deprecated)
- implemented some compile-constants alter behavior of c-code (no hw-dependency)
- used established folder-structure ... this code can now live in shepherd/software/firmware/pru0-cython-module
- switched to cython-prerelease to allow "volatile" (pipenv usage)
- use py3 code in cython -> language_level=... in .pyx file
- more detailed / explicit cython-config in setup.py
- some more FNs uncommented (/activated) in .pyx/pxd
- changed import in pyx-file to "cimport hvirtual_converter" to allow py-FNs with same name as c-FNs
  - are same names wanted??? this change was just a guess
- wrote a readme to help using the code
- ... lib compiles

- Fixed the earlier issues with testing.py:
	- added few cdefs[from "calibration.h", "math64_safe.h"]
	- added corresponding .c(paths) in setup.py and few definitions in .pxy
	- Had to remove 'const' from - "uint32_t msb_position(const uint32_t value)" in math64_safe.c to make
	  the cython build possible
- Fixed issues with structure by:
	- adding cdefs
	- introducing class to handle it as a python object
	- see second point in todo

- made setup.py more modular and explicit
- transformed .pyx-file into class -> still without data-transformations
- updated install-instructions
- guessed the interface of the module for testing.py
- Add Virtual_Harvester - To be added in setup.py
- Included wrapper functions in virtual_converter.c to access static functions in C in .pyx

todo:
- Implement pytest for all the functions, and compare it with a credible root function (instead of Magic-values).
- While the format of "structures" in .pyx cythonizes, however when it runs through the python interpreter during testing, incurs incompatibility.
- Add Virtual_Harvester, functions
- Review of the created wrapper functions, since it altered source file(.c)
