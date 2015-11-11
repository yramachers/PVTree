import ROOT
import datetime

def convertToRootTime(year, day):
    currentTime = datetime.date(year,1,1) + datetime.timedelta(day)

    # ROOT defines time as starting from 01/01/1995
    rootStartingTime = datetime.date(1995,1,1)
    
    deltaTime = currentTime - rootStartingTime
    totalSeconds = deltaTime.seconds + deltaTime.days * 24 * 3600

    return totalSeconds

def scaleGraph(inputGraph, factor):
    
    for p in range(0, inputGraph.GetN()):
        currentX = inputGraph.GetX()[p]
        scaledY = inputGraph.GetY()[p]*factor
        scaledYHighError = inputGraph.GetErrorYhigh(p)*factor
        scaledYLowError =  inputGraph.GetErrorYlow(p)*factor
        
        inputGraph.SetPointEYhigh(p, scaledYHighError)
        inputGraph.SetPointEYlow(p,  scaledYHighError)
        inputGraph.SetPoint(p, currentX, scaledY)

# Load in the four graphs
example1File = ROOT.TFile("5yearly.proposalExampleA.clearSky.root")
imaginationFile = ROOT.TFile("5yearly.printed.clearSky.root")
planarFile = ROOT.TFile("5yearly.optimalPlanar.clearSky.root")

example1Graph = example1File.FindObjectAny("averageTreeEnergyDensityGraph")
imaginationGraph = imaginationFile.FindObjectAny("averageTreeEnergyDensityGraph")
planarGraph = planarFile.FindObjectAny("averageTreeEnergyDensityGraph")

# Scale Y-axis values to take into account a rough efficiency
quantumEfficiency=0.2
scaleGraph(example1Graph, quantumEfficiency)
scaleGraph(imaginationGraph, quantumEfficiency)
scaleGraph(planarGraph, quantumEfficiency)

# Plot them on the same canvas
canvas = ROOT.TCanvas("averageEnergyDensityCanvas", "", 1247, 666)
canvas.SetLeftMargin(0.0993)
canvas.SetRightMargin(0.0356)
canvas.SetBottomMargin(0.105)
canvas.SetTopMargin(0.0387)

example1Graph.Draw("ALE3")
imaginationGraph.Draw("SAMELE3")
planarGraph.Draw("SAMELE3")

# Setup graph properties
example1Graph.GetXaxis().SetTitle("Date")
example1Graph.GetYaxis().SetTitle("Energy Density [kWhm^{-2}day^{-1}]")
example1Graph.GetXaxis().SetTitleSize(0.05)
example1Graph.GetXaxis().SetLabelSize(0.05)
example1Graph.GetYaxis().SetTitleSize(0.05)
example1Graph.GetYaxis().SetLabelSize(0.05)
example1Graph.GetYaxis().SetTitleOffset(0.84)
example1Graph.GetXaxis().SetTitleFont(62)
example1Graph.GetYaxis().SetTitleFont(62)
example1Graph.SetFillColor(ROOT.kRed-6)
example1Graph.SetLineColor(ROOT.kRed-6)
example1Graph.GetXaxis().SetTimeDisplay(1)
example1Graph.GetXaxis().SetTimeFormat("%d/%m/%Y")

imaginationGraph.SetFillColor(ROOT.kGreen-6)
imaginationGraph.SetLineColor(ROOT.kGreen-6)

planarGraph.SetFillColor(ROOT.kBlue-6)
planarGraph.SetLineColor(ROOT.kBlue-6)

# Set the range of each axis
#example1Graph.GetXaxis().SetRange(13, 128)
#example1Graph.GetYaxis().SetRangeUser(0.0, 1.6)

# Add a legend
legend = ROOT.TLegend(0.751, 0.773, 0.952, 0.940)
legend.AddEntry(example1Graph, "Proposal", "F")
legend.AddEntry(imaginationGraph, "Imagination", "F")
legend.AddEntry(planarGraph, "Planar", "F")
legend.SetLineColor(ROOT.kWhite)
legend.SetTextFont(62)
legend.Draw()

# Add text label describing latitude and longitude of simulation point
additionalText = ROOT.TPaveText(0.117, 0.772, 0.582, 0.940, "NDC")
additionalText.SetBorderSize(0)
additionalText.SetTextAlign(12)
additionalText.SetFillStyle(0)
additionalText.AddText("Simulation")
additionalText.AddText("Latitude = 52.4 ; Longitude = -1.56")
additionalText.AddText("Efficiency_{PV module} = 0.2")
additionalText.Draw()

# Save canvas to a ROOT file
outputFile = ROOT.TFile("composed.root", "RECREATE")

canvas.Update()
canvas.Write()

outputFile.Close()
