// Machaira: SwordBackend.cpp
// GUI viewer for SWORD Project files using wxWidgets
// This file is the backend, using SWORD Project to find the appropriate
// texts and to deliver them to the UI
// Steven Dolly
// Created: November 9, 2021
// Current version: Pre-release

#include "SwordBackend.hpp"

#include <iostream>
#include <sstream>
#include <filesystem>

#include <swversion.h>
#include <swmgr.h>
#include <installmgr.h>
#include <filemgr.h>
#include <markupfiltmgr.h>

SwordBackend::SwordBackend()
{
  base_dir = "./";
  library_dir = ".sword";
  install_manager_dir = ".sword/InstallMgr";

  default_source = "CrossWire";
}

bool SwordBackend::HasInstallerConfig()
{
  std::string config_path = base_dir + install_manager_dir +
    std::string("/InstallMgr.conf");
  return std::filesystem::exists(config_path.c_str());
}

void SwordBackend::InitInstallerConfig()
{
  sword::SWBuf confPath = (base_dir + install_manager_dir).c_str();
  confPath += "/InstallMgr.conf";
  sword::FileMgr::createParent(confPath.c_str());
  remove(confPath.c_str());

  sword::SWConfig config(confPath.c_str());
  config["General"]["PassiveFTP"] = "true";
  config["General"]["TimeoutMillis"] = "10000";
  config["General"]["UnverifiedPeerAllowed"] = "true";

  sword::InstallSource is("FTP");
  is.caption = "CrossWire";
  is.source = "ftp.crosswire.org";
  is.directory = "/pub/sword/raw";
  config["Sources"]["FTPSource"] = is.getConfEnt();

  config.save();
}

std::vector<SwordModuleInfo>
  SwordBackend::GetRemoteSourceModules(std::string src_name)
{
  if(src_name == "") src_name = default_source;
  sword::InstallMgr install_mgr((base_dir + install_manager_dir).c_str());
  install_mgr.setUserDisclaimerConfirmed(true);
  sword::InstallSourceMap::iterator source =
    install_mgr.sources.find(src_name.c_str());
  if(source == install_mgr.sources.end())
  {
    std::cout << "Error: Couldn't find remote source " << src_name << '\n';
  }
  else
  {
    std::cout << "Found source " << src_name << '\n';
    if(!install_mgr.refreshRemoteSource(source->second))
    {
      std::cout << "Remote source " << src_name << " refreshed\n";
    }
  	else std::cout << "Error refreshing remote source " << src_name << "\n";
  }

  std::vector<SwordModuleInfo> module_info_list;
  sword::ModMap::iterator list_it = source->second->getMgr()->Modules.begin();
  sword::ModMap::iterator list_end = source->second->getMgr()->Modules.end();
  for(list_it; list_it != list_end; list_it++)
  {
    SwordModuleInfo temp_module;
  	sword::SWModule * module = (*list_it).second;
    temp_module.Name = module->Name();
    temp_module.Type = module->Type();
    temp_module.Description = module->getDescription();
    temp_module.Version = module->getConfigEntry("Version");
    module_info_list.push_back(temp_module);
  }

  return module_info_list;
}

std::string SwordBackend::GetSwordVersion()
{
  sword::SWVersion retval;
	retval = sword::SWVersion::currentVersion;
  std::stringstream ss;
  ss << "SWORD Version: " << retval;
  return ss.str();
}
