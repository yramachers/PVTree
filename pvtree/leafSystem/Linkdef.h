#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class std::vector<std::string>+;
#pragma link C++ class std::map<std::string, double>+;
#pragma link C++ class std::map<std::string, int>+;
#pragma link C++ class std::map<std::string, std::pair<double, double> >+;
#pragma link C++ class std::map<std::string, std::pair<int, int> >+;
#pragma link C++ class LeafConstructionInterface+;
#pragma link C++ class CordateConstruction+;
#pragma link C++ class RoseConstruction+;
#pragma link C++ class SimpleConstruction+;
#pragma link C++ class PlanarConstruction+;

#endif // __CINT__
