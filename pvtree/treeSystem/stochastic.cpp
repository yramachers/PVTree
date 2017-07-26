#include "pvtree/treeSystem/stochastic.hpp"
#include "pvtree/treeSystem/treeConstructionInterface.hpp"

//To shorten the following statements
using LSysPtr = std::shared_ptr<TreeSystemInterface>;

//Trunk formation
StochasticBranching::F::F(TreeConstructionInterface* constructor, double elongation) : TreeSystemInterface(constructor) {
  m_elongation = elongation;
}

std::vector<LSysPtr> StochasticBranching::F::applyRule() {

  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new F(m_constructor, m_constructor->getDoubleParameter("elongationRate")*m_elongation)) );

  return results;
} 

void StochasticBranching::F::processTurtles(std::vector<Turtle*>& turtleStack,
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

void StochasticBranching::F::print(std::ostream& os) const {
  os << "F(" << m_elongation << ")";
}


//Width
StochasticBranching::Exclame::Exclame(TreeConstructionInterface* constructor, double width, double increaseRate) : TreeSystemInterface(constructor) {
  m_width = width;
  m_increaseRate = increaseRate;
}

std::vector<LSysPtr> StochasticBranching::Exclame::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Exclame(m_constructor, m_increaseRate*m_width, m_increaseRate)) );
  return results;
} 

void StochasticBranching::Exclame::processTurtles(std::vector<Turtle*>& turtleStack,
						  std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.back()->width = m_width;
}

void StochasticBranching::Exclame::print(std::ostream& os) const {
  os << "!(" << m_width << "," << m_increaseRate << ")";
}


//Store the current state on the stack
StochasticBranching::LeftBracket::LeftBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> StochasticBranching::LeftBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new LeftBracket(m_constructor)));
  return results;
} 

void StochasticBranching::LeftBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						      std::vector<Turtle*>& /*retiredTurtles*/){
  turtleStack.push_back(new Turtle( *(turtleStack.back()) ));
}

void StochasticBranching::LeftBracket::print(std::ostream& os) const {
  os << "[";
}


//Pull the last state onto the stack
StochasticBranching::RightBracket::RightBracket(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) { }

std::vector<LSysPtr> StochasticBranching::RightBracket::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr(new RightBracket(m_constructor)));
  return results;
} 

void StochasticBranching::RightBracket::processTurtles(std::vector<Turtle*>& turtleStack,
						       std::vector<Turtle*>& /*retiredTurtles*/){
  delete turtleStack.back();
  turtleStack.pop_back(); 
}

void StochasticBranching::RightBracket::print(std::ostream& os) const {
  os << "]";
}


//Rotate around vector H by an angle in degrees.
StochasticBranching::Slash::Slash(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> StochasticBranching::Slash::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Slash(m_constructor, m_angle)) );
  return results;
} 

void StochasticBranching::Slash::processTurtles(std::vector<Turtle*>& turtleStack,
						std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate lVector around orientation
  turtleStack.back()->lVector.Rotate(angleToRotate, turtleStack.back()->orientation);
}

void StochasticBranching::Slash::print(std::ostream& os) const {
  os << "/(" << m_angle << ")";
}


//Rotate around vector L by an angle in degrees.
StochasticBranching::Ampersand::Ampersand(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> StochasticBranching::Ampersand::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Ampersand(m_constructor, m_angle)) );
  return results;
}

void StochasticBranching::Ampersand::processTurtles(std::vector<Turtle*>& turtleStack,
						    std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  //Rotate orientation around L vector
  turtleStack.back()->orientation.Rotate(angleToRotate, turtleStack.back()->lVector);
} 

void StochasticBranching::Ampersand::print(std::ostream& os) const {
  os << "&(" << m_angle << ")";
}


//Attempt to rotate to the vertical
StochasticBranching::Verticate::Verticate(TreeConstructionInterface* constructor, double angle) : TreeSystemInterface(constructor) {
  m_angle = angle;
}

std::vector<LSysPtr> StochasticBranching::Verticate::applyRule() {
  std::vector<LSysPtr> results;
  results.push_back(LSysPtr( new Verticate(m_constructor, m_angle)) );
  return results;
}

void StochasticBranching::Verticate::processTurtles(std::vector<Turtle*>& turtleStack,
						    std::vector<Turtle*>& /*retiredTurtles*/){
  //Get rotation angle in radians
  double angleToRotate = m_angle * (M_PI/180.0);
  
  TVector3 trialVector1(turtleStack.back()->orientation); 
  TVector3 trialVector2(turtleStack.back()->orientation); 
  trialVector1.Rotate(angleToRotate, turtleStack.back()->lVector);
  trialVector2.Rotate(-angleToRotate, turtleStack.back()->lVector);

  //Choose the vector with the smallest angle wrt the vertical vector
  TVector3 vertical(0.0, 0.0, 1.0);
  
  if (trialVector1.Angle(vertical) < trialVector2.Angle(vertical)) {
    turtleStack.back()->orientation = trialVector1;
  } else {
    turtleStack.back()->orientation = trialVector2;
  }
} 

void StochasticBranching::Verticate::print(std::ostream& os) const {
  os << "V(" << m_angle << ")";
}


//Controls the growth (Doesn't draw anything!)
StochasticBranching::A::A(TreeConstructionInterface* constructor, double branchProbabilityThreshold, int iterationCount) : TreeSystemInterface(constructor) {
  m_branchProbabilityThreshold = branchProbabilityThreshold;
  m_iterationCount = iterationCount;
}

std::vector<LSysPtr> StochasticBranching::A::applyRule() {
  std::vector<LSysPtr> results;
  
  std::uniform_real_distribution<double> distribution(0.0, 1.0);

  results.push_back( LSysPtr(new Exclame(m_constructor, m_constructor->getDoubleParameter("initialWidth"), m_constructor->getDoubleParameter("widthIncreaseRate"))) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale"))) );
  
  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  if ( distribution(randomEngine) < m_branchProbabilityThreshold) {
    results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
    results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
    results.push_back( LSysPtr(new A(m_constructor, m_branchProbabilityThreshold*m_constructor->getDoubleParameter("branchProbReduction") , m_iterationCount+1)) );
  } else {
    if (m_iterationCount > m_constructor->getIntegerParameter("leafIterationNumber")) {
      results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
      results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
      results.push_back( LSysPtr(new B(m_constructor, m_constructor->getIntegerParameter("totalLeafIterations") )) );
    }
  }
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  results.push_back( LSysPtr(new Slash(m_constructor, m_constructor->getDoubleParameter("divergenceAngle1") )) );
  
  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  if ( distribution(randomEngine) < m_branchProbabilityThreshold) {
    results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
    results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
    results.push_back( LSysPtr(new A(m_constructor, m_branchProbabilityThreshold*m_constructor->getDoubleParameter("branchProbReduction"), m_iterationCount+1)) );
  } else {
    if (m_iterationCount > m_constructor->getIntegerParameter("leafIterationNumber")) {
      results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
      results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
      results.push_back( LSysPtr(new B(m_constructor, m_constructor->getIntegerParameter("totalLeafIterations"))) );
    }
  }
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  results.push_back( LSysPtr(new Slash(m_constructor, m_constructor->getDoubleParameter("divergenceAngle2") )) );

  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  if ( distribution(randomEngine) < m_branchProbabilityThreshold) {
    results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle") )) );
    results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale") )) );
    results.push_back( LSysPtr(new A(m_constructor, m_branchProbabilityThreshold*m_constructor->getDoubleParameter("branchProbReduction"), m_iterationCount+1)) );
  } else {
    if (m_iterationCount > m_constructor->getIntegerParameter("leafIterationNumber")) {
      results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle"))) );
      results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale"))) );
      results.push_back( LSysPtr(new B(m_constructor, m_constructor->getIntegerParameter("totalLeafIterations"))) );
    }
  }
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  
  return results;
} 

void StochasticBranching::A::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					    std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void StochasticBranching::A::print(std::ostream& os) const {
  os << "A";
}

std::default_random_engine StochasticBranching::A::randomEngine;


//Controls the end of branch growth (Doesn't draw anything!)
StochasticBranching::B::B(TreeConstructionInterface* constructor, int iterationCount) : TreeSystemInterface(constructor) {
  m_iterationCount = iterationCount;
}

std::vector<LSysPtr> StochasticBranching::B::applyRule() {
  std::vector<LSysPtr> results;
  
  if (m_iterationCount < 0) {
    //Limit the number of iterations applied when creating leaf branches
    return results;
  }

  results.push_back( LSysPtr(new Exclame(m_constructor, m_constructor->getDoubleParameter("initialWidth"), m_constructor->getDoubleParameter("widthIncreaseRate") )) );
  results.push_back( LSysPtr(new Verticate(m_constructor, m_constructor->getDoubleParameter("angleToVertical") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale2") )) );
  
  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle2") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale2") )) );
  results.push_back( LSysPtr(new B(m_constructor, m_iterationCount -1)) );
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  results.push_back( LSysPtr(new Slash(m_constructor, m_constructor->getDoubleParameter("divergenceAngle1") )) );
  
  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle2") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale2") )) );
  results.push_back( LSysPtr(new B(m_constructor, m_iterationCount -1)) );
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  results.push_back( LSysPtr(new Slash(m_constructor, m_constructor->getDoubleParameter("divergenceAngle2") )) );

  results.push_back( LSysPtr(new LeftBracket(m_constructor)) );
  results.push_back( LSysPtr(new Ampersand(m_constructor, m_constructor->getDoubleParameter("branchingAngle2") )) );
  results.push_back( LSysPtr(new F(m_constructor, m_constructor->getDoubleParameter("lengthScale2") )) );
  results.push_back( LSysPtr(new B(m_constructor, m_iterationCount -1)) );
  results.push_back( LSysPtr(new RightBracket(m_constructor)) );
  
  return results;
} 

void StochasticBranching::B::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					    std::vector<Turtle*>& /*retiredTurtles*/){
  //Do nothing...
}

void StochasticBranching::B::print(std::ostream& os) const {
  os << "B(" <<  m_iterationCount << ")";
}


//Prepare the random number generator
StochasticBranching::Rand::Rand(TreeConstructionInterface* constructor) : TreeSystemInterface(constructor) {}

std::vector<LSysPtr> StochasticBranching::Rand::applyRule() {
  
  //Set the random generator seed as early as possible
  A::randomEngine.seed(m_constructor->getIntegerParameter("seed"));

  std::vector<LSysPtr> results; //Then forget about it  
  return results;
} 

void StochasticBranching::Rand::processTurtles(std::vector<Turtle*>& /*turtleStack*/,
					       std::vector<Turtle*>& /*retiredTurtles*/){
  //This should never be called
}

void StochasticBranching::Rand::print(std::ostream& os) const {
  os << "RandomSeed(" << m_constructor->getIntegerParameter("seed")  << ")";
}
