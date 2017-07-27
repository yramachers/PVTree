#include "pvtree/analysis/yearlyResult.hpp"

#include <list>
#include <algorithm>
#include <iostream>

ClassImp(YearlyResult)

time_t YearlyResult::getReferenceTime() const {

  // Store the time w.r.t. 1/1/1970 (unix timestamp)
  struct tm calendarTime;
  calendarTime.tm_sec=0;
  calendarTime.tm_min=0;
  calendarTime.tm_hour=1;
  calendarTime.tm_mday=1;
  calendarTime.tm_mon=0;
  calendarTime.tm_year=70;
  calendarTime.tm_isdst=1;
  return mktime(&calendarTime);
}

time_t YearlyResult::getReducedGranularityTime(time_t time) const {

  // Convert to calendar time
  struct tm* calendarTime = gmtime(&time);

  // Set some of the elements to default values
  calendarTime->tm_sec=0;
  calendarTime->tm_min=0;
  calendarTime->tm_hour=12;

  // return a new time
  return mktime(calendarTime);
}

YearlyResult::YearlyResult() :  m_tree(0), m_leaf(0) {}

YearlyResult::~YearlyResult() {
  if (m_tree != nullptr) {
    delete m_tree;
    m_tree = nullptr;
  }
  if (m_leaf != nullptr) {
    delete m_leaf;
    m_leaf = nullptr;
  }
}

void YearlyResult::setTree(TreeConstructionInterface* tree) {
  m_tree = tree;
}

void YearlyResult::setLeaf(LeafConstructionInterface* leaf) {
  m_leaf = leaf;
}

void YearlyResult::setEnergyDeposited(std::vector<double> energyDeposited) {
  m_energyDeposited = energyDeposited;
}

void YearlyResult::setDayTimes(std::vector<time_t> dayTimes) {

  m_dayTimes.clear();

  time_t referenceTime = getReferenceTime();

  // Translate day times into something portable
  for (auto& time : dayTimes) {

    time_t dayStartTime = getReducedGranularityTime(time);

    m_dayTimes.push_back( difftime(dayStartTime, referenceTime) );
  }
}

TreeConstructionInterface* YearlyResult::getTree() {
  return m_tree;
}

LeafConstructionInterface* YearlyResult::getLeaf() {
  return m_leaf;
}

std::vector<double> YearlyResult::getEnergyDeposited() const {
  return m_energyDeposited;
}

double YearlyResult::getEnergyDeposited(time_t time,
					ROOT::Math::Interpolation::Type interpolationType/* = ROOT::Math::Interpolation::kCSPLINE */) const {

  // Set all the time parameters below day granularity to default values
  // for consistancy
  time = getReducedGranularityTime(time);

  // Check if time is valid
  if (time < getMinimumTime() || time > getMaximumTime()){
    return 0.0;
  }

  // Find the closest day which has been evaluated
  std::vector<time_t> dayTimes = getDayTimes();
  auto nextTimeIterator = std::lower_bound(begin(dayTimes), end(dayTimes), time);

  int nextTimeIndex = nextTimeIterator - dayTimes.begin();
  int previousTimeIndex = nextTimeIndex;

  // Build interpolation vectors
  std::list<double> xValues; // time
  std::list<double> yValues; // energy value

  // Go forwards by m_interpolationPointNumber times
  // where missing values are skipped!
  int nextFoundValues = 0;
  while (nextFoundValues < m_interpolationPointNumber) {

    // No more values at later times
    if (nextTimeIndex >= (int)dayTimes.size()) {
      break;
    }

    xValues.push_back( dayTimes[nextTimeIndex] );
    yValues.push_back( m_energyDeposited[nextTimeIndex] );
    nextFoundValues++;
    nextTimeIndex++;
  }

  // Go backwards by m_interpolationPointNumber times
  // where missing values are skipped!
  int previousFoundValues = 0;
  while (previousFoundValues < m_interpolationPointNumber) {

    // No more values at earlier times
    if ( previousTimeIndex <= 0) {
      break;
    }

    previousTimeIndex--;

    xValues.push_front( dayTimes[previousTimeIndex] );
    yValues.push_front( m_energyDeposited[previousTimeIndex] );
    previousFoundValues++;
  }

  if (nextFoundValues == 0) {
    // Currently just report a problem
    std::cerr << "WARNING: Interpolation not valid at this time point, using last available data point." << std::endl;

    if (previousFoundValues > 0){
      // Then return the last recorded value
      return yValues.back();
    } else {
      // Actually can't find any value...
      throw std::string( "Found no applicable values.");
    }
  }

  if (previousFoundValues == 0){
    // Currently just report a problem
    std::cerr << "WARNING: Interpolation not valid at this time point, using first available data point." << std::endl;

    if (nextFoundValues > 0){
      // Then return the first recorded value
      return yValues.front();
    } else {
      // Actually can't find any value...
      throw std::string("Found no applicable values.");
    }
  }

  // Evaluate and return interpolation value
  // Copy into vectors
  std::vector<double> xValueVector;
  std::vector<double> yValueVector;

  std::copy(begin(xValues), end(xValues), std::back_inserter(xValueVector));
  std::copy(begin(yValues), end(yValues), std::back_inserter(yValueVector));

  // Build an interpolator
  ROOT::Math::Interpolator interpolator(xValueVector, yValueVector, interpolationType);

  double candidateValue = interpolator.Eval( (double)time );

  // Always ensure positive value
  //! \todo think more carefully about interpolated negative values
  if  (candidateValue < 0.0) {
    candidateValue = 0.0;
  }

  return candidateValue;
}

double YearlyResult::getEnergyIntegral(time_t startTime,
				       time_t endTime,
				       ROOT::Math::Interpolation::Type interpolationType/* = ROOT::Math::Interpolation::kCSPLINE */) const {

  // Set all the time parameters below day granularity to default values
  // for consistancy
  startTime = getReducedGranularityTime(startTime);
  endTime = getReducedGranularityTime(endTime);

  if (startTime > endTime) {
    std::cerr << "WARNING: Requested energy integral where start time is after end time." << std::endl;
    return 0.0;
  }

  // Step through each day and calculate the energy
  double dayStep = 60.0*60.0*24.0;
  time_t currentTime = startTime;
  double energySum = 0.0;

  // When doing the comparison add on a minute to avoid double
  // comparison woes.
  while ( currentTime+60.0 < endTime ) {
    //! \todo Check this stepping actually works.
    energySum += getEnergyDeposited(currentTime, interpolationType);
    currentTime += dayStep;
  }

  return energySum;
}

double YearlyResult::getEnergyIntegral(ROOT::Math::Interpolation::Type interpolationType/* = ROOT::Math::Interpolation::kCSPLINE */) const {

  time_t startTime = getMinimumTime();
  time_t endTime = getMaximumTime();

  // Use the standard function
  return getEnergyIntegral(startTime, endTime, interpolationType);
}

std::vector<time_t> YearlyResult::getDayTimes() const {

  std::vector<time_t> dayTimes;

  // Convert back, but can not of course recover
  // seconds/minutes/hour originally passed in.
  for (auto time : m_dayTimes) {
    dayTimes.push_back( time_t(time) );
  }

  return dayTimes;
}

time_t YearlyResult::getMaximumTime() const {

  if (m_dayTimes.size() == 0){
    return 0;
  }

  time_t maximumTime = time_t(m_dayTimes.back());

  // Check that there is no later time
  for ( auto dayTime : getDayTimes() ) {
    if ( dayTime > maximumTime ) {
      maximumTime = dayTime;
    }
  }

  return maximumTime;
}

time_t YearlyResult::getMinimumTime() const {

  if (m_dayTimes.size() == 0){
    return 0;
  }

  time_t minimumTime = time_t(m_dayTimes.front());

  // Check that there is no earlier time
  for ( auto dayTime : getDayTimes() ) {
    if ( dayTime < minimumTime ) {
      minimumTime = dayTime;
    }
  }

  return minimumTime;
}
