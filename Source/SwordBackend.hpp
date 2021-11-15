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

#include <swmgr.h>
#include <installmgr.h>

class SwordBackendSettings
{
  public:
    // Functions
    SwordBackendSettings();
    void Read(std::string file_name);
    void Save(std::string file_name);
    // Settings Values
    std::string LibraryDir;
    std::string InstallDir;
    std::string DefaultSource;
};

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
    SwordBackend(SwordBackendSettings settings);
    // Install Manager
    bool HasInstallerConfig();
    void InitInstallerConfig();
    void SelectRemoteSource(std::string src_name = "");
    std::vector<SwordModuleInfo> GetRemoteSourceModules();
    void InstallRemoteModule(std::string mod_name);
    // Library Manager
    std::string GetText(std::string key, std::string mod_name);
    // Get/Set
    std::string GetInstallDir(){ return install_manager_dir; }
    std::string GetLibraryDir(){ return library_dir; }
    std::string GetDefaultSource(){ return default_source; }
    // Utilities
    std::string GetSwordVersion();
  private:
    // Directories
    std::string install_manager_dir;
    std::string library_dir;
    // Variables
    std::string default_source;
    std::string selected_source;
    // Local Module Library
    sword::SWMgr library_mgr;
    // Module Installer
    sword::InstallMgr install_mgr;
    std::vector<SwordModuleInfo> remote_module_info_list;
};

#endif
