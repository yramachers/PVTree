#ifndef PVTREE_MATERIAL_MATERIAL_FACTORY_HPP
#define PVTREE_MATERIAL_MATERIAL_FACTORY_HPP

/* @file
 * \brief Factory which will provide Geant4 materials
 *        and related optical surface properties built
 *        from configuration files.
 */
#include "G4Material.hh"
#include "G4OpticalSurface.hh"

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace libconfig{
  class Config;
}


/*! \brief Factory which will provide Geant4 materials
 *         and related optical surface properties built
 *         from configuration files.
 *
 * Follows the singleton pattern so all access to methods
 * is made through a static instance.
 */
class MaterialFactory {
private:
  
  // List of currently opened configuration files
  std::vector<libconfig::Config* >          m_openedConfigurations;

  // List of all the materials that can be found in configuration
  std::vector<std::string>                  m_availableMaterialNames;

  // map of materials to config files
  std::map<std::string, libconfig::Config*> m_materialConfigurations;

  // map of materials to constructed Geant4 material
  std::map<std::string, G4Material*>        m_geant4Materials;

  // map of materials to constructed optical surface
  std::map<std::string, G4OpticalSurface*>  m_geant4OpticalSurfaces;

  /*! \brief Prevent construction of additional instances
   *
   */
  MaterialFactory();

  /*! \brief Prevent copy construction of instances
   *
   */
  MaterialFactory(MaterialFactory& materialFactory);

  /*! \brief Clean up.
   *
   */
  ~MaterialFactory();

  /*! \brief Check if a file exists for a given path
   *
   */
  bool fileExists(std::string filePath);

  /*! \brief Attempt to read in configuration file with
   *         some standard parse checking applied by libconfig.
   *
   */
  bool openConfigurationFile(std::string fileName, libconfig::Config* cfg);  

  /*! \brief Obtain the set of configurations described
   *         by the path.
   */
  bool collectConfigurations(std::string configPath);

  /*! \brief Parse the specified configuration to construct the
   *         Geant4 material.
   *
   */
  void parseConfigurationForMaterial(std::string materialName);

  /*! \brief Parse the specified configuration to construct the
   *         Geant4 surface.
   *
   */
  void parseConfigurationForSurface(std::string materialName);

  /*! \brief Translate string to material state
   *
   */
  G4State translateState(std::string input);

  /*! \brief Translate string to optical surface type
   *
   */
  G4SurfaceType translateSurfaceType(std::string input);

  /*! \brief Translate string to optical surface finish
   *
   */
  G4OpticalSurfaceFinish translateSurfaceFinish(std::string input);

  /*! \brief Translate string to optical surface finish
   *
   */
  G4OpticalSurfaceModel translateSurfaceModel(std::string input);

public:

  /*! \brief Retrieve the singleton reference to this factory.
   */
  static MaterialFactory* instance();
  
  /*! \brief Pass additional configuration file.
   */
  bool addConfigurationFile(std::string configurationFileName);

  /*! \brief Retrieve a Geant4 material with a given name that
   *         should be found within configuration files.
   *
   * Will lazily construct the Geant4 material when first requested.
   */
  G4Material* getMaterial(std::string materialName);

  /*! \brief Retrieve a Geant4 optical surface with a given name that
   *         should be found within configuration files.
   *
   * Will lazily construct the Geant4 optical surface when first requested.
   */
  G4OpticalSurface* getOpticalSurface(std::string materialName);
};

#endif //PVTREE_MATERIAL_MATERIAL_FACTORY_HPP
