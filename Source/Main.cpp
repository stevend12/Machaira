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
#include <wx/combobox.h>
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
    wxStaticText * CurrentVerseText;
    wxButton * PreviousVerseButton;
    wxButton * NextVerseButton;
    wxHtmlWindow * ScriptureHtmlWindow;
    wxHtmlWindow * CommentaryHtmlWindow;
    wxComboBox * CommentaryComboBox;
  private:
    // Event Functions
    void OnExit(wxCommandEvent& event);
    void LoadText(wxCommandEvent& event);
    void AddModule(wxCommandEvent& event);
    void ChooseCommentary(wxCommandEvent& event);
    void GoToPreviousVerse(wxCommandEvent& event);
    void GoToNextVerse(wxCommandEvent& event);
    // Utilities
    void UpdateWindows(std::string verse);
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
  ID_PrevVerse = wxID_HIGHEST + 3,
  ID_NextVerse = wxID_HIGHEST + 4,
  ID_Install = wxID_HIGHEST + 5
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(ID_Add, MainFrame::AddModule)
  EVT_MENU(wxID_EXIT, MainFrame::OnExit)
  EVT_BUTTON(ID_Get, MainFrame::LoadText)
  EVT_COMBOBOX(wxID_ANY, MainFrame::ChooseCommentary)
  EVT_BUTTON(ID_PrevVerse, MainFrame::GoToPreviousVerse)
  EVT_BUTTON(ID_NextVerse, MainFrame::GoToNextVerse)
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
    wxPoint(30, 30), wxSize(100,30), 0);

  // Text Control for Verse Entry
  VerseTextCtrl = new wxTextCtrl(panel, wxID_ANY, "Enter Verse(s) Here",
    wxPoint(150, 30), wxSize(200,30));

  // Static Text to show Current Verse
  CurrentVerseText = new wxStaticText(panel, wxID_ANY, "No Verse Selected",
    wxPoint(380, 30), wxSize(240,30), wxALIGN_CENTRE_HORIZONTAL);

  // Button Control to go to previous verse
  PreviousVerseButton = new wxButton(panel, ID_PrevVerse, _T("<-"),
    wxPoint(650, 30), wxSize(60,30), 0);

  // Button Control to go to next verse
  NextVerseButton = new wxButton(panel, ID_NextVerse, _T("->"),
    wxPoint(730, 30), wxSize(60,30), 0);

  // Text Control for Scripture Reference
  ScriptureHtmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxPoint(50, 120),
    wxSize(400, 200));

  // Choose Commentary to Display
  std::vector<std::string> commentaries = SwordApp.GetCommentaries();
  wxArrayString choices;
  wxString value("");
  if(commentaries.size() > 0)
  {
    value = commentaries[0].c_str();
    for(int n = 0; n < commentaries.size(); n++)
    {
      choices.Add(commentaries[n].c_str());
    }
  }
  CommentaryComboBox = new wxComboBox(panel, wxID_ANY, value,
    wxPoint(500, 70), wxSize(200, 30), choices, wxCB_READONLY);

  // Text Control for Commentary
  CommentaryHtmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxPoint(500, 120),
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
  UpdateWindows(std::string(VerseTextCtrl->GetLineText(0)));
}

void MainFrame::AddModule(wxCommandEvent& event)
{
  InstallerFrame * installer_frame = new InstallerFrame("Module Installer",
    wxPoint(200, 200), wxSize(800, 600));
  installer_frame->Show(true);
	wxGetApp().SetTopWindow(installer_frame);
}

void MainFrame::ChooseCommentary(wxCommandEvent& event)
{
  UpdateWindows(SwordApp.GetVerseRef(SwordApp.GetBiblicalText(0)));
}

void MainFrame::GoToPreviousVerse(wxCommandEvent& event)
{
  SwordApp.UpdateVerse(SwordApp.GetBiblicalText(0), -1);
  UpdateWindows(SwordApp.GetVerseRef(SwordApp.GetBiblicalText(0)));
}

void MainFrame::GoToNextVerse(wxCommandEvent& event)
{
  SwordApp.UpdateVerse(SwordApp.GetBiblicalText(0), 1);
  UpdateWindows(SwordApp.GetVerseRef(SwordApp.GetBiblicalText(0)));
}

void MainFrame::UpdateWindows(std::string verse)
{
  ScriptureHtmlWindow->SetPage(
    SwordApp.GetText(verse, SwordApp.GetBiblicalText(0))
  );
  CommentaryHtmlWindow->SetPage(
    SwordApp.GetText(verse, std::string(CommentaryComboBox->GetValue()))
  );
  CurrentVerseText->SetLabel(SwordApp.GetVerseRef(SwordApp.GetBiblicalText(0)));
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
  long int item_index = -1;
  item_index = ModuleListCtrl->GetNextItem(item_index, wxLIST_NEXT_ALL,
    wxLIST_STATE_SELECTED);
  std::vector<SwordModuleInfo> mod_list = SwordApp.GetRemoteSourceModules();
  SwordApp.InstallRemoteModule(mod_list[item_index].Name);
}

void InstallerFrame::DisplayModuleInfo(wxListEvent& event)
{
  long int item_index = -1;
  item_index = ModuleListCtrl->GetNextItem(item_index, wxLIST_NEXT_ALL,
    wxLIST_STATE_SELECTED);
  std::vector<SwordModuleInfo> mod_list = SwordApp.GetRemoteSourceModules();
  ModDescriptionTextCtrl->Clear();
  *ModDescriptionTextCtrl << wxString(mod_list[item_index].Description.c_str());
}
