// Machaira: SwordBackend.cpp
// GUI viewer for SWORD Project files using wxWidgets
// This file is the backend, using SWORD Project to find the appropriate
// texts and to deliver them to the UI
// Steven Dolly
// Created: November 9, 2021
// Current version: Pre-release

#include "SwordBackend.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <swversion.h>
#include <filemgr.h>
#include <markupfiltmgr.h>
#include <swoptfilter.h>

SwordBackend::SwordBackend() :
  library_mgr("./Res/.sword", true, new sword::MarkupFilterMgr(sword::FMT_XHTML)),
  install_mgr("./Res/.sword/InstallMgr")
{
  library_dir = "./Res/.sword";
  install_manager_dir = "./Res/.sword/InstallMgr";
  default_source = "CrossWire";

  InitializeInstaller();
  InitializeLibrary();
}

SwordBackend::SwordBackend(SwordBackendSettings settings) :
  library_mgr(settings.LibraryDir.c_str(), true,
    new sword::MarkupFilterMgr(sword::FMT_XHTML)),
  install_mgr(settings.InstallDir.c_str())
{
  library_dir = settings.LibraryDir;
  install_manager_dir = settings.InstallDir;
  default_source = settings.DefaultSource;

  InitializeInstaller();
  InitializeLibrary();
}

bool SwordBackend::HasInstallerConfig()
{
  std::string config_path = install_manager_dir +
    std::string("/InstallMgr.conf");
  return std::filesystem::exists(config_path.c_str());
}

void SwordBackend::InitInstallerConfig()
{
  // Set file path & delete installer config file if it exists
  sword::SWBuf confPath = install_manager_dir.c_str();
  confPath += "/InstallMgr.conf";
  sword::FileMgr::createParent(confPath.c_str());
  remove(confPath.c_str());
  // Set basic settings for config
  sword::SWConfig config(confPath.c_str());
  config["General"]["PassiveFTP"] = "true";
  config["General"]["TimeoutMillis"] = "10000";
  config["General"]["UnverifiedPeerAllowed"] = "true";
  // Initialize with one source (CrossWire HTTPS)
  sword::InstallSource is("HTTPS");
  is.caption = "CrossWire";
  is.source = "crosswire.org";
  is.directory = "/ftpmirror/pub/sword/raw";
  config["Sources"]["HTTPSSource"] = is.getConfEnt();
  // Save to file and read
  config.save();
  install_mgr.readInstallConf();
}

void SwordBackend::AddRemoteSourcesCSV(std::string csv_file_name)
{
  // Attempt to load sources from text file
  if(std::filesystem::exists(csv_file_name.c_str()))
  {
    size_t p1, p2;
    std::string line;
    std::ifstream fin(csv_file_name.c_str());
    while(std::getline(fin, line))
    {
      if(line[0] == '#') continue;

      p2 = line.find(',');
      std::string type = line.substr(0, p2);
      p1 = p2+1; p2 = line.find(',', p1);
      std::string confEnt = line.substr(p1, p2-p1); confEnt += '|';
      p1 = p2+1; p2 = line.find(',', p1);
      confEnt += (line.substr(p1, p2-p1) + '|');
      p1 = p2+1; p2 = line.find(',', p1);
      confEnt += line.substr(p1, p2-p1) + "|||";
      std::cout << confEnt << '\n';

      sword::InstallSource is(type.c_str(), confEnt.c_str());
      install_mgr.sources["AndBible"] = &is;
    }
    fin.close();
    // Save and re-read soucre list from config file
    install_mgr.saveInstallConf();
    install_mgr.readInstallConf();
  }
}

void SwordBackend::InitializeInstaller()
{
  if(!HasInstallerConfig())
  {
    InitInstallerConfig();
    AddRemoteSourcesCSV(install_manager_dir + std::string("/SourceList.txt"));
  }
  install_mgr.setUserDisclaimerConfirmed(true);
  remote_sources.clear();
  for(const auto & [key, value] : install_mgr.sources)
  {
    remote_sources.push_back(std::string(key));
  }
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
    temp_module.Name = module->getName();
    temp_module.Type = module->getType();
    temp_module.Language = module->getLanguage();
    temp_module.Description = module->getDescription();
    if(module->getConfigEntry("Version") == 0) temp_module.Version = "NA";
    else temp_module.Version = module->getConfigEntry("Version");
    remote_module_info_list.push_back(temp_module);
  }
}

std::vector<SwordModuleInfo> SwordBackend::GetRemoteSourceModules()
{
  return remote_module_info_list;
}

bool SwordBackend::InstallRemoteModule(std::string mod_name)
{
  sword::InstallSourceMap::iterator source =
    install_mgr.sources.find(selected_source.c_str());
  if(source == install_mgr.sources.end())
  {
    error_text = std::string("Error: Couldn't find selected remote source ") +
      selected_source;
    return false;
  }

  sword::InstallSource * is = source->second;
  sword::SWModule * module = is->getMgr()->getModule(mod_name.c_str());
  if(!module)
  {
    error_text = std::string("Remote source ") + selected_source +
      std::string(" does not make available module [") + mod_name + "]";
    return false;
  }

  int error = install_mgr.installModule(&library_mgr, 0, module->getName(), is);
  if(error)
  {
    error_text = std::string("Error installing module: [") + module->getName() +
      "] (write permissions?)";
    return false;
  }
  else
  {
    library_mgr.augmentModules(library_dir.c_str());
  }

  return true;
}

void SwordBackend::InitializeLibrary()
{
  if(!library_mgr.config) std::cout << "Warning: SWORD configuration not found.\n";

  biblical_texts.clear();
  commentaries.clear();
  dictionaries.clear();

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
    else if(std::string(module->Type()) == "Lexicons / Dictionaries")
    {
      dictionaries.push_back(module->Name());
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
    p2 = std::min(link.find('\"', p1), link.find('&', p1));
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

void SwordBackend::SetVerseRef(std::string mod_name, std::string key)
{
  library_mgr.getModule(mod_name.c_str())->setKey(key.c_str());
}

std::string SwordBackend::IncrementVerse(std::string mod_name, int n)
{
  sword::SWKey * my_key = (library_mgr.getModule(mod_name.c_str()))->getKey();
  if(n >= 0) my_key->increment(n);
  else my_key->decrement(-1*n);
  return std::string(my_key->getText());
}
