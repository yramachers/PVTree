#!/bin/bash

echo "For $HOSTNAME trying to find appropriate compilers."
if [[ "$HOSTNAME" == "epp-ui01" || "$HOSTNAME" =~ lxplus[0-9]+.cern.ch ]];
then
    # This is a bit squiffy
    echo "Setting up compilers installed manually on cluster."

    #For cmake point at these compilers
    export CC=$(which gcc-4.9.2)
    export CXX=$(which g++-4.9.2)
    export FC=$(which gfortran-4.9.2)
else
    echo "Using standard compilers."

    #For cmake point at these compilers
    export CC=/usr/bin/gcc-4.9
    export CXX=/usr/bin/g++-4.9
    export FC=/usr/bin/gfortran-4.9
fi

