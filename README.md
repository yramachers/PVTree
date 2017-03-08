=======================
PVTree simulation code
=======================

Authors:  Graham Jones, Yorck Ramachers
Contributor: Kieran Uzzell


Licensing
---------
Please study the file ``LICENSE.txt`` for the distribution terms and
conditions of use of PVTree.


Quick Start-up 
=============

Installation Instructions
-------------------------------

1. Start by sourcing the environment setup script within this directory

> source setupEnvironment.sh

which will set the environment variables controlling the choice of 
compiler used during the build. Currently GCC 4.9.X+ is required.

2. Create a new build and install directory

> mkdir Build
> mkdir Install

3. From the build directory use cmake to configure the PVTree source 
tree. This will also include the installation of a number of external
packages called Geant4, ROOT, libconfig and Evolutionary Objects. 

> cmake -DCMAKE_INSTALL_PREFIX=${PWD}/../Install/ -DPVTREE_SOURCE_DIRECTORY=../ -DPVTREE_CLIMATE_DATA_PATH=/my/data/storage/location/ClimateData/ ../super/

You should check there are no error messages during this step. Recent versions
of ROOT require python installations of version 2.7 and greater. Will need
to specify a python exe by hand if the default version does not meet the criteria.
Newer versions of ROOT might be better as python is supposidly disabled for the 
ROOT super-build (a requirement that doesn't seem to work at the moment.).

If the climate data path no longer exists you will have download the files again
from the ECMWF website. The query specifications are listed in the configuration
files config/climate/*.cfg. Grib files get big and the website has a limit so you
will have to download in smaller time periods and then merge the files (can do a
simple cat with grib files).

4. Compile all the code using make. This may take a long time (about one
hour when compiling with four cores) when first compiling as the external 
packages Geant4 and ROOT are both quite large.

> nice make -j4

where `-j4' indicates that up to four cores can be simultaneously used
in the compilation. You should change this number depending exactly on
what computer you are running on. During the compilation of the PVTree
library CPPCheck will apply static analysis to the code, where occasionally
spurious errors can be found (but it does not prevent compilation).

Alternatively a debug build can be created using the additional cmake option
-DCMAKE_BUILD_TYPE=DEBUG. This will turn off compiler optimization and
it will enable code coverage to be run (with 'make test_coverage'). When running
the executables produced by the debug build the allowed virtual memory usage 
must be increased (by tools like ulimit) because of the undefined behaviour and
address sanitizers. The debug build requires both lcov and gcov to be present in 
the path (gcov needs to be from the same compiler suit).


Running Example Programs
------------------------

1. Source the environment setup script from within the install directory

> source Install/share/scripts/setup.sh

which will setup the current environment variables for Geant4, ROOT and
also the PVTree code.

2. Execute one of the example programs now present in the PATH environment

> basicSimulate

3. Look at the output! 


Compiling DOxygen Documentation
-------------------------------

Documentation of the code can be automatically generated from the build 
directory of the PVTree software by running. 

> cd Build/PVTree/src/SuperPVTree-build/
> make doc
> make install

To browse the generated documentation point your browser at the html file 
within your install path  Install/share/doc/html/index.html



