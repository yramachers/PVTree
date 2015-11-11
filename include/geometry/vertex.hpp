#ifndef PV_VERTEX
#define PV_VERTEX

#include <string>
#include <vector>
#include "TVector3.h"
#include <memory>

class Polygon;

class Vertex {

private:
  TVector3              m_position;
  std::vector<Polygon*> m_containingFaces;

public:
  Vertex();
  explicit Vertex(TVector3 position);

  void     setPosition(TVector3 position);
  TVector3 getPosition() const;
  TVector3 getNormal();
  TVector3 getEdgeNormal(std::shared_ptr<Vertex> otherVertex);
  void     registerFace(Polygon* face);
  void     deregisterFace(Polygon* face);
};

#endif //PV_Vertex
