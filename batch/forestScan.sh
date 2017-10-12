#!/bin/sh

#source ./forestScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${geant4SeedOffset} ${parameterSeedOffset} ${treesPerForest} ${timeSegments} ${photonsPerTimeSegment} ${treeType} ${leafType}
startDirectory=$1
jobName=$2
outputFileDirectory=$3
geant4SeedOffset=$4
parameterSeedOffset=$5
treesPerForest=$6
timeSegments=$7
photonsPerTimeSegment=$8
nsimulations=$9
treeType=${10}
leafType=${11}

#Now using job arrays so get index from env
#and then get the j-th input file
jobCounter=$LSB_JOBINDEX
echo "Sub-Job number ${jobCounter}"

#Make a working directory under tmp
workingDir=/tmp/phsdaq/${jobName}/${jobCounter}/
mkdir -p ${workingDir}
cd ${workingDir}

#Copy important files
cp -v ${startDirectory}/../../Install/bin/pvtree-forestScan .

#Is everything here?
ls -la

#Run the tree scan job
currentGeant4Seed=$[geant4SeedOffset+jobCounter]
currentParameterSeed=$[parameterSeedOffset+jobCounter]
./pvtree-forestScan --tree ${treeType} \
    --leaf ${leafType} \
    --simulations ${nsimulations} \
    --geant4Seed ${currentGeant4Seed} \
    --parameterSeedOffset ${currentParameterSeed} \
    --treeNumber ${treesPerForest} \
    --timeSegments ${timeSegments} \
    --photonNumber ${photonsPerTimeSegment}

#Copy the output root file
if [ -f forestScan.results.root ]; then
    cp -v forestScan.results.root ${outputFileDirectory}/forestScan.results.${jobCounter}.root
else
    echo "No analysis output to save, so job FAILED!" 
fi

#Clean up
cd ${startDirectory}
rm -v -rf ${workingDir}

