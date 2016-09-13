#!/bin/sh

#Always run this script from the batch directory otherwise pathing goes awry
jobName=YearScan_StumpPlanar_Spectrum_VSky2
jobNumber=100
simultaneousJobNumber=$jobNumber
OutputFileDirectory=/data/detdev/$USER/PVTree/AnalysisResults/$jobName
OutputLogDirectory=/data/detdev/$USER/PVTree/AnalysisLogs/$jobName

treeType=stump
leafType=planar
treesPerJob=100
maximumTreeTrials=100000
timeSegments=12
photonsPerTimeSegment=100000
geant4SeedOffset=1100
parameterSeedOffset=1100
startDate=1/1/2014
endDate=1/1/2015
yearSegments=150
minimumSensitiveArea=0.5

#Make sure there is an output directory
mkdir -p $OutputFileDirectory
mkdir -p $OutputLogDirectory

#Submit an appropriately sized job array
bsub -o ${OutputLogDirectory}/%J_%I.out \
    -q "xlong" \
    -J "$jobName[1-${jobNumber}]%${simultaneousJobNumber}" \
    source ./yearlyTreeScan.sh ${PWD} ${jobName} ${OutputFileDirectory} ${treeType} ${leafType} ${treesPerJob} ${maximumTreeTrials} ${timeSegments} ${photonsPerTimeSegment} ${geant4SeedOffset} ${parameterSeedOffset} ${startDate} ${endDate} ${yearSegments} ${minimumSensitiveArea}
