#include "geometry/polygon.hpp"
#include "geometry/vertex.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>

Polygon::Polygon() {}

Polygon::Polygon(const Polygon& original) {

  //`Clone' the vertices
  for ( auto& vertex : original.m_vertices ){
    this->addVertex(std::shared_ptr<Vertex>( new Vertex( vertex->getPosition() ) ));
  }
}

unsigned int Polygon::size() {
  return m_vertices.size();
}

void Polygon::addVertex(std::shared_ptr<Vertex> vertex) {
  m_vertices.push_back(vertex);

  //Also register this polygon with the vertex
  vertex->registerFace(this);
}

void Polygon::addVertex(TVector3 vertexPosition) {
  std::shared_ptr<Vertex> vertex(new Vertex(vertexPosition));

  m_vertices.push_back(vertex);

  //Also register this polygon with the vertex
  vertex->registerFace(this);
}

std::shared_ptr<Vertex> Polygon::getVertex(unsigned int index) {
  if (index >= this->size()){
    throw std::out_of_range("Not enough vertices pushed into the polygon.");
  } 

  return m_vertices[index];
}

void Polygon::replaceVertex(std::shared_ptr<Vertex> originalVertex, 
			    std::shared_ptr<Vertex> replacementVertex){

  //ensure the vertices know what is up
  originalVertex->deregisterFace(this);
  replacementVertex->registerFace(this);

  std::replace(m_vertices.begin(), m_vertices.end(), originalVertex, replacementVertex);
}

TVector3 Polygon::getNormal() {

  if (this->size() != 3) {
    //Not the correct number of vertices --> bad things will happen
    throw;
  }

  TVector3 ab = m_vertices[0]->getPosition() - m_vertices[1]->getPosition();
  TVector3 cb = m_vertices[2]->getPosition() - m_vertices[1]->getPosition();

  TVector3 normal = cb.Cross(ab);
  
  return normal.Unit();
}

void Polygon::invertNormal() {
  
  //Switch the order of the last two vertices
  //so that the normal will be evaluated differently.
  if (this->size() != 3) {
    //Not the correct number of vertices --> bad things will happen
    throw;
  }

  std::shared_ptr<Vertex> tempVertex = m_vertices[1];
  m_vertices[1] = m_vertices[2];
  m_vertices[2] = tempVertex;
}

double Polygon::getArea() {

  if (this->size() != 3){
    std::cerr << "Polygon does not handle the non triangle case for area evaluation" << std::endl;
    throw;
  }

  TVector3 ab = m_vertices[0]->getPosition() - m_vertices[1]->getPosition();
  TVector3 cb = m_vertices[2]->getPosition() - m_vertices[1]->getPosition();

  double area = 0.5 * (cb.Cross(ab)).Mag();

  return area;
}
