#!/bin/sh

#Always run this script from the batch directory otherwise pathing goes awry
#Now have a floor!
jobName=Scan_MonopodialCordate_YR_VMLeaf
jobNumber=100
simultaneousJobNumber=$jobNumber
OutputFileDirectory=/data/detdev/$USER/PVTree/AnalysisResults/$jobName
OutputLogDirectory=/data/detdev/$USER/PVTree/AnalysisLogs/$jobName

geant4SeedOffset=400
parameterSeedOffset=400
treesPerJob=50 
timeSegments=12
photonsPerTimeSegment=100000 
treeType=monopodial
leafType=cordate

#Make sure there is an output directory
mkdir -p $OutputFileDirectory
mkdir -p $OutputLogDirectory

#Submit an appropriately sized job array
bsub -o ${OutputLogDirectory}/%J_%I.out \
    -q "long" \
    -J "$jobName[1-${jobNumber}]%${simultaneousJobNumber}" \
    source ./treeScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${geant4SeedOffset} ${parameterSeedOffset} ${treesPerJob} ${timeSegments} ${photonsPerTimeSegment} ${treeType} ${leafType}

    
