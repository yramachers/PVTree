#include "pvtree/full/material/materialFactory.hpp"

#include "pvtree/utils/resource.hpp"

#include "G4Element.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4NistManager.hh"

#include <libconfig.h++>
#include <fstream>
#include <iomanip>
#include <algorithm>

MaterialFactory::MaterialFactory() {}

MaterialFactory::MaterialFactory(MaterialFactory& /*materialFactory*/) {}

MaterialFactory::~MaterialFactory() {
  // Delete any opened configuration files
  for (auto& config : m_openedConfigurations) {
    delete config;
  }
  m_openedConfigurations.clear();

  // Do not need to delete the constructed Geant4
  // materials or optical surfaces as that should
  // be handled by Geant4. How kind.
}

MaterialFactory* MaterialFactory::instance() {
  static MaterialFactory materialFactory;
  return &materialFactory;
}

bool MaterialFactory::addConfigurationFile(std::string configurationFileName) {
  // Start with the path specified
  bool initialFileOpened = collectConfigurations(configurationFileName);

  if (!initialFileOpened) {
    return initialFileOpened;
  }

  // Then expand to any 'extra' configuration specified within
  // configuration files already loaded. Can't use the include
  // mechanics because full path not known.
  bool allFilesOpened = true;
  try {
    if (m_openedConfigurations.back()->exists("extraConfiguration")) {
      libconfig::Setting& extraConfiguration =
          m_openedConfigurations.back()->lookup("extraConfiguration");

      for (int c = 0; c < extraConfiguration.getLength(); c++) {
        std::string extraFile = extraConfiguration[c];
        bool currentFileOpened = collectConfigurations(extraFile);

        if (!currentFileOpened) {
          allFilesOpened = false;
        }
      }
    }
  } catch (const libconfig::SettingNotFoundException& nfex) {
    std::cerr << "No 'extra configuration' setting in configuration file."
              << std::endl;
  }

  return allFilesOpened;
}

G4Material* MaterialFactory::getMaterial(std::string materialName) {
  // Check if the material has already been constructed
  if (m_geant4Materials.find(materialName) != m_geant4Materials.end()) {
    return m_geant4Materials[materialName];
  }

  // Check if a configuration file contains the material requested
  if (m_materialConfigurations.find(materialName) !=
      m_materialConfigurations.end()) {
    // Should be possible to construct
    parseConfigurationForMaterial(materialName);

    if (m_geant4Materials.find(materialName) != m_geant4Materials.end()) {
      return m_geant4Materials[materialName];
    }
  }

  // Problem creating/finding material
  std::cerr << "Material configuration for " << materialName
            << " cannot be found." << std::endl;
  return 0;
}

G4OpticalSurface* MaterialFactory::getOpticalSurface(std::string materialName) {
  if (m_geant4OpticalSurfaces.find(materialName) !=
      m_geant4OpticalSurfaces.end()) {
    return m_geant4OpticalSurfaces[materialName];
  }

  // Check if a configuration file contains the surface requested
  if (m_materialConfigurations.find(materialName) !=
      m_materialConfigurations.end()) {
    // Should be possible to construct
    parseConfigurationForSurface(materialName);

    if (m_geant4OpticalSurfaces.find(materialName) !=
        m_geant4OpticalSurfaces.end()) {
      return m_geant4OpticalSurfaces[materialName];
    }
  }

  // Problem creating/finding surface
  std::cerr << "Optical surface configuration for " << materialName
            << " cannot be found." << std::endl;
  return 0;
}

bool MaterialFactory::openConfigurationFile(std::string fileName,
                                            libconfig::Config* cfg) {
  // Read the file. If there is an error, report it and exit.
  try {
    cfg->readFile(fileName.c_str());
  } catch (const libconfig::FileIOException& fioex) {
    std::cerr << "I/O error while reading file." << std::endl;
    return false;
  } catch (const libconfig::ParseException& pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return false;
  }

  // Check for material definition.
  if (cfg->exists("material.name")) {
    std::string materialName = cfg->lookup("material.name");

    if (std::find(m_availableMaterialNames.begin(),
                  m_availableMaterialNames.end(),
                  materialName) != m_availableMaterialNames.end()) {
      // Already have a material with this name.
      std::cerr << "Already have a material with name " << materialName
                << std::endl;
      return false;
    } else {
      // Record the material stored within a particular configuration file.
      m_availableMaterialNames.push_back(materialName);
      m_materialConfigurations[materialName] = cfg;
    }
  }

  return true;
}

bool MaterialFactory::fileExists(std::string filePath) {
  std::ifstream localTest(filePath.c_str());

  if (localTest.is_open()) {
    return true;
  }

  return false;
}

bool MaterialFactory::collectConfigurations(std::string configPath) {
  libconfig::Config* cfg = new libconfig::Config;

  if (fileExists(configPath)) {
    bool isFileOpen = openConfigurationFile(configPath, cfg);

    if (!isFileOpen) {
      delete cfg;
      return false;
    }

  } else {
    // Not a local file so look in the installed share directory
    std::string shareFilePath = pvtree::getConfigFile("config/material/" + configPath);

    if (fileExists(shareFilePath)) {
      bool isFileOpen = openConfigurationFile(shareFilePath, cfg);

      if (!isFileOpen) {
        delete cfg;
        return false;
      }
    } else {
      // Not in either place so give up!
      std::cerr << "Unable to locate file " << configPath
          << " locally or in the shared config." << std::endl;
      delete cfg;
      return false;
    }
  }

  m_openedConfigurations.push_back(cfg);
  return true;
}

void MaterialFactory::parseConfigurationForMaterial(std::string materialName) {
  libconfig::Config* cfg = m_materialConfigurations[materialName];

  // Initially check all required information is present
  if (!cfg->exists("material.name") || !cfg->exists("material.version") ||
      !cfg->exists("material.density") || !cfg->exists("material.state") ||
      !cfg->exists("material.composition")) {
    // Missing something important
    std::cerr << "Missing standard information in " << materialName
              << std::endl;
    return;
  }

  G4Material* material;

  // Extract some necessary details
  std::string materialStateString = "";
  double materialDensity = 0.0;

  cfg->lookupValue("material.state", materialStateString);
  cfg->lookupValue("material.density", materialDensity);

  materialDensity = materialDensity * (g / cm3);  // convert to correct units

  G4State materialState = translateState(materialStateString);

  if (cfg->exists("material.composition.baseMaterial")) {
    // Build from previous material
    std::string baseMaterialName =
        cfg->lookup("material.composition.baseMaterial");

    G4NistManager* man = G4NistManager::Instance();
    G4Material* baseMaterial = man->FindOrBuildMaterial(baseMaterialName);

    // Construct material based upon base
    material = new G4Material(materialName, materialDensity, baseMaterial,
                              materialState);

  } else if (cfg->exists("material.composition.elements")) {
    G4NistManager* man = G4NistManager::Instance();

    // Build from a list of elements
    libconfig::Setting& elements = cfg->lookup("material.composition.elements");
    int materialElementNumber = elements.getLength();

    std::vector<G4Element*> constructedElements;
    std::vector<int> atomNumbers;

    for (int e = 0; e < materialElementNumber; e++) {
      int numberOfAtoms = 0;
      std::string elementName = "";

      elements[e].lookupValue("atomNumber", numberOfAtoms);
      elements[e].lookupValue("name", elementName);

      G4Element* element = man->FindOrBuildElement(elementName);

      constructedElements.push_back(element);
      atomNumbers.push_back(numberOfAtoms);
    }

    // Now construct the material from the given elements
    material = new G4Material(materialName, materialDensity,
                              materialElementNumber, materialState);

    for (int e = 0; e < materialElementNumber; e++) {
      material->AddElement(constructedElements[e], atomNumbers[e]);
    }

  } else {
    std::cerr << "Unable to parse the composition information in "
              << materialName << std::endl;
    return;
  }

  // Now need to interpret additional material properties (if there are any)
  if (cfg->exists("material.defaultPhotonEnergies") &&
      cfg->exists("material.properties")) {
    G4MaterialPropertiesTable* materialPropertyTable =
        new G4MaterialPropertiesTable();

    libconfig::Setting& configEnergies =
        cfg->lookup("material.defaultPhotonEnergies");

    // Get the photon energies (in eV)
    std::vector<double> photonEnergies;

    for (int e = 0; e < configEnergies.getLength(); e++) {
      double energy = (double)configEnergies[e];

      photonEnergies.push_back(energy * eV);
    }

    // Now go through each of the properties (adding to the material property
    // table
    // as we go along.
    libconfig::Setting& properties = cfg->lookup("material.properties");

    for (int p = 0; p < properties.getLength(); p++) {
      std::string propertyName = properties[p]["name"];
      libconfig::Setting& values = properties[p]["values"];

      // Need to build a property vector as do not know the number
      // of value a priori
      G4MaterialPropertyVector* propertyVector = new G4MaterialPropertyVector();

      for (int v = 0; v < values.getLength(); v++) {
        double propertyValue = (double)values[v];

        propertyVector->InsertValues(photonEnergies[v], propertyValue);
      }

      materialPropertyTable->AddProperty(propertyName.c_str(), propertyVector);
    }
    //    materialPropertyTable->DumpTable();
    material->SetMaterialPropertiesTable(materialPropertyTable);
  }

  m_geant4Materials[materialName] = material;
}

void MaterialFactory::parseConfigurationForSurface(std::string materialName) {
  libconfig::Config* cfg = m_materialConfigurations[materialName];

  // Initially check all required information is present
  if (!cfg->exists("material.name") || !cfg->exists("material.surface")) {
    // Missing something important
    std::cerr << "Missing standard information in " << materialName
              << std::endl;
    return;
  }

  // Extract some necessary details
  std::string name = materialName + "-surface";
  std::string typeString = "";
  std::string finishString = "";
  std::string modelString = "";

  cfg->lookupValue("material.surface.type", typeString);
  cfg->lookupValue("material.surface.finish", finishString);
  cfg->lookupValue("material.surface.model", modelString);

  // Translate into Geant4 properties
  G4SurfaceType surfaceType = translateSurfaceType(typeString);
  G4OpticalSurfaceFinish surfaceFinish = translateSurfaceFinish(finishString);
  G4OpticalSurfaceModel surfaceModel = translateSurfaceModel(modelString);

  G4OpticalSurface* opticalSurface = new G4OpticalSurface(name);
  opticalSurface->SetType(surfaceType);
  opticalSurface->SetFinish(surfaceFinish);
  opticalSurface->SetModel(surfaceModel);

  // Now need to interpret additional surface optical material properties (if
  // there are any)
  if (cfg->exists("material.surface.defaultPhotonEnergies") &&
      cfg->exists("material.surface.properties")) {
    G4MaterialPropertiesTable* materialPropertyTable =
        new G4MaterialPropertiesTable();

    libconfig::Setting& configEnergies =
        cfg->lookup("material.surface.defaultPhotonEnergies");

    // Get the photon energies (in eV)
    std::vector<double> photonEnergies;

    for (int e = 0; e < configEnergies.getLength(); e++) {
      double energy = (double)configEnergies[e];

      photonEnergies.push_back(energy * eV);
    }

    // Now go through each of the properties (adding to the material property
    // table
    // as we go along.
    libconfig::Setting& properties = cfg->lookup("material.surface.properties");

    for (int p = 0; p < properties.getLength(); p++) {
      std::string propertyName = properties[p]["name"];
      libconfig::Setting& values = properties[p]["values"];

      // Need to build a property vector as do not know the number
      // of value a priori
      G4MaterialPropertyVector* propertyVector = new G4MaterialPropertyVector();

      for (int v = 0; v < values.getLength(); v++) {
        double propertyValue = (double)values[v];

        propertyVector->InsertValues(photonEnergies[v], propertyValue);
      }

      materialPropertyTable->AddProperty(propertyName.c_str(), propertyVector);
    }

    opticalSurface->SetMaterialPropertiesTable(materialPropertyTable);
  }

  m_geant4OpticalSurfaces[materialName] = opticalSurface;
}

G4State MaterialFactory::translateState(std::string input) {
  G4State currentState = kStateUndefined;

  std::map<std::string, G4State> stateEnumeration = {
      {"solid", kStateSolid},
      {"liquid", kStateLiquid},
      {"gas", kStateGas},
      {"undefined", kStateUndefined}};

  if (stateEnumeration.find(input) != stateEnumeration.end()) {
    currentState = stateEnumeration[input];
  } else {
    std::cerr << "Unknown state specified " << input << std::endl;
  }

  return currentState;
}

G4SurfaceType MaterialFactory::translateSurfaceType(std::string input) {
  G4SurfaceType surfaceType = dielectric_dielectric;

  std::map<std::string, G4SurfaceType> surfaceTypeEnumeration = {
      {"dielectric_metal", dielectric_metal},
      {"dielectric_dielectric", dielectric_dielectric},
      {"dielectric_LUT", dielectric_LUT},
      {"dielectric_dichroic", dielectric_dichroic},
      {"firsov", firsov},
      {"x_ray", x_ray}};

  // Check if input is present
  if (surfaceTypeEnumeration.find(input) != surfaceTypeEnumeration.end()) {
    surfaceType = surfaceTypeEnumeration[input];
  } else {
    std::cerr << "Unknown surface type specified " << input << std::endl;
  }

  return surfaceType;
}

G4OpticalSurfaceFinish MaterialFactory::translateSurfaceFinish(
    std::string input) {
  G4OpticalSurfaceFinish surfaceFinish = ground;

  std::map<std::string, G4OpticalSurfaceFinish> finishEnumeration = {
      {"polished", polished},
      {"polishedfrontpainted", polishedfrontpainted},
      {"polishedbackpainted", polishedbackpainted},
      {"ground", ground},
      {"groundfrontpainted", groundfrontpainted},
      {"groundbackpainted", groundbackpainted},
      {"polishedlumirrorair", polishedlumirrorair},
      {"polishedlumirrorglue", polishedlumirrorglue},
      {"polishedair", polishedair},
      {"polishedteflonair", polishedteflonair},
      {"polishedtioair", polishedtioair},
      {"polishedtyvekair", polishedtyvekair},
      {"polishedvm2000air", polishedvm2000air},
      {"polishedvm2000glue", polishedvm2000glue},
      {"etchedlumirrorair", etchedlumirrorair},
      {"etchedlumirrorglue", etchedlumirrorglue},
      {"etchedair", etchedair},
      {"etchedteflonair", etchedteflonair},
      {"etchedtioair", etchedtioair},
      {"etchedtyvekair", etchedtyvekair},
      {"etchedvm2000air", etchedvm2000air},
      {"etchedvm2000glue", etchedvm2000glue},
      {"groundlumirrorair", groundlumirrorair},
      {"groundlumirrorglue", groundlumirrorglue},
      {"groundair", groundair},
      {"groundteflonair", groundteflonair},
      {"groundtioair", groundtioair},
      {"groundtyvekair", groundtyvekair},
      {"groundvm2000air", groundvm2000air},
      {"groundvm2000glue", groundvm2000glue}};

  // Check if input is present
  if (finishEnumeration.find(input) != finishEnumeration.end()) {
    surfaceFinish = finishEnumeration[input];
  } else {
    std::cerr << "Unknown surface finish specified " << input << std::endl;
  }

  return surfaceFinish;
}

G4OpticalSurfaceModel MaterialFactory::translateSurfaceModel(
    std::string input) {
  G4OpticalSurfaceModel surfaceModel = unified;

  std::map<std::string, G4OpticalSurfaceModel> modelEnumeration = {
      {"glisur", glisur},
      {"unified", unified},
      {"LUT", LUT},
      {"dichroic", dichroic}};

  // Check if input is present
  if (modelEnumeration.find(input) != modelEnumeration.end()) {
    surfaceModel = modelEnumeration[input];
  } else {
    std::cerr << "Unknown surface model specified " << input << std::endl;
  }

  return surfaceModel;
}
