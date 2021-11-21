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
#include <filemgr.h>
#include <markupfiltmgr.h>
#include <swoptfilter.h>

SwordBackend::SwordBackend() :
  library_mgr("./.sword", true, new sword::MarkupFilterMgr(sword::FMT_XHTML)),
  install_mgr("./.sword/InstallMgr")
{
  library_dir = "./.sword";
  install_manager_dir = "./.sword/InstallMgr";
  default_source = "CrossWire";
  install_mgr.setUserDisclaimerConfirmed(true);
  if(!library_mgr.config) std::cout << "SWORD configuration not found.\n";
  InitializeAppModules();
}

SwordBackend::SwordBackend(SwordBackendSettings settings) :
  library_mgr(settings.LibraryDir.c_str(), true,
    new sword::MarkupFilterMgr(sword::FMT_XHTML)),
  install_mgr(settings.InstallDir.c_str())
{
  library_dir = settings.LibraryDir;
  install_manager_dir = settings.InstallDir;
  default_source = settings.DefaultSource;
  install_mgr.setUserDisclaimerConfirmed(true);
  if(!library_mgr.config) std::cout << "SWORD configuration not found.\n";
  InitializeAppModules();
}

bool SwordBackend::HasInstallerConfig()
{
  std::string config_path = install_manager_dir +
    std::string("/InstallMgr.conf");
  return std::filesystem::exists(config_path.c_str());
}

void SwordBackend::InitInstallerConfig()
{
  sword::SWBuf confPath = install_manager_dir.c_str();
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

void SwordBackend::SelectRemoteSource(std::string src_name)
{
  if(src_name == "") selected_source = default_source;
  else selected_source = src_name;
  sword::InstallSourceMap::iterator source =
    install_mgr.sources.find(selected_source.c_str());
  if(source == install_mgr.sources.end())
  {
    std::cout << "Error: Couldn't find remote source " << selected_source << '\n';
  }
  else
  {
    std::cout << "Found source " << selected_source << '\n';
    if(!install_mgr.refreshRemoteSource(source->second))
    {
      std::cout << "Remote source " << selected_source << " refreshed\n";
    }
    else std::cout << "Error refreshing remote source " << selected_source << "\n";
  }

  remote_module_info_list.clear();
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
    remote_module_info_list.push_back(temp_module);
  }
}

std::vector<SwordModuleInfo> SwordBackend::GetRemoteSourceModules()
{
  return remote_module_info_list;
}

void SwordBackend::InstallRemoteModule(std::string mod_name)
{
  sword::InstallSourceMap::iterator source =
    install_mgr.sources.find(selected_source.c_str());
  if(source == install_mgr.sources.end())
  {
    std::cout << "Error: Couldn't find selected remote source " <<
      selected_source << '\n';
  }
  else std::cout << "Found source " << selected_source << '\n';

  sword::InstallSource * is = source->second;
  sword::SWModule * module = is->getMgr()->getModule(mod_name.c_str());
  if (!module) {
    std::cout << "Remote source " << selected_source <<
      " does not make available module [" << mod_name << "]\n";
  }

  int error = install_mgr.installModule(&library_mgr, 0, module->getName(), is);
  if(error)
  {
    std::cout << "\nError installing module: [" << module->getName() <<
      "] (write permissions?)\n";
  }
  else
  {
    std::cout << "\nInstalled module: [" << module->getName() << "]\n";
    library_mgr.augmentModules(library_dir.c_str());
  }
}

void SwordBackend::InitializeAppModules()
{
  biblical_texts.clear();
  commentaries.clear();

  sword::ModMap::iterator modIterator;
  for(modIterator = library_mgr.Modules.begin();
    modIterator != library_mgr.Modules.end(); modIterator++)
  {
    sword::SWModule * module = (*modIterator).second;
    // Assign module to group
    if(std::string(module->Type()) == "Biblical Texts")
    {
      biblical_texts.push_back(module->Name());
    }
    else if(std::string(module->Type()) == "Commentaries")
    {
      commentaries.push_back(module->Name());
    }
    else std::cout << "Module " << module->Name() << " not included in app.\n";
    // Set default options for modules
    for(sword::OptionFilterList::const_iterator it =
      module->getOptionFilters().begin();
      it != module->getOptionFilters().end(); ++it)
    {
      if((std::string((*it)->getOptionName()) == "Strong's Numbers") &&
        (std::string(module->Type()) == "Biblical Texts"))
      {
        (*it)->setOptionValue("On");
      }
      else (*it)->setOptionValue("Off");
    }
  }
}

std::string SwordBackend::GetText(std::string key, std::string mod_name)
{
  std::stringstream ss;
  // Get text from SWORD (raw data)
  sword::SWKey myKey(key.c_str());
  sword::SWModule * module = library_mgr.getModule(mod_name.c_str());
  module->setKey(myKey);
  std::string input(module->renderText());
  // Convert all non-ASCII characters to HTML entities (hexadecimal format)
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs4conv;
  std::u32string ucs4 = ucs4conv.from_bytes(input);
  for(char32_t c : ucs4)
  {
    if(static_cast<uint32_t>(c) <= static_cast<uint32_t>(0x7e)) ss << char(c);
    else
    {
      ss << "&#x" << std::hex << std::setw(4) << std::setfill('0')
        << static_cast<uint32_t>(c) << '\n';
    }
  }
  input = ss.str();
  ss.clear();
  ss.str("");
  // Condense links
  size_t a_begin = 0, a_end, p1, p2;
  while(input.find("<a", a_begin) != std::string::npos)
  {
    a_begin = input.find("<a", a_begin);
    a_end = input.find("</a>", a_begin); a_end += 4;
    std::string link = input.substr(a_begin, a_end-a_begin);

    p1 = link.find("action=", 0); p1 += 7;
    p2 = link.find('&', p1);
    std::string l_action = link.substr(p1, p2-p1);

    p1 = link.find("type=", 0); p1 += 5;
    p2 = link.find('&', p1);
    std::string l_type = link.substr(p1, p2-p1);

    p1 = link.find("value=", 0); p1 += 6;
    p2 = link.find('&', p1);
    std::string l_val = link.substr(p1, p2-p1);

    p1 = link.find('>', 0); p1 += 1;
    p2 = link.find('<', p1);
    std::string l_text = link.substr(p1, p2-p1);

    ss << "<a href=\"" << l_action << '_' << l_type << '_' << l_val << "\">"
      << l_text << "</a>";

    input.erase(a_begin, a_end-a_begin);
    input.insert(a_begin, ss.str());

    a_begin += ss.str().length();
    ss.clear();
    ss.str("");
  }

  return input;
}

std::string SwordBackend::GetSwordVersion()
{
  sword::SWVersion retval;
	retval = sword::SWVersion::currentVersion;
  std::stringstream ss;
  ss << "SWORD Version: " << retval;
  return ss.str();
}

std::string SwordBackend::GetVerseRef(std::string mod_name)
{
  sword::SWKey * my_key = (library_mgr.getModule(mod_name.c_str()))->getKey();
  return std::string(my_key->getText());
}

std::string SwordBackend::IncrementVerse(std::string mod_name, int n)
{
  sword::SWKey * my_key = (library_mgr.getModule(mod_name.c_str()))->getKey();
  if(n >= 0) my_key->increment(n);
  else my_key->decrement(-1*n);
  return std::string(my_key->getText());
}
