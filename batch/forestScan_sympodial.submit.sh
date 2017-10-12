#!/bin/sh

#Always run this script from the batch directory otherwise pathing goes awry
jobName=DayForest_SympodialCordate
jobNumber=100
simultaneousJobNumber=$jobNumber
OutputFileDirectory=/data/detdev/$USER/PVTree/AnalysisResults/$jobName
OutputLogDirectory=/data/detdev/$USER/PVTree/AnalysisLogs/$jobName

geant4SeedOffset=200
parameterSeedOffset=200
nsimulations=50
treesPerForest=25
timeSegments=12
photonsPerTimeSegment=100000
treeType=sympodial
leafType=cordate

#Make sure there is an output directory
mkdir -p $OutputFileDirectory
mkdir -p $OutputLogDirectory

#Submit an appropriately sized job array
bsub -o ${OutputLogDirectory}/%J_%I.out \
    -q "long" \
    -J "$jobName[1-${jobNumber}]%${simultaneousJobNumber}" \
    source ./forestScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${geant4SeedOffset} ${parameterSeedOffset} ${treesPerForest} ${timeSegments} ${photonsPerTimeSegment} ${nsimulations} ${treeType} ${leafType}

