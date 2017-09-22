PVTree simulation code
=======================

Authors:  Graham Jones, Yorck Ramachers
Contributor: Kieran Uzzell


External dependencies
=====================
- *Required*
  - C++11 compatible compiler (GCC >= 4.9, Clang >= 3.5)
  - Fortran compiler
  - [CMake](https://cmake.org) >= 3.5
  - [ROOT](https://root.cern.ch) >= 6.8
    - With Python and GSL (MathMore) support
  - [Geant4](https://geant4.cern.ch) >= 10.3
    - Requires support for Qt4/5 if PVTree visualization is required
  - [Libconfig](http://www.hyperrealm.com/libconfig/)
  - [ECCodes](https://software.ecmwf.int/wiki/display/ECC/ecCodes+Home) (for reading climate data files)
- *Optional*
  - [Doxygen](http://doxygen.org) for building documentation
  - [CPPCheck](https://sourceforge.net/projects/cppcheck/) for static analysis during development
  - [EO](https://sourceforge.net/projects/eodev/) for future optimisation, not currently used

ROOT, Geant4, Libconfig, together with all of their C++ dependencies must be compiled
against the C++11 standard to ensure binary compatibility and correct genreration
of ROOT dictionaries.

It is expected that PVTree can be compiled and run on all platforms where the above
dependencies can be installed, which are primarily Linux and macOS.

On University of Warwick development systems, all of the above packages
are available through the `pvtree.devel` Environment Module. Run

```console
$ module use --append /warwick/epp/modules
$ module load brews/pvtree.devel
```

to set everything up, or add these commands to your shell configuration
file.



Quick Start-up
==============
Ensure all the dependencies listed above are available, and available in the session `PATH`

``` console
$ git clone <url> pvtree.git
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=<installpath> ../pvtree.git
...
-- Configuring done
-- Generating done
-- Build files have been writen to: /.../build
```

The `<installpath>` should be a directory to which you have write permissions. If you
do not set it, it defaults to `/usr/local`. If all of the dependencies are in paths known
to CMake, they should be found automatically. Should any errors occur when running `cmake`
due to unfound packages, you may need to add their install locations to `CMAKE_PREFIX_PATH`
either in the environment or via `cmake -DCMAKE_PREFIX_PATH="path1;path2;...;pathN" <args>`.

With newer versions of CMake and ROOT, you will see a warning about
CMake policies from the `RootNewMacros.cmake` module. This can be
ignored.

Once configuration has run successfully, PVTree can be built via:

``` console
$ make -j4
```

Adjust the numeric argument to `make` as appropriate for the cores on your
build machine. The PVTree programs and libraries are output into the `BuildProducts`
directory under the build directory:

```console
+- BuildProducts/
   +- bin/
   |  +- pvtree-XXX
   + lib/
     +- PVTree/
        + lib...
```

and may be run directly (NB: you will need to set the `PVTREE_CLIMATE_DATA_PATH`
environment variable to point to the climate data you wish to use). This is the
recommended way to run if you are developing or studying the code. Tests can be
run using

```console
$ make test
```

or to get more information:

```console
$ ctest -VV
```

You may also install the programs, libraries and configuration files by running:

```console
$ make install
```

All PVTree resources are installed into a standard filesystem hierarchy:

```
+- CMAKE_INSTALL_PREFIX/
   +- bin/
   |  +- pvtree-*
   +- lib/
   |  +- PVTree/
   |     +- libpvtree-*
   +- share/
      +- doc/
      |  +- PVtree/
      +- PVTree/
         +- config/
         +- smarts/
         +- spectra/
```

After installation, you may need to set `LD_LIBRARY_PATH` on Linux systems to point
the `pvtree-*` executables to their dependent libraries such as ROOT and Geant4.
Alternately, you can add the argument `-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON` when
running cmake and before installing. This will set the RPATH of the executables to point
to all the used libraries.

To get started with using PVTree programs, open the file `share/doc/PVTree/html/index.html`
in your preferred browser.




Obtaining Climate Data
======================
If the climate data path no longer exists you will have download the files again
from the [ECMWF website](http://apps.ecmwf.int/datasets/). The query 
specifications are listed in the configuration files `config/climate/XXX.cfg`. 
Grib files get big and the website has a limit so you will have to download in 
smaller time periods and then merge the files (can do a simple cat with grib 
files). Instructions on retrieving data from ECMWF is provided on their 
[wiki](https://software.ecmwf.int/wiki/display/WEBAPI/How+to+retrieve+ECMWF+Public+Datasets).



Developing PVTree
=================

Pull requests for new functionality and/or bug fixes are welcome. Please ensure your
work is done on a branch from the `master`, e.g.

```
$ git checkout -b myfix master
```

Collaborators can publish this branch direct to the authoratative repo:

```
$ git push -u origin myfix
```

and use this to create a Pull Request for review.

New features or identified bugs should have accompanying tests (in the latter case
to first reproduce the bug and then demonstrate the fixe resolves it). PVTree tests
use the [Catch](https://github.com/philsquared/Catch) framework for suites and cases. 
A "catch main" program is supplied, so you only need [write the needed test cases](https://github.com/philsquared/Catch/blob/master/docs/tutorial.md). 
All tests should be placed in the `pvtree/tests`
directory, and the `pvtree/tests/CMakeLists.txt` script shows how to compile, link and
add the tests so that they can be run by `ctest` (or `make test`).


PVTree supports, but does not yet fully implement performance and static analysis
testing. This is a WIP:
_For development, a debug build can be created using the additional cmake option
-DCMAKE_BUILD_TYPE=DEBUG. This will turn off compiler optimization and
it will enable code coverage to be run (with 'make test_coverage'). When running
the executables produced by the debug build the allowed virtual memory usage
must be increased (by tools like ulimit) because of the undefined behaviour and
address sanitizers. The debug build requires both lcov and gcov to be present in
the path (gcov needs to be from the same compiler suit)._


Licensing
=========
Please study the file ``LICENSE.txt`` for the distribution terms and
conditions of use of PVTree.

Climate data from ECMW under CC BY-NC-ND 4.0, http://creativecommons.org/licenses/by-nc-nd/4.0/


