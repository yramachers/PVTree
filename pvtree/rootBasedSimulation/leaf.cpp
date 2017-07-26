#include "pvtree/rootBasedSimulation/leaf.hpp"

Leaf::Leaf(TVector3 position, TVector3 normal, double area, int volumeNumber) : m_position(position), m_normal(normal), m_area(area),  m_volumeName("Leaf"), m_volumeNumber(volumeNumber), m_energy(0.0), m_lastEnergy(0.0) {}

TVector3 Leaf::getPosition() const {
  return m_position;
}

TVector3 Leaf::getNormal() const {
  return m_normal;
}

double Leaf::getArea() const {
  return m_area;
}

std::string Leaf::getID() const {
  return m_volumeName + std::string("_") + std::to_string(m_volumeNumber); //Checking might be less fun.
}

double Leaf::getEnergy() const {
  return m_energy;
}

double Leaf::getLastEnergy() const {
  return m_lastEnergy;
}

void Leaf::setEnergy(double energy) {
  m_lastEnergy = m_energy;
  m_energy = energy;
}
