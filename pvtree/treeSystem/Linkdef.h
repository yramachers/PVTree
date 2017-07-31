#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class std::vector < std::string > +;
#pragma link C++ class std::map < std::string, double > +;
#pragma link C++ class std::map < std::string, int > +;
#pragma link C++ class std::map < std::string, std::pair < double, double >> +;
#pragma link C++ class std::map < std::string, std::pair < int, int >> +;
#pragma link C++ class TreeConstructionInterface + ;
#pragma link C++ class HelicalConstruction + ;
#pragma link C++ class MonopodialConstruction + ;
#pragma link C++ class StochasticConstruction + ;
#pragma link C++ class SympodialConstruction + ;
#pragma link C++ class TernaryConstruction + ;
#pragma link C++ class StumpConstruction + ;

#endif  // __CINT__
