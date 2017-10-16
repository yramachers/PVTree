#!/bin/sh

startDirectory=$1
jobName=$2
outputFileDirectory=$3

#Parameters for yearlyTreeScan job
treeType=$4
leafType=$5
treesPerJob=$6
maximumTreeTrials=$7
timeSegments=$8
photonsPerTimeSegment=$9
geant4SeedOffset=${10}
parameterSeedOffset=${11}
startDate=${12}
endDate=${13}
yearSegments=${14}
minimumSensitiveArea=${15}


#Now using job arrays so get index from env
#and then get the j-th input file
jobCounter=$LSB_JOBINDEX
echo "Sub-Job number ${jobCounter}"

#Make a working directory under tmp
workingDir=/tmp/phslag/${jobName}/${jobCounter}/
mkdir -p ${workingDir}
cd ${workingDir}

#Copy important files
cp -v ${startDirectory}/../build/BuildProducts/bin/pvtree-yearlyTreeScan .

#Is everything here?
ls -la

#Run the yearly tree scan job
currentGeant4Seed=$[geant4SeedOffset+jobCounter]
currentParameterSeed=$[parameterSeedOffset+jobCounter]

./pvtree-yearlyTreeScan --tree ${treeType} \
    --leaf ${leafType} \
    --treeNumber ${treesPerJob} \
    --timeSegments ${timeSegments} \
    --photonNumber ${photonsPerTimeSegment} \
    --geant4Seed ${currentGeant4Seed} \
    --parameterSeed ${currentParameterSeed} \
    --startDate ${startDate} \
    --endDate ${endDate} \
    --yearSegments ${yearSegments} \
    --outputFileName "yearlyTreeScan.results.root" \
    --minimumSensitiveArea ${minimumSensitiveArea} \
    --maximumTreeTrials ${maximumTreeTrials}


#Copy the output root file
if [ -f yearlyTreeScan.results.root ]; then
    cp -v yearlyTreeScan.results.root ${outputFileDirectory}/yearlyTreeScan.results.${jobCounter}.root
else
    echo "No analysis output to save, so job FAILED!" 
fi

#Clean up
cd ${startDirectory}
rm -v -rf ${workingDir}

