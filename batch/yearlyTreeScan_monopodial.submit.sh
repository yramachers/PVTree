#!/bin/sh

#Always run this script from the batch directory otherwise pathing goes awry
jobName=YearScan_MonopodialCordate_Spectrum_VMLeaf2
jobNumber=500
simultaneousJobNumber=$jobNumber
OutputFileDirectory=/data/detdev/$USER/PVTree/AnalysisResults/$jobName
OutputLogDirectory=/data/detdev/$USER/PVTree/AnalysisLogs/$jobName

treeType=monopodial
leafType=cordate
treesPerJob=10
maximumTreeTrials=100000
timeSegments=12
photonsPerTimeSegment=100000
geant4SeedOffset=2200
parameterSeedOffset=2200
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
    source ./yearlyTreeScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${treeType} ${leafType} ${treesPerJob} ${maximumTreeTrials} ${timeSegments} ${photonsPerTimeSegment} ${geant4SeedOffset} ${parameterSeedOffset} ${startDate} ${endDate} ${yearSegments} ${minimumSensitiveArea}
