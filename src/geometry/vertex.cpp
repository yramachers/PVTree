#include "geometry/vertex.hpp"
#include "geometry/polygon.hpp"
#include <stdexcept>
#include <algorithm>

Vertex::Vertex() {}
Vertex::Vertex(TVector3 position) : m_position(position){}

void Vertex::setPosition(TVector3 position) {
  m_position = position;
}

TVector3 Vertex::getPosition() const {
  return m_position;
}

TVector3 Vertex::getNormal() {
  //Always re-calculate (don't know if faces have changed)
  if (m_containingFaces.size() == 0){
    return TVector3(0.0, 0.0, 0.0);
  }

  //Get the normal of each face containing the vertex then take the average
  TVector3 faceNormalAverage(0.0, 0.0, 0.0);

  for (unsigned int f=0; f<m_containingFaces.size(); f++){ 
    TVector3 faceNormal = m_containingFaces[f]->getNormal();

    faceNormalAverage += faceNormal;
  }

  //Always return a unit vector (also don't need to then divide by the number of faces!)
  return faceNormalAverage.Unit();
}

TVector3 Vertex::getEdgeNormal(std::shared_ptr<Vertex> otherVertex) {

  //Find the common face for this vertex and the other vertex
  bool foundCommonFace = false;
  Polygon* commonFace = 0;
  for (Polygon* testFace : m_containingFaces) {
    
    auto otherVertexFace = std::find(otherVertex->m_containingFaces.begin(), otherVertex->m_containingFaces.end(), testFace);

    if (otherVertexFace != otherVertex->m_containingFaces.end()) {
      //Found a shared face, so use it!
      foundCommonFace = true;
      commonFace = testFace;
      break;
    }
  }

  //! \todo I need to handle the case where there is no common face better.
  if (!foundCommonFace) {
    return TVector3(0.0,0.0,0.0);
  }

  //Need the normal of the common face
  TVector3 commonFaceNormal = commonFace->getNormal();

  //and the direction between the edge vertices
  TVector3 edgeVector = (otherVertex->m_position - m_position).Unit();

  //Use cross product to get the edge face normal vector (or at least something vaguelly similar)
  TVector3 candidateEdgeNormal = commonFaceNormal.Cross(edgeVector).Unit();

  //Check candidate direction is pointing in the right direction (i.e. not towards the centre of the face!)
  //Need to use the edge vector to the other unused vertex in the face
  for (unsigned int v=0; v<commonFace->size(); v++) {
    
    std::shared_ptr<Vertex> vertex = commonFace->getVertex(v);

    if (vertex != otherVertex && vertex.get() != this) {
      // Can now check candidate edge normal direction is correct
      TVector3 differentEdgeVector = (vertex->m_position - m_position).Unit();

      //Check that angle between these vectors is greater than 90.0 deg
      if (differentEdgeVector.Dot(candidateEdgeNormal) >= 0.0) {
	//Need to invert candidate normal direction
	candidateEdgeNormal = - candidateEdgeNormal;
	break;
      }
    }
  }

  return candidateEdgeNormal;
}

void Vertex::registerFace(Polygon* face) {

  //Check if face is already present
  auto facePointer = std::find(m_containingFaces.begin(), m_containingFaces.end(), face);

  if (facePointer == m_containingFaces.end()){
    //Not present, so add
    m_containingFaces.push_back(face);
  }

}

void Vertex::deregisterFace(Polygon* face) {

  //Find the face to be removed
  auto facePointer = std::find(m_containingFaces.begin(), m_containingFaces.end(), face);

  if (facePointer != m_containingFaces.end()){
    //Present, so remove!
    m_containingFaces.erase(facePointer);
  }

}




