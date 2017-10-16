#!/bin/sh

startDirectory=$1
jobName=$2
outputFileDirectory=$3

#Parameters for yearlyForestScan job
treeType=$4
leafType=$5
nsimulations=$6
treesPerForest=$7
maximumTreeTrials=$8
timeSegments=$9
photonsPerTimeSegment=${10}
geant4SeedOffset=${11}
parameterSeedOffset=${12}
startDate=${13}
endDate=${14}
yearSegments=${15}
minimumSensitiveArea=${16}


#Now using job arrays so get index from env
#and then get the j-th input file
jobCounter=$LSB_JOBINDEX
echo "Sub-Job number ${jobCounter}"

#Make a working directory under tmp
workingDir=/tmp/phslag/${jobName}/${jobCounter}/
mkdir -p ${workingDir}
cd ${workingDir}

#Copy important files
cp -v ${startDirectory}/../build/BuildProducts/bin/pvtree-yearlyForestScan .

#Is everything here?
ls -la

#Run the yearly tree scan job
currentGeant4Seed=$[geant4SeedOffset+jobCounter]
currentParameterSeed=$[parameterSeedOffset+jobCounter]

./pvtree-yearlyForestScan --tree ${treeType} \
    --leaf ${leafType} \
    --simulations ${nsimulations} \
    --treeNumber ${treesPerForest} \
    --timeSegments ${timeSegments} \
    --photonNumber ${photonsPerTimeSegment} \
    --geant4Seed ${currentGeant4Seed} \
    --parameterSeed ${currentParameterSeed} \
    --startDate ${startDate} \
    --endDate ${endDate} \
    --yearSegments ${yearSegments} \
    --outputFileName "yearlyForestScan.results.root" \
    --minimumSensitiveArea ${minimumSensitiveArea} \
    --maximumTreeTrials ${maximumTreeTrials}


#Copy the output root file
if [ -f yearlyForestScan.results.root ]; then
    cp -v yearlyForestScan.results.root ${outputFileDirectory}/yearlyForestScan.results.${jobCounter}.root
else
    echo "No analysis output to save, so job FAILED!" 
fi

#Clean up
cd ${startDirectory}
rm -v -rf ${workingDir}

