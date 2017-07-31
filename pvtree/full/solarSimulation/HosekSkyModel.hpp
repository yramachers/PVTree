#ifndef PVTREE_SOLAR_SIMULATION_HWMODEL
#define PVTREE_SOLAR_SIMULATION_HWMODEL

/*! @file
 * \brief Hosek Wilkie Sky Model
 *
 * Uses formulae from SIGGRAPH 2012 and code from
 * http://cgg.mff.cuni.cz/projects/SkylightModelling/
 * translated to C++, YR, Univ. of Warwick (2015)
 */

#include <vector>
#include "TFile.h"
#include "TTree.h"

class HosekSkyModelState {
 private:
  std::vector<std::vector<double> > configs;
  std::vector<double> radiances;

 public:
  std::vector<std::vector<double> > configurations() { return configs; }
  void add_config(std::vector<double> cfg) { configs.push_back(cfg); }
  std::vector<double> rads() { return radiances; }
  void add_rads(double val) { radiances.push_back(val); }
};

class SkyFunction {
 private:
  TFile* ff;
  TTree* tree;
  HosekSkyModelState* state;
  double theta;
  double gamma;
  double wavelength;
  bool ready;

 protected:
  std::vector<double> HosekSkyModel_CookConfiguration(int wlid,
                                                      double turbidity,
                                                      double albedo,
                                                      double solar_elevation);
  double HosekSkyModel_CookRadianceConfiguration(int wlid, double turbidity,
                                                 double albedo,
                                                 double solar_elevation);
  double hosekskymodel_radiance();
  double HosekSkyModel_GetRadianceInternal(std::vector<double> configuration);
  void init(double solar_elevation, double atmospheric_turbidity,
            double ground_albedo);

 public:
  SkyFunction(double solar_elevation, double atmospheric_turbidity,
              double ground_albedo);
  ~SkyFunction();

  /*! \brief Evaluate the 2-dimensional SkyFunction object
   *
   * \returns the probability of light emission at a point
   * on the sky in azimuth, elevation (coordinates) given the input parameters
   *to
   * the SkyFunction constructor. Internal coordinates are theta and gamma as in
   * paper, Figure 5, theta = zero at zenith and
   * gamma, the angle difference to sun position on sky, i.e. gamma  = 0 follows
   *the sun.
   */
  double Eval(double* x, double* par);
};

#endif  // PVTREE_SOLAR_SIMULATION_HWMODEL
