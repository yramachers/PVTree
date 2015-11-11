#include "treeSystem/helical.hpp"
#include "treeSystem/treeConstructionInterface.hpp"
#include <math.h>

//To shorten the following statements
using LSysPtr = std::shared_ptr<TreeSystemInterface>;

//Trunk formation
HelicalBranching::F::F(TreeConstructionInterface* constructor, double elongation) : TreeSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> HelicalBranching::F::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new F(m_constructor, m_elongation)) );

  return results;
} 

void HelicalBranching::F::processTurtles(std::vector<Turtle*>& turtleStack,
					 std::vector<Turtle*>& retiredTurtles){
  Turtle* activeTurtle = turtleStack.back();
  
  //Set the turtle speed
  activeTurtle->length += m_elongation;
  activeTurtle->complete = true;
  
  //Retire the turtle
  retiredTurtles.push_back(activeTurtle);
  turtleStack.pop_back();
  
  //Create a new active turtle to replace it
  turtleStack.push_back(new Turtle( *activeTurtle ));
  
  //Move the active turtle to the new starting position
  turtleStack.back()->move();
  turtleStack.back()->length=0.0;
}

void HelicalBranching::F::print(std::ostream& os) const {
  os << "F(" << m_elongation << ")";
}

//Move but don't create structure
HelicalBranching::f::f(TreeConstructionInterface* constructor, double elongation) : TreeSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> HelicalBranching::f::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new f(m_constructor, m_elongation)) );

  return results;
} 

void HelicalBranching::f::processTurtles(std::vector<Turtle*>& turtleStack,
					 std::vector<Turtle*>& /*retiredTurtles*/){
  Turtle* activeTurtle = turtleStack.back();
  
  activeTurtle->length += m_elongation;
  turtleStack.back()->move();
  turtleStack.back()->length=0.0;
}

void HelicalBranching::f::print(std::ostream& os) const {
  os << "f(" << m_elongation << ")";
}


//Width
HelicalBranching::Exclame::Exclame(TreeConstructionInterface* constructor, double width) : TreeSystemInterface(constructor) {

  if ( width < m_constructor->getDoubleParameter("minimumWidth") ){
    m_width = m_constructor->getDoubleParameter("minimumWidth");
  } else {
    m_width = width;
  }

}

std::vector<LSysPtr> HelicalBranching::Exclame::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Exclame(m_constructor, m_width)) );
  return results;
} 

void HelicalBranching::Exclame::processTurtles(std::vector<Turtle*>& turtleStack,
					       std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.back()->width = m_width;
}

void HelicalBranching::Exclame::print(std::ostream& os) const {
  os << "!(" << m_width << ")";
}

//Length
HelicalBranching::Woosh::Woosh(TreeConstructionInterface* constructor, double length, double elongation) : TreeSystemInterface(constructor) {
  m_length = length;
  m_elongation = elongation;
}

std::vector<LSysPtr> HelicalBranching::Woosh::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Woosh(m_constructor, m_length*m_elongation, m_elongation)) );
  return results;
} 

void HelicalBranching::Woosh::processTurtles(std::vector<Turtle*>& turtleStack,
					     std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.back()->length = m_length;
}

void HelicalBranching::Woosh::print(std::ostream& os) const {
  os << "*(" << m_length << "," << m_elongation  << ")";
}


//Store the current state on the stack
HelicalBranching::LeftBracket::LeftBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> HelicalBranching::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
} 

void HelicalBranching::LeftBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						   std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.push_back(new Turtle( *(turtleStack.back()) ));
}

void HelicalBranching::LeftBracket::print(std::ostream& os) const {
  os << "[";
}


//Pull the last state onto the stack
HelicalBranching::RightBracket::RightBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> HelicalBranching::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
} 

void HelicalBranching::RightBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						    std::vector<Turtle*>& /*retiredTurtles*/){
  delete turtleStack.back();
  turtleStack.pop_back(); 
}

void HelicalBranching::RightBracket::print(std::ostream& os) const {
  os << "]";
}


//Rotate around vector H by an angle in degrees.
HelicalBranching::Slash::Slash(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> HelicalBranching::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
} 

void HelicalBranching::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
					     std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void HelicalBranching::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}


//Rotate around vector L by an angle in degrees.
HelicalBranching::Ampersand::Ampersand(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> HelicalBranching::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void HelicalBranching::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
						 std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
} 

void HelicalBranching::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}

//Rotate around vertical vector in clockwise direction.
HelicalBranching::Plus::Plus(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> HelicalBranching::Plus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Plus(m_constructor, m_angle)) );
  return results;
}

void HelicalBranching::Plus::processTurtles(std::vector<Turtle*>& turtleStack,
					    std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
} 

void HelicalBranching::Plus::print(std::ostream& os) const {
  os << "+(" << m_angle << ")";
}

//Rotate around vertical vector in the anti-clockwise direction
HelicalBranching::Minus::Minus(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> HelicalBranching::Minus::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Minus(m_constructor, m_angle)) );
  return results;
}

void HelicalBranching::Minus::processTurtles(std::vector<Turtle*>& turtleStack,
					     std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = - m_angle * (M_PI/180.0);
  
  //Rotate orientation around vertical vector
  TVector3 verticalVector(0.0, 0.0, 1.0);
  turtleStack.back()->orientation.Rotate(angleToRotate, verticalVector);

  //Also rotate the lVector!
  turtleStack.back()->lVector.Rotate(angleToRotate, verticalVector);
} 

void HelicalBranching::Minus::print(std::ostream& os) const {
  os << "-(" << m_angle << ")";
}


//Controls the growth (Doesn't draw anything!)
HelicalBranching::A::A(TreeConstructionInterface* constructor, double length, double width, double angle, int count) : TreeSystemInterface(constructor) {
  m_length = length;
  m_width  = width;
  m_angle  = angle;
  m_count  = count;
}

std::vector<LSysPtr> HelicalBranching::A::applyRule() {
  std::vector<LSysPtr> results;
  
  //New stuff
  results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("incDecRate"))) );
  results.push_back( LSysPtr(new Exclame(m_constructor, m_width)) );
  results.push_back( LSysPtr(new F(m_constructor, m_length)) );

  if (m_count > m_constructor->getIntegerParameter("branchlessPoints") && 
      (m_count - m_constructor->getIntegerParameter("branchlessPoints")) % m_constructor->getIntegerParameter("stepsBetweenSplit") == 0 ){

    if (m_constructor->getIntegerParameter("simpleBranch") != true){

      //Add a turning end segment
      double nextBranchAngle = fmod((m_angle + m_constructor->getDoubleParameter("branchingAngle")), 360.0);		
      
      results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
      results.push_back( LSysPtr(new Ampersand(m_constructor, m_angle)) );
      results.push_back( LSysPtr(new Plus(m_constructor, m_constructor->getDoubleParameter("turningAngle") )) );
      results.push_back( LSysPtr(new A(m_constructor, m_length*m_constructor->getDoubleParameter("elongationRate"), m_width*m_constructor->getDoubleParameter("contractionRate")*0.8, nextBranchAngle, m_count+1)) );
      results.push_back( LSysPtr(new RightBracket(m_constructor)) );
      results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
      results.push_back( LSysPtr(new Ampersand(m_constructor,-m_angle)) );
      results.push_back( LSysPtr(new Plus(m_constructor, m_constructor->getDoubleParameter("turningAngle") )) );
      results.push_back( LSysPtr(new A(m_constructor, m_length*m_constructor->getDoubleParameter("elongationRate"), m_width*m_constructor->getDoubleParameter("contractionRate")*0.8, nextBranchAngle, m_count+1)) );
      results.push_back( LSysPtr(new RightBracket(m_constructor)) );
      
    } else {

      //Simple branching to produce leaves
      results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
      results.push_back( LSysPtr(new Slash(m_constructor,m_angle)) );
      results.push_back( LSysPtr(new Ampersand(m_constructor,90.0)) );
      results.push_back( LSysPtr(new B(m_constructor, m_length/10.0, m_width/2.0)) );
      results.push_back( LSysPtr(new RightBracket(m_constructor)) );
      
      double nextBranchAngle = fmod((m_angle + m_constructor->getDoubleParameter("branchingAngle")), 360.0);
      results.push_back( LSysPtr(new Plus(m_constructor, m_constructor->getDoubleParameter("turningAngle"))) );
      results.push_back( LSysPtr(new A(m_constructor, m_length*m_constructor->getDoubleParameter("elongationRate"), m_width*m_constructor->getDoubleParameter("contractionRate"), nextBranchAngle, m_count+1)) );

    }

  } else {
    //Add a turning end segment
    double nextBranchAngle = fmod((m_angle + m_constructor->getDoubleParameter("branchingAngle")), 360.0);
    results.push_back( LSysPtr(new Plus(m_constructor, m_constructor->getDoubleParameter("turningAngle") )) );
    results.push_back( LSysPtr(new A(m_constructor, m_length*m_constructor->getDoubleParameter("elongationRate"), m_width*m_constructor->getDoubleParameter("contractionRate"), nextBranchAngle, m_count+1)) );
  }

  return results;
} 

void HelicalBranching::A::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					 std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void HelicalBranching::A::print(std::ostream& os) const {
  os << "A(" << m_length << "," << m_width << "," << m_angle << "," << m_count  <<  ")";
}

//Controls the growth of the leaf points
HelicalBranching::B::B(TreeConstructionInterface* constructor, double length, double width) : TreeSystemInterface(constructor) {
  m_length = length;
  m_width  = width;
}

std::vector<LSysPtr> HelicalBranching::B::applyRule() {
  std::vector<LSysPtr> results;
  
  //Make a branch that slowly extends
  results.push_back( LSysPtr(new Exclame(m_constructor, m_width)) );
  results.push_back( LSysPtr(new Woosh(m_constructor, m_length, m_constructor->getDoubleParameter("branchElongation") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_length)) );

  return results;
} 

void HelicalBranching::B::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					 std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void HelicalBranching::B::print(std::ostream& os) const {
  os << "B(" << m_length << "," << m_width << ")";
}



