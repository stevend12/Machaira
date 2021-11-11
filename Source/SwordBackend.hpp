// Machaira: SwordBackend.hpp
// GUI viewer for SWORD Project files using wxWidgets
// This file is the backend, using SWORD Project to find the appropriate
// texts and to deliver them to the UI
// Steven Dolly
// Created: November 9, 2021
// Current version: Pre-release

#ifndef SWORDBACKEND_HPP
#define SWORDBACKEND_HPP

#include <string>
#include <vector>

struct SwordModuleInfo
{
  std::string Name;
  std::string Type;
  std::string Description;
  std::string Version;
};

class SwordBackend
{
  public:
    // Constructor
    SwordBackend();
    // Install Manager
    bool HasInstallerConfig();
    void InitInstallerConfig();
    std::vector<SwordModuleInfo>
      GetRemoteSourceModules(std::string src_name = "");
    // Library Manager

    // Get/Set
    std::string GetBaseDir(){ return base_dir; }
    std::string GetInstallDir(){ return install_manager_dir; }
    std::string GetLibraryDir(){ return library_dir; }
    std::string GetDefaultSource(){ return default_source; }
    // Utilities
    std::string GetSwordVersion();
  private:
    // Directories
    std::string base_dir;
    std::string install_manager_dir;
    std::string library_dir;
    // Variables
    std::string default_source;
};

#endif
