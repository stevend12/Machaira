// Machaira: Main.cpp
// GUI viewer for SWORD Project files using wxWidgets
// This file is the main program, including UI (wxWidgets)
// Steven Dolly
// Created: November 9, 2021
// Current version: Pre-release

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif
#include <wx/listctrl.h>

#include "SwordBackend.hpp"

SwordBackend SwordApp;

class MyApp: public wxApp
{
  public:
    virtual bool OnInit();
};
wxDECLARE_APP(MyApp);

class MainFrame: public wxFrame
{
  public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    wxTextCtrl * ScriptureTextCtrl;
    wxTextCtrl * CommentaryTextCtrl;
  private:
    void OnExit(wxCommandEvent& event);
    void AddModule(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};

class InstallerFrame: public wxFrame
{
public:
  InstallerFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
  wxListCtrl * ModuleListCtrl;
  //wxTextCtrl * CommentaryTextCtrl;
private:
  void OnExit(wxCommandEvent& event);
  //void AddModule(wxCommandEvent& event);
  wxDECLARE_EVENT_TABLE();
};

enum
{
  ID_Add = wxID_HIGHEST + 1
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(ID_Add, MainFrame::AddModule)
  EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(InstallerFrame, wxFrame)
  EVT_MENU(wxID_EXIT, InstallerFrame::OnExit)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);


bool MyApp::OnInit()
{
  MainFrame * frame = new MainFrame("Machaira", wxPoint(50, 50),
    wxSize(1000, 800));
  frame->Show(true);
	SetTopWindow(frame);

	return true;
}

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
  /////////////////////
  // Menu Bar at Top //
  /////////////////////
  wxMenu * menuFile = new wxMenu;
  menuFile->Append(ID_Add, "&Add Module\tCtrl-A", "Add SWORD Module");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);
  wxMenuBar * menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  //menuBar->Append(menuHelp, "&Help");
  SetMenuBar(menuBar);

  // Main Panel
  wxPanel * panel = new wxPanel(this, wxID_ANY);

  //////////////////////////////////////////
  // Text Control for Scripture Reference //
  //////////////////////////////////////////
  ScriptureTextCtrl = new wxTextCtrl(panel, wxID_ANY, "Scripture Text",
    wxPoint(50, 100), wxSize(400, 200), wxTE_READONLY | wxTE_MULTILINE);

  /////////////////////////////////
  // Text Control for Commentary //
  /////////////////////////////////
  CommentaryTextCtrl = new wxTextCtrl(panel, wxID_ANY, "Commentary Text",
    wxPoint(500, 100), wxSize(400, 400), wxTE_READONLY | wxTE_MULTILINE);

  ////////////////////////////
  // Status Bar (at bottom) //
  ////////////////////////////
  CreateStatusBar();
  std::string initial_status("Welcome to Machaira!");
  initial_status += " Using "+SwordApp.GetSwordVersion();
  SetStatusText(initial_status);
}

void MainFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

void MainFrame::AddModule(wxCommandEvent& event)
{
  //wxMessageBox(wxT("Not ready yet!"), wxT("Whoops!"), wxICON_INFORMATION);
  InstallerFrame * installer_frame = new InstallerFrame("Module Installer",
    wxPoint(200, 200), wxSize(800, 600));
  installer_frame->Show(true);
	wxGetApp().SetTopWindow(installer_frame);
}

InstallerFrame::InstallerFrame(const wxString& title, const wxPoint& pos,
  const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
  /////////////////////
  // Menu Bar at Top //
  /////////////////////
  wxMenu * menuFile = new wxMenu;
  menuFile->Append(ID_Add, "&Add Source\tCtrl-A", "Add Module Source");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);
  wxMenuBar * menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  //menuBar->Append(menuHelp, "&Help");
  SetMenuBar(menuBar);

  // Main Panel
  wxPanel * panel = new wxPanel(this, wxID_ANY);

  //////////////////////////////
  // List Control for Modules //
  //////////////////////////////
  ModuleListCtrl = new wxListCtrl(panel, wxID_ANY, wxPoint(50, 100),
    wxSize(370, 500), wxLC_REPORT | wxLC_HRULES);
  ModuleListCtrl->InsertColumn(0, "Module Name", wxLIST_FORMAT_LEFT, 120);
  ModuleListCtrl->InsertColumn(1, "Type", wxLIST_FORMAT_LEFT, 120);
  ModuleListCtrl->InsertColumn(1, "Version", wxLIST_FORMAT_LEFT, 120);

  if(!SwordApp.HasInstallerConfig())
  {
    SwordApp.InitInstallerConfig();
  }
  std::vector<SwordModuleInfo> mod_list = SwordApp.GetRemoteSourceModules();
  for(int n = 0; n < mod_list.size(); n++)
  {
    ModuleListCtrl->InsertItem(n, mod_list[n].Name);
    ModuleListCtrl->SetItem(n, 1, mod_list[n].Type);
    ModuleListCtrl->SetItem(n, 2, mod_list[n].Version);
  }

  ////////////////////////////
  // Status Bar (at bottom) //
  ////////////////////////////
  CreateStatusBar();
  std::string initial_status("Module Installer");
  SetStatusText(initial_status);
}

void InstallerFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}
