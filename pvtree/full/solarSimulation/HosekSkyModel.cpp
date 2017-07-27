#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include "TCut.h"

#include "pvtree/full/solarSimulation/HosekSkyModel.hpp"

SkyFunction::SkyFunction(	      
			 double  solar_elevation,
			 double  atmospheric_turbidity,
			 double  ground_albedo
				      ) 
{
  state = new HosekSkyModelState();
  theta = 0.0;
  gamma = 0.0;
  wavelength = 0.0;
  ready = 0; // False

  std::ifstream localTest("HosekSkyModelData.root");

  if ( localTest.is_open() ) {
    // If found use the local file
    localTest.close();
    ff = new TFile("HosekSkyModelData.root","read");
    tree = (TTree*)ff->Get("skymodeldata");
    init(solar_elevation, atmospheric_turbidity, ground_albedo);
  }
  else {
    const char* environmentVariableContents = std::getenv("PVTREE_SHARE_PATH");
    if ( environmentVariableContents != 0 ) {
      
      //Environment variable set so give it a try
      std::string   shareFilePath(std::string(environmentVariableContents) + "/" + "HosekSkyModelData.root");
      std::ifstream shareTest(shareFilePath.c_str());
      
      if ( shareTest.is_open() ) {
	shareTest.close();
	ff = new TFile(shareFilePath.c_str(),"read");
	tree = (TTree*)ff->Get("skymodeldata");
	init(solar_elevation, atmospheric_turbidity, ground_albedo);
      }
      else {
	// If reaching here then unable to extract a file's contents!
	std::cout << "Spectrum::Spectrum - Unable to find the Sky model file " << std::endl;
	throw std::invalid_argument("Can't find HosekSkyModelData.root input file.");
      }
    }
  }

  //  std::cout << "in constructor configs, size=" << state->configurations().size() << std::endl;
}


SkyFunction::~SkyFunction() {
  ff->Close();
  if (state) delete state;
}


double SkyFunction::Eval(double *x, double* par) { 
//   std::cout << "called Eval with " << x[0] << " " << x[1] << std::endl;
//   std::cout << "in Eval configs, size=" << state->configurations().size() << std::endl;
  if (ready) {
    theta = x[0];
    gamma = x[1];
    int wlchannels[11] = {320,360,400,440,480,520,560,600,640,680,720};
    double sum = 0.0;
    for( int wl = 0; wl < 11; ++wl ) {
      wavelength = wlchannels[wl];
      sum += hosekskymodel_radiance();
    }
    return sum; // total radiance over all wavelengths
  }
  else {
    std::cout << "SkyFunction: launch init first before use. Bailing out with zero return." << std::endl;
    return 0.0;
  }
} 


std::vector<double> SkyFunction::HosekSkyModel_CookConfiguration(
						    int                         wlid, 
						    double                      turbidity, 
						    double                      albedo, 
						    double                      solar_elevation
						    )
{
  double pi = acos(-1.0);
  std::vector<double> config;
  int wlchannels[11] = {320,360,400,440,480,520,560,600,640,680,720};
  int     int_turbidity = (int)turbidity;
  double  turbidity_rem = turbidity - (double)int_turbidity;
  
  TString name_cut = "name==0"; // Data
  TCut c1 = name_cut.Data();
  TString albedo_cut = "albedo==0";
  TCut c2 = albedo_cut.Data();

  TString turbidity_cut = "turbidity==";
  turbidity_cut += int_turbidity;
  TCut c3 = turbidity_cut.Data();

  TString wl_cut = "wl==";
  wl_cut += wlchannels[wlid];
  TCut c4 = wl_cut.Data();

  TCut cut = c1 && c2 && c3 && c4;
  //  cout << cut.GetTitle() << endl;

  solar_elevation = pow(solar_elevation / (pi / 2.0), (1.0 / 3.0));
  
  // alb 0 low turb
  std::vector<double> *data = 0;
  tree->SetBranchAddress("datavector",&data);
  
  //  elev_matrix = dataset + ( 9 * 6 * (int_turbidity-1) );
  TEntryList* elist = 0;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");
  
  for( unsigned int i = 0; i < 9; ++i )
    {
      //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
      config.push_back( 
        (1.0-albedo) * (1.0 - turbidity_rem) 
        * ( pow(1.0-solar_elevation, 5.0) * data->at(i)  + 
	    5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * data->at(i+9) +
	    10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(i+18) +
	    10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(i+27) +
	    5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(i+36) +
	    pow(solar_elevation, 5.0)  * data->at(i+45))
			);
    }
  
  // alb 1 low turb
  albedo_cut = "albedo==1";
  c2 = albedo_cut.Data();

  cut = c1 + c2 + c3 + c4;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");

  //  elev_matrix = dataset + (9*6*10 + 9*6*(int_turbidity-1));
  for(unsigned int i = 0; i < 9; ++i)
    {
      //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
      config[i] += 
        (albedo) * (1.0 - turbidity_rem)
        * ( pow(1.0-solar_elevation, 5.0) * data->at(i)  + 
	    5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * data->at(i+9) +
	    10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(i+18) +
	    10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(i+27) +
	    5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(i+36) +
	    pow(solar_elevation, 5.0)  * data->at(i+45));
    }
  
  // alb 0 high turb
  albedo_cut = "albedo==0";
  c2 = albedo_cut.Data();

  turbidity_cut = "turbidity==";
  turbidity_cut += int_turbidity+1;
  c3 = turbidity_cut.Data();

  cut = c1 + c2 + c3 + c4;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");

  //  elev_matrix = dataset + (9*6*(int_turbidity));
  for(unsigned int i = 0; i < 9; ++i)
    {
      //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
      config[i] += 
        (1.0-albedo) * (turbidity_rem)
        * ( pow(1.0-solar_elevation, 5.0) * data->at(i)  + 
	    5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * data->at(i+9) +
	    10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(i+18) +
	    10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(i+27) +
	    5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(i+36) +
	    pow(solar_elevation, 5.0)  * data->at(i+45));
    }
  
  // alb 1 high turb
  albedo_cut = "albedo==1";
  c2 = albedo_cut.Data();

  cut = c1 + c2 + c3 + c4;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");

  //  elev_matrix = dataset + (9*6*10 + 9*6*(int_turbidity));
  for(unsigned int i = 0; i < 9; ++i)
    {
      //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
      config[i] += 
        (albedo) * (turbidity_rem)
        * ( pow(1.0-solar_elevation, 5.0) * data->at(i)  + 
	    5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * data->at(i+9) +
	    10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(i+18) +
	    10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(i+27) +
	    5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(i+36) +
	    pow(solar_elevation, 5.0)  * data->at(i+45));
    }
  return config;
}


double SkyFunction::HosekSkyModel_CookRadianceConfiguration(
					       int                  wlid, 
					       double               turbidity, 
					       double               albedo, 
					       double               solar_elevation
					       )
{
  double pi = acos(-1.0);
  int wlchannels[11] = {320,360,400,440,480,520,560,600,640,680,720};
  
  int int_turbidity = (int)turbidity;
  double turbidity_rem = turbidity - (double)int_turbidity;
  double res;

  TString name_cut = "name==1"; // RadData
  TCut c1 = name_cut.Data();
  TString albedo_cut = "albedo==0";
  TCut c2 = albedo_cut.Data();

  TString turbidity_cut = "turbidity==";
  turbidity_cut += int_turbidity;
  TCut c3 = turbidity_cut.Data();

  TString wl_cut = "wl==";
  wl_cut += wlchannels[wlid];
  TCut c4 = wl_cut.Data();

  TCut cut = c1 + c2 + c3 + c4;
  //  cout << "in rad: " << cut.GetTitle() << endl;

  solar_elevation = pow(solar_elevation / (pi / 2.0), (1.0 / 3.0));
  
  // alb 0 low turb
  std::vector<double> *data = 0;
  tree->SetBranchAddress("datavector",&data);

  TEntryList* elist = 0;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");

  //  elev_matrix = dataset + (6*(int_turbidity-1));
  //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res = (1.0-albedo) * (1.0 - turbidity_rem) *
    ( pow(1.0-solar_elevation, 5.0) * data->at(0) +
      5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * data->at(1) +
      10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(2) +
      10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(3) +
      5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(4) +
      pow(solar_elevation, 5.0) * data->at(5));
  
  // alb 1 low turb
  albedo_cut = "albedo==1";
  c2 = albedo_cut.Data();

  cut = c1 + c2 + c3 + c4;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");

  //  elev_matrix = dataset + (6*10 + 6*(int_turbidity-1));
  //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res += (albedo) * (1.0 - turbidity_rem) *
    ( pow(1.0-solar_elevation, 5.0) * data->at(0) +
      5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * data->at(1) +
      10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(2) +
      10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(3) +
      5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(4) +
      pow(solar_elevation, 5.0) * data->at(5));

  if(int_turbidity >= 10)
    return res;
  
  // alb 0 high turb
  albedo_cut = "albedo==0";
  c2 = albedo_cut.Data();

  turbidity_cut = "turbidity==";
  turbidity_cut += int_turbidity+1;
  c3 = turbidity_cut.Data();

  cut = c1 + c2 + c3 + c4;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");

  //  elev_matrix = dataset + (6*(int_turbidity));
  //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res += (1.0-albedo) * (turbidity_rem) *
    ( pow(1.0-solar_elevation, 5.0) * data->at(0) +
      5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * data->at(1) +
      10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(2) +
      10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(3) +
      5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(4) +
      pow(solar_elevation, 5.0) * data->at(5));
  
  // alb 1 high turb
  albedo_cut = "albedo==1";
  c2 = albedo_cut.Data();

  cut = c1 + c2 + c3 + c4;
  tree->SetEntryList(0);
  tree->Draw(">>elist",cut,"entrylist");
  elist = (TEntryList*)gDirectory->Get("elist");
  tree->SetEntryList(elist);
  tree->Draw("datavector","","goff");

  //  elev_matrix = dataset + (6*10 + 6*(int_turbidity));
  //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res += (albedo) * (turbidity_rem) *
    ( pow(1.0-solar_elevation, 5.0) * data->at(0) +
      5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * data->at(1) +
      10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * data->at(2) +
      10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * data->at(3) +
      5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * data->at(4) +
      pow(solar_elevation, 5.0) * data->at(5));
  return res;
}


void SkyFunction::init(
		       double  solar_elevation,
		       double  atmospheric_turbidity,
		       double  ground_albedo
		       )
{
  std::vector<double> config;
  
  for( int wl = 0; wl < 11; ++wl )
    {
      config = HosekSkyModel_CookConfiguration(
					       wl,
					       atmospheric_turbidity,
					       ground_albedo,
					       solar_elevation
					       );
      
      state->add_config(config);
      config.clear();

      state->add_rads(HosekSkyModel_CookRadianceConfiguration(
							      wl,
							      atmospheric_turbidity,
							      ground_albedo,
							      solar_elevation
							      ));
      
    }

  //  std::cout << "read configs, size=" << state->configurations().size() << std::endl;
  ready = 1; // precaution
}



double SkyFunction::hosekskymodel_radiance()
{
  int low_wl = (wavelength - 320.0 ) / 40.0;
  //  std::cout << "in radiance function, wavel = " << wavelength << " at index = " << low_wl << std::endl;
  
  if ( low_wl < 0 || low_wl >= 11 )
    return 0.0f;
  
  double interp = fmod((wavelength - 320.0 ) / 40.0, 1.0);
  std::vector<double> cfg = state->configurations().at(low_wl);  

  double val_low = 
    HosekSkyModel_GetRadianceInternal(cfg)
    * state->rads()[low_wl];
  
  if ( interp < 1e-6 )
    return val_low;
  
  double result = ( 1.0 - interp ) * val_low;
  
  if ( low_wl+1 < 11 )
    {
      result +=
	interp
	* HosekSkyModel_GetRadianceInternal(state->configurations()[low_wl+1])
	* state->rads()[low_wl+1];
    }
  
  return result;
}


double SkyFunction::HosekSkyModel_GetRadianceInternal(std::vector<double> configuration)
{
  //  std::cout << "in radiance internal config size = " << configuration.size() << std::endl;
  const double expM = exp(configuration[4] * gamma);
  const double rayM = cos(gamma)*cos(gamma);
  const double mieM = (1.0 + cos(gamma)*cos(gamma)) / pow((1.0 + configuration[8]*configuration[8] - 2.0*configuration[8]*cos(gamma)), 1.5);
  const double zenith = sqrt(cos(theta));
  
  return (1.0 + configuration[0] * exp(configuration[1] / (cos(theta) + 0.01))) *
    (configuration[2] + configuration[3] * expM + configuration[5] * rayM + configuration[6] * mieM + configuration[7] * zenith);
}

