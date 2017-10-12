#!/bin/sh

#Always run this script from the batch directory otherwise pathing goes awry
jobName=YearForest_HelicalCordate
jobNumber=500
simultaneousJobNumber=$jobNumber
OutputFileDirectory=/data/detdev/$USER/PVTree/AnalysisResults/$jobName
OutputLogDirectory=/data/detdev/$USER/PVTree/AnalysisLogs/$jobName

treeType=helical
leafType=cordate
nsimulations=10
treesPerForest=25
maximumTreeTrials=100000
timeSegments=12
photonsPerTimeSegment=100000
geant4SeedOffset=3300
parameterSeedOffset=3300
startDate=1/1/2014
endDate=1/1/2015
yearSegments=48
minimumSensitiveArea=0.9

#Make sure there is an output directory
mkdir -p $OutputFileDirectory
mkdir -p $OutputLogDirectory

#Submit an appropriately sized job array
bsub -o ${OutputLogDirectory}/%J_%I.out \
    -q "xlong" \
    -J "$jobName[1-${jobNumber}]%${simultaneousJobNumber}" \
    source ./yearlyForestScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${treeType} ${leafType} ${nsimulations} ${treesPerForest} ${maximumTreeTrials} ${timeSegments} ${photonsPerTimeSegment} ${geant4SeedOffset} ${parameterSeedOffset} ${startDate} ${endDate} ${yearSegments} ${minimumSensitiveArea}
