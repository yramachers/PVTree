#!/bin/sh

#Always run this script from the batch directory otherwise pathing goes awry
#Now have a floor!
jobName=Scan_TernaryCordate_Spectrum_V4
jobNumber=200
simultaneousJobNumber=$jobNumber
OutputFileDirectory=/data/atlas/$USER/PVTree/AnalysisResults/$jobName
OutputLogDirectory=/data/atlas/$USER/PVTree/AnalysisLogs/$jobName

geant4SeedOffset=1
parameterSeedOffset=1
treesPerJob=100 
timeSegments=25
photonsPerTimeSegment=10000 
treeType=ternary
leafType=cordate

#Make sure there is an output directory
mkdir -p $OutputFileDirectory
mkdir -p $OutputLogDirectory

#Submit an appropriately sized job array
bsub -o ${OutputLogDirectory}/%J_%I.out \
    -q "long" \
    -J "$jobName[1-${jobNumber}]%${simultaneousJobNumber}" \
    source ./treeScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${geant4SeedOffset} ${parameterSeedOffset} ${treesPerJob} ${timeSegments} ${photonsPerTimeSegment} ${treeType} ${leafType}

    