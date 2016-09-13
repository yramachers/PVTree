#!/bin/sh

#Always run this script from the batch directory otherwise pathing goes awry
#Now have a floor!
jobName=Scan_StochasticCordate_YR_V1
jobNumber=200
simultaneousJobNumber=$jobNumber
OutputFileDirectory=/data/detdev/$USER/PVTree/AnalysisResults/$jobName
OutputLogDirectory=/data/detdev/$USER/PVTree/AnalysisLogs/$jobName

geant4SeedOffset=200
parameterSeedOffset=1
treesPerJob=100
timeSegments=24
photonsPerTimeSegment=100000
treeType=stochastic
leafType=cordate

#Make sure there is an output directory
mkdir -p $OutputFileDirectory
mkdir -p $OutputLogDirectory

#Submit an appropriately sized job array
bsub -o ${OutputLogDirectory}/%J_%I.out \
    -q "xlong" \
    -J "$jobName[1-${jobNumber}]%${simultaneousJobNumber}" \
    source ./treeScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${geant4SeedOffset} ${parameterSeedOffset} ${treesPerJob} ${timeSegments} ${photonsPerTimeSegment} ${treeType} ${leafType}

    
