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
environment variable to pint to the climate data you wish to use).

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

To get started with using PVTree programs, open the file `share/doc/PVTree/html/index.html`
in your preferred browser.

For development, a debug build can be created using the additional cmake option
-DCMAKE_BUILD_TYPE=DEBUG. This will turn off compiler optimization and
it will enable code coverage to be run (with 'make test_coverage'). When running
the executables produced by the debug build the allowed virtual memory usage
must be increased (by tools like ulimit) because of the undefined behaviour and
address sanitizers. The debug build requires both lcov and gcov to be present in
the path (gcov needs to be from the same compiler suit).


Obtaining Climate Data
======================
If the climate data path no longer exists you will have download the files again
from the ECMWF website. The query specifications are listed in the configuration
files `config/climate/XXX.cfg`. Grib files get big and the website has a limit so you
will have to download in smaller time periods and then merge the files (can do a
simple cat with grib files).



Licensing
=========
Please study the file ``LICENSE.txt`` for the distribution terms and
conditions of use of PVTree.

Climate data from ECMW under CC BY-NC-ND 4.0, http://creativecommons.org/licenses/by-nc-nd/4.0/


