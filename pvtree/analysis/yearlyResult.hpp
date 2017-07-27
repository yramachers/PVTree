#ifndef PVTREE_ANALYSIS_YEARLY_RESULT_HPP
#define PVTREE_ANALYSIS_YEARLY_RESULT_HPP

#include <vector>
#include "TObject.h"
#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"

#include <Math/Interpolator.h>

/*! \brief Class to hold analysis results for yearly job.
 *
 */
class YearlyResult : public TObject {
private:

  // Store the unix time stamp for each day (the start of)
  std::vector<double> m_dayTimes;

  // Store each simulated day's energy
  std::vector<double> m_energyDeposited;

  // Store the tree and leaf used.
  TreeConstructionInterface* m_tree;
  LeafConstructionInterface* m_leaf;

  // Store the number of interpolation points to use
  int m_interpolationPointNumber = 5;

  /* \brief Common reference time point definition
   *
   *  1/1/1970 (unix timestamp)
   *
   * \returns The reference time which everything is
   *          relative to.
   */
  time_t getReferenceTime() const;

  /* \brief Reduce time granularity
   *
   * Don't want to have any second/minute/hour
   * differences when interpolating.
   *
   * @param[in] time The input time to be cleaned up.
   * \returns The time with reduced granularity.
   */
  time_t getReducedGranularityTime(time_t time) const;

public:
  YearlyResult();
  ~YearlyResult();

  /* \brief Set the tree being simulated
   *
   * Do not delete the tree object after passing it to the
   * yearly result. It is handled in the destructor now.
   *
   * @param[in] tree The tree construction interface pointer.
   */
  void setTree(TreeConstructionInterface* tree);

  /* \brief Set the leaf being simulated
   *
   * Do not delete the leaf object after passing it to the
   * yearly result. It is handled in the destructor now.
   *
   * @param[in] leaf The leaf construction interface pointer.
   */
  void setLeaf(LeafConstructionInterface* leaf);

  /* \brief Set the energy deposited for each of the days.
   *
   * @param[in] energyDeposited A vector of energies deposited
   *            in kWh on each day.
   */
  void setEnergyDeposited(std::vector<double> energyDeposited);

  /* \brief Set the list of times for the middle of
   *        each day where the energy has been simulated.
   *
   * This function will automatically clean times to remove
   * second/minute/hour differences. (Always middle of day).
   *
   * @param[in] dayTimes Is a vector of time_t objects.
   */
  void setDayTimes(std::vector<time_t> dayTimes);

  /* \brief Retreive the tree simulated.
   *
   * \returns The tree construction interface.
   */
  TreeConstructionInterface* getTree();

  /* \brief Retreive the tree simulated.
   *
   * \returns The tree construction interface.
   */
  LeafConstructionInterface* getLeaf();

  /* \brief Retrieve all the simulated energy deposits
   *
   * \returns The vector of energy deposits in kWh
   */
  std::vector<double> getEnergyDeposited() const;

  /* \brief Retrieve the energy deposited on a specific
   *        day. May use interpolation if no value is
   *        found.
   *
   * @param[in] time Time to retrieve energy deposit, which
   *                  will resolve to just a day.
   * @param[in] interpolationType The method of interpolation that should be used,
   *            where by default the cubic spline method is used.
   *
   * \returns The energy deposited in kWh on this day only.
   */
  double getEnergyDeposited(time_t time,
			    ROOT::Math::Interpolation::Type interpolationType = ROOT::Math::Interpolation::kCSPLINE) const;

  /* \brief Get the energy integral between two days
   *
   * @param[in] startTime First day of the integral
   * @param[in] endTime   The day to stop integration.
   * @param[in] interpolationType The method of interpolation that should be used,
   *            where by default the cubic spline method is used.
   *
   * \returns The energy deposited in kWh between two days.
   */
  double getEnergyIntegral(time_t startTime,
			   time_t endTime,
			   ROOT::Math::Interpolation::Type interpolationType = ROOT::Math::Interpolation::kCSPLINE) const;

  /* \brief Get the energy integral for complete time range
   *
   * @param[in] interpolationType The method of interpolation that should be used,
   *            where by default the cubic spline method is used.
   *
   * \returns The energy deposited in kWh in complete time range
   */
  double getEnergyIntegral(ROOT::Math::Interpolation::Type interpolationType = ROOT::Math::Interpolation::kCSPLINE) const;

  /* \brief Get the list of day middle times where simulation
   *        has taken place.
   *
   * \returns The vector of times for each of the energy deposits.
   */
  std::vector<time_t> getDayTimes() const;

  /* \brief Retrieve the maximum time of simulation
   *
   * \returns The maximum time at which simulation has taken place.
   */
  time_t getMaximumTime() const;

  /* \brief Retrieve the minimum time of simulation
   *
   * \returns The minimum time at which simulation has taken place.
   */
  time_t getMinimumTime() const;


  ClassDef(YearlyResult, 2);
};


#endif //PVTREE_ANALYSIS_YEARLY_RESULT_HPP




