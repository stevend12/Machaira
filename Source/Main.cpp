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
#include <wx/html/htmlwin.h>

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
    wxButton * GetTextButton;
    wxTextCtrl * VerseTextCtrl;
    wxHtmlWindow * ScriptureHtmlWindow;
    wxHtmlWindow * CommentaryHtmlWindow;
  private:
    void OnExit(wxCommandEvent& event);
    void LoadText(wxCommandEvent& event);
    void AddModule(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};

class InstallerFrame: public wxFrame
{
public:
  InstallerFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
  wxButton * InstallButton;
  wxListCtrl * ModuleListCtrl;
  wxTextCtrl * ModDescriptionTextCtrl;
private:
  void OnExit(wxCommandEvent& event);
  void InstallModule(wxCommandEvent& event);
  void DisplayModuleInfo(wxListEvent& event);
  wxDECLARE_EVENT_TABLE();
};

enum
{
  ID_Get = wxID_HIGHEST + 1,
  ID_Add = wxID_HIGHEST + 2,
  ID_Install = wxID_HIGHEST + 3
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(ID_Add, MainFrame::AddModule)
  EVT_MENU(wxID_EXIT, MainFrame::OnExit)
  EVT_BUTTON(ID_Get, MainFrame::LoadText)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(InstallerFrame, wxFrame)
  EVT_MENU(wxID_EXIT, InstallerFrame::OnExit)
  EVT_BUTTON(ID_Install, InstallerFrame::InstallModule)
  EVT_LIST_ITEM_SELECTED(wxID_ANY, InstallerFrame::DisplayModuleInfo)
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
  // Menu Bar at Top
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

  // Button Control to Load Text
  GetTextButton = new wxButton(panel, ID_Get, _T("Get Verse"),
    wxPoint(50, 30), wxSize(120,30), 0);

  // Text Control for Verse Entry
  VerseTextCtrl = new wxTextCtrl(panel, wxID_ANY, "Enter Verse(s) Here",
    wxPoint(200, 30), wxSize(200,30));

  // Text Control for Scripture Reference
  ScriptureHtmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxPoint(50, 100),
    wxSize(400, 200));

  // Text Control for Commentary
  CommentaryHtmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxPoint(500, 100),
    wxSize(400, 400));

  // Status Bar at Bottom
  CreateStatusBar();
  std::string initial_status("Welcome to Machaira!");
  initial_status += " Using "+SwordApp.GetSwordVersion();
  SetStatusText(initial_status);
}

void MainFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

void MainFrame::LoadText(wxCommandEvent& event)
{
  std::string verse_txt = std::string(VerseTextCtrl->GetLineText(0));
  ScriptureHtmlWindow->SetPage(SwordApp.GetText(verse_txt, "KJV"));
  CommentaryHtmlWindow->SetPage(SwordApp.GetText(verse_txt, "Wesley"));
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
  // Menu Bar at Top
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

  // Button Control to Install Modules
  InstallButton = new wxButton(panel, ID_Install, _T("Install Module"),
    wxPoint(50, 30), wxSize(100,30), 0);

  // List Control for Modules
  ModuleListCtrl = new wxListCtrl(panel, wxID_ANY, wxPoint(40, 100),
    wxSize(420, 400), wxLC_REPORT | wxLC_HRULES | wxLC_SINGLE_SEL);
  ModuleListCtrl->InsertColumn(0, "Module Name", wxLIST_FORMAT_LEFT, 120);
  ModuleListCtrl->InsertColumn(1, "Type", wxLIST_FORMAT_LEFT, 200);
  ModuleListCtrl->InsertColumn(2, "Version", wxLIST_FORMAT_LEFT, 90);

  // Text Control for Description
  ModDescriptionTextCtrl = new wxTextCtrl(panel, wxID_ANY, "Module Description",
    wxPoint(480, 100), wxSize(300, 200), wxTE_READONLY | wxTE_MULTILINE);

  if(!SwordApp.HasInstallerConfig()) SwordApp.InitInstallerConfig();
  SwordApp.SelectRemoteSource();
  std::vector<SwordModuleInfo> mod_list = SwordApp.GetRemoteSourceModules();
  for(int n = 0; n < mod_list.size(); n++)
  {
    ModuleListCtrl->InsertItem(n, mod_list[n].Name);
    ModuleListCtrl->SetItem(n, 1, mod_list[n].Type);
    ModuleListCtrl->SetItem(n, 2, mod_list[n].Version);
  }

  // Status Bar at Bottom
  CreateStatusBar();
  std::string initial_status("Module Installer");
  SetStatusText(initial_status);
}

void InstallerFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

void InstallerFrame::InstallModule(wxCommandEvent& event)
{
  long int itemIndex = -1;
  itemIndex = ModuleListCtrl->GetNextItem(itemIndex, wxLIST_NEXT_ALL,
    wxLIST_STATE_SELECTED);
  std::vector<SwordModuleInfo> mod_list = SwordApp.GetRemoteSourceModules();
  SwordApp.InstallRemoteModule(mod_list[itemIndex].Name);
}

void InstallerFrame::DisplayModuleInfo(wxListEvent& event)
{
  long int itemIndex = -1;
  itemIndex = ModuleListCtrl->GetNextItem(itemIndex, wxLIST_NEXT_ALL,
    wxLIST_STATE_SELECTED);
  std::vector<SwordModuleInfo> mod_list = SwordApp.GetRemoteSourceModules();
  ModDescriptionTextCtrl->Clear();
  *ModDescriptionTextCtrl << wxString(mod_list[itemIndex].Description.c_str());
}
