#ifndef PV_POLYGON
#define PV_POLYGON

#include <string>
#include <vector>
#include "TVector3.h"
#include "geometry/vertex.hpp"
#include <memory>

class Vertex;

class Polygon {

private:
  std::vector<std::shared_ptr<Vertex>> m_vertices;

public:
  Polygon();
  Polygon(const Polygon& original);

  unsigned int              size();
  void                      addVertex(std::shared_ptr<Vertex> vertex);
  void                      addVertex(TVector3 vertexPosition);
  std::shared_ptr<Vertex>   getVertex(unsigned int index);
  void                      replaceVertex(std::shared_ptr<Vertex> originalVertex, 
					  std::shared_ptr<Vertex> replacementVertex);
  TVector3                  getNormal();
  void                      invertNormal();
  
  /*! \brief Calculate the area of the polygon.
   *
   * Only handles triangles at the moment...
   *
   * \returns The area of the triangle.
   */
  double                    getArea();

};

#endif //PV_POLYGON
