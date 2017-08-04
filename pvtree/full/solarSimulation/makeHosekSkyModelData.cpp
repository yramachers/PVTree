// Compile with
// g++ makeHosekSkyModelData.cpp -std=c++11 -I`root-config --incdir` +
// `root-config --libs` -o makeHosekSkyModelData.exe
//
// Hosek Wilkie Sky Model spectral data converter.
// For license and reference, see included original file
// ArHosekSkyModelData_Spectral.h

// std
#include <vector>

// ROOT
#include <TFile.h>
#include <TTree.h>

// project
#include "ArHosekSkyModelData_Spectral.h"

// Fwd declaration
void convert(double** hsm_data, double** hsm_raddata);

int main() {
  // all data sets are prepared in the header file already
  // void convert(double** datasets, double** raddata);

  // conversion creates the ROOT file for persistent storage
  convert(datasets, datasetsRad);

  return 0;
}

void convert(double** datasets_, double** raddata_) {
  TFile ff("HosekSkyModelData.root", "recreate");
  TTree* tr = new TTree("skymodeldata", "spectral data only");

  int name = 0;
  int wavelength = 0;
  int albedo = 0;
  int turbidity = 0;
  std::vector<double> datavec;
  tr->Branch("name", &name);
  tr->Branch("wl", &wavelength);
  tr->Branch("albedo", &albedo);
  tr->Branch("turbidity", &turbidity);
  tr->Branch("datavector", &datavec);

  int wlchannels[11] = {320, 360, 400, 440, 480, 520, 560, 600, 640, 680, 720};
  int indx = 0;
  for (int j = 0; j < 11; j++) {
    double* dd = datasets_[j];
    for (int al = 0; al < 2; al++) {
      for (int turb = 1; turb < 11; turb++) {
        for (int i = 0; i < 54; i++) {
          indx = i + 54 * (turb - 1) + 10 * 54 * al;
          datavec.push_back(dd[indx]);
        }
        wavelength = wlchannels[j];
        albedo = al;
        turbidity = turb;
        tr->Fill();
        datavec.clear();
      }
    }
  }

  // RadData
  indx = 0;
  name = 1;
  for (int j = 0; j < 11; j++) {
    double* dd = raddata_[j];
    for (int al = 0; al < 2; al++) {
      for (int turb = 1; turb < 11; turb++) {
        for (int i = 0; i < 6; i++) {
          indx = i + 6 * (turb - 1) + 10 * 6 * al;
          datavec.push_back(dd[indx]);
        }
        wavelength = wlchannels[j];
        albedo = al;
        turbidity = turb;
        tr->Fill();
        datavec.clear();
      }
    }
  }
  tr->Write();
  ff.Close();
}
