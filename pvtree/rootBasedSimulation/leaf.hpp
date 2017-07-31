#ifndef PV_LEAF
#define PV_LEAF

#include <string>
#include "TVector3.h"

// Just a quick storage of leaf simulation properties.
class Leaf {
 private:
  TVector3 m_position;
  TVector3 m_normal;
  double m_area;
  std::string m_volumeName;  // Leaf
  int m_volumeNumber;
  double m_energy;
  double m_lastEnergy;

 public:
  Leaf(TVector3 position, TVector3 normal, double area, int volumeNumber);

  TVector3 getPosition() const;
  TVector3 getNormal() const;
  double getArea() const;
  std::string getID() const;
  double getEnergy() const;
  double getLastEnergy() const;
  void setEnergy(double energy);
};

#endif  // PV_LEAF
