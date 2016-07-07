#!/bin/sh

#source ./treeScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${geant4SeedOffset} ${parameterSeedOffset} ${treesPerJob} ${timeSegments} ${photonsPerTimeSegment} ${treeType} ${leafType}
startDirectory=$1
jobName=$2
outputFileDirectory=$3
geant4SeedOffset=$4
parameterSeedOffset=$5
treesPerJob=$6
timeSegments=$7
photonsPerTimeSegment=$8
treeType=$9
leafType=${10}

#Now using job arrays so get index from env
#and then get the j-th input file
jobCounter=$LSB_JOBINDEX
echo "Sub-Job number ${jobCounter}"

#Make a working directory under tmp
workingDir=/tmp/phslag/${jobName}/${jobCounter}/
mkdir -p ${workingDir}
cd ${workingDir}

#Copy important files
cp -v ${startDirectory}/../../bin/treeScan .

#Is everything here?
ls -la

#Run the tree scan job
currentGeant4Seed=$[geant4SeedOffset+jobCounter]
currentParameterSeed=$[parameterSeedOffset+jobCounter*treesPerJob]
./treeScan --tree ${treeType} \
    --leaf ${leafType} \
    --geant4Seed ${currentGeant4Seed} \
    --parameterSeedOffset ${currentParameterSeed} \
    --treeNumber ${treesPerJob} \
    --timeSegments ${timeSegments} \
    --photonNumber ${photonsPerTimeSegment}

#Copy the output root file
if [ -f treeScan.results.root ]; then
    cp -v treeScan.results.root ${outputFileDirectory}/treeScan.results.${jobCounter}.root
else
    echo "No analysis output to save, so job FAILED!" 
fi

#Clean up
cd ${startDirectory}
rm -v -rf ${workingDir}

