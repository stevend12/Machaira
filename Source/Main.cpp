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
    // Verse controls at top of window
    wxButton * GetTextButton;
    wxTextCtrl * VerseTextCtrl;
    wxStaticText * CurrentVerseText;
    wxButton * PreviousVerseButton;
    wxButton * NextVerseButton;
    // Scripture display
    wxComboBox * ScriptureComboBox;
    wxHtmlWindow * ScriptureHtmlWindow;
    // Commentary display
    wxComboBox * CommentaryComboBox;
    wxHtmlWindow * CommentaryHtmlWindow;
    // Hover display (Dictionary/Lexicon/Cross-Reference)
    wxHtmlWindow * HoverHtmlWindow;
  private:
    // Event Functions
    void OnExit(wxCommandEvent& event);
    void LoadText(wxCommandEvent& event);
    void AddModule(wxCommandEvent& event);
    void ChooseTranslation(wxCommandEvent& event);
    void ChooseCommentary(wxCommandEvent& event);
    void GoToPreviousVerse(wxCommandEvent& event);
    void GoToNextVerse(wxCommandEvent& event);
    void UpdateMiscDisplay(wxHtmlLinkEvent& event);
    // Utilities
    void UpdateWindows(std::string verse);
    wxDECLARE_EVENT_TABLE();
};

class InstallerFrame: public wxFrame
{
public:
  InstallerFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
  wxComboBox * SourceComboBox;
  wxButton * LoadSourceButton;
  wxButton * InstallButton;
  wxListCtrl * ModuleListCtrl;
  wxTextCtrl * ModDescriptionTextCtrl;
private:
  void OnExit(wxCommandEvent& event);
  void LoadSource(wxCommandEvent& event);
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
  ID_LoadSource = wxID_HIGHEST + 5,
  ID_Install = wxID_HIGHEST + 6
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(ID_Add, MainFrame::AddModule)
  EVT_MENU(wxID_EXIT, MainFrame::OnExit)
  EVT_BUTTON(ID_Get, MainFrame::LoadText)
  EVT_COMBOBOX(wxID_ANY, MainFrame::ChooseTranslation)
  EVT_COMBOBOX(wxID_ANY, MainFrame::ChooseCommentary)
  EVT_BUTTON(ID_PrevVerse, MainFrame::GoToPreviousVerse)
  EVT_BUTTON(ID_NextVerse, MainFrame::GoToNextVerse)
  EVT_HTML_LINK_CLICKED(wxID_ANY, MainFrame::UpdateMiscDisplay)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(InstallerFrame, wxFrame)
  EVT_MENU(wxID_EXIT, InstallerFrame::OnExit)
  EVT_BUTTON(ID_LoadSource, InstallerFrame::LoadSource)
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
  std::string boot_verse("Genesis 1:1");
  CurrentVerseText = new wxStaticText(panel, wxID_ANY, boot_verse,
    wxPoint(380, 30), wxSize(240,30), wxALIGN_CENTRE_HORIZONTAL);

  // Button Control to go to previous verse
  PreviousVerseButton = new wxButton(panel, ID_PrevVerse, _T("<-"),
    wxPoint(650, 30), wxSize(60,30), 0);

  // Button Control to go to next verse
  NextVerseButton = new wxButton(panel, ID_NextVerse, _T("->"),
    wxPoint(730, 30), wxSize(60,30), 0);

  // Combo Box to Choose Scripture Translation
  std::vector<std::string> translations = SwordApp.GetBiblicalTexts();
  wxArrayString t_choices;
  wxString t_value("");
  if(translations.size() > 0)
  {
    t_value = translations[0].c_str();
    for(int n = 0; n < translations.size(); n++)
    {
      t_choices.Add(translations[n].c_str());
    }
  }
  ScriptureComboBox = new wxComboBox(panel, wxID_ANY, t_value,
    wxPoint(50, 70), wxSize(200, 30), t_choices, wxCB_READONLY);

  // HTML Window for Scripture Display
  ScriptureHtmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxPoint(50, 120),
    wxSize(400, 180));

  // Combo Box to Choose Commentary
  std::vector<std::string> commentaries = SwordApp.GetCommentaries();
  wxArrayString c_choices;
  wxString c_value("");
  if(commentaries.size() > 0)
  {
    c_value = commentaries[0].c_str();
    for(int n = 0; n < commentaries.size(); n++)
    {
      c_choices.Add(commentaries[n].c_str());
    }
  }
  CommentaryComboBox = new wxComboBox(panel, wxID_ANY, c_value,
    wxPoint(500, 70), wxSize(200, 30), c_choices, wxCB_READONLY);

  // HTML Window for Commentary Display
  CommentaryHtmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxPoint(500, 120),
    wxSize(400, 400));

  // HTML Window for Hover Display (Dictionary/Lexicon/Cross-Reference)
  HoverHtmlWindow = new wxHtmlWindow(panel, wxID_ANY, wxPoint(50, 320),
    wxSize(400, 180));

  // Status Bar at Bottom
  CreateStatusBar();
  std::string initial_status("Welcome to Machaira!");
  initial_status += " Using "+SwordApp.GetSwordVersion();
  SetStatusText(initial_status);

  // Initialize app by showing Genesis 1:1
  UpdateWindows(boot_verse);
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

void MainFrame::ChooseTranslation(wxCommandEvent& event)
{
  UpdateWindows(std::string(CurrentVerseText->GetLabel()));
}

void MainFrame::ChooseCommentary(wxCommandEvent& event)
{
  UpdateWindows(std::string(CurrentVerseText->GetLabel()));
}

void MainFrame::GoToPreviousVerse(wxCommandEvent& event)
{
  SwordApp.SetVerseRef(std::string(ScriptureComboBox->GetValue()),
    std::string(CurrentVerseText->GetLabel()));
  SwordApp.IncrementVerse(std::string(ScriptureComboBox->GetValue()), -1);
  UpdateWindows(SwordApp.GetVerseRef(std::string(ScriptureComboBox->GetValue())));
}

void MainFrame::GoToNextVerse(wxCommandEvent& event)
{
  SwordApp.SetVerseRef(std::string(ScriptureComboBox->GetValue()),
    std::string(CurrentVerseText->GetLabel()));
  SwordApp.IncrementVerse(std::string(ScriptureComboBox->GetValue()), 1);
  UpdateWindows(SwordApp.GetVerseRef(std::string(ScriptureComboBox->GetValue())));
}

void MainFrame::UpdateWindows(std::string verse)
{
  ScriptureHtmlWindow->SetPage(
    SwordApp.GetText(verse, std::string(ScriptureComboBox->GetValue()))
  );
  CommentaryHtmlWindow->SetPage(
    SwordApp.GetText(verse, std::string(CommentaryComboBox->GetValue()))
  );
  CurrentVerseText->SetLabel(SwordApp.GetVerseRef(std::string(ScriptureComboBox->GetValue())));
}

void MainFrame::UpdateMiscDisplay(wxHtmlLinkEvent& event)
{
  wxString ref(event.GetLinkInfo().GetHref());
  int f = ref.Find('_');
  int l = ref.Find('_', true);
  wxString l_action(ref.SubString(0, f-1));
  wxString l_type(ref.SubString(f+1, l-1));
  wxString l_val(ref.Mid(l+1));

  if(l_action == "showRef")
  {
    HoverHtmlWindow->SetPage(SwordApp.GetText(std::string(l_val),
      std::string(ScriptureComboBox->GetValue()))
    );
  }
  if(l_action == "showStrongs")
  {
    if(l_type == "Hebrew")
    {
      HoverHtmlWindow->SetPage(SwordApp.GetText(std::string(l_val),
        "StrongsHebrew")
      );
    }
    if(l_type == "Greek")
    {
      HoverHtmlWindow->SetPage(SwordApp.GetText(std::string(l_val),
        "StrongsGreek")
      );
    }
  }
  SetStatusText(event.GetLinkInfo().GetHref());
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

  // Combo Box to Choose Scripture Translation
  std::vector<std::string> sources = SwordApp.GetRemoteSources();
  wxArrayString s_choices;
  wxString s_value("");
  if(sources.size() > 0)
  {
    s_value = sources[0].c_str();
    for(int n = 0; n < sources.size(); n++)
    {
      s_choices.Add(sources[n].c_str());
    }
  }
  SourceComboBox = new wxComboBox(panel, wxID_ANY, s_value,
    wxPoint(50, 30), wxSize(200, 30), s_choices, wxCB_READONLY);

  // Button Control to Install Modules
  LoadSourceButton = new wxButton(panel, ID_LoadSource, _T("Load Source"),
    wxPoint(50, 70), wxSize(100, 30), 0);

  // Button Control to Install Modules
  InstallButton = new wxButton(panel, ID_Install, _T("Install Module"),
    wxPoint(180, 70), wxSize(100, 30), 0);

  // List Control for Modules
  ModuleListCtrl = new wxListCtrl(panel, wxID_ANY, wxPoint(40, 120),
    wxSize(420, 400), wxLC_REPORT | wxLC_HRULES | wxLC_SINGLE_SEL);
  ModuleListCtrl->InsertColumn(0, "Module Name", wxLIST_FORMAT_LEFT, 120);
  ModuleListCtrl->InsertColumn(1, "Type", wxLIST_FORMAT_LEFT, 200);
  ModuleListCtrl->InsertColumn(2, "Version", wxLIST_FORMAT_LEFT, 90);

  // Text Control for Description
  ModDescriptionTextCtrl = new wxTextCtrl(panel, wxID_ANY, "Module Description",
    wxPoint(480, 120), wxSize(300, 200), wxTE_READONLY | wxTE_MULTILINE);

  if(!SwordApp.HasInstallerConfig()) SwordApp.InitInstallerConfig();

  // Status Bar at Bottom
  CreateStatusBar();
  std::string initial_status("Module Installer");
  SetStatusText(initial_status);
}

void InstallerFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

void InstallerFrame::LoadSource(wxCommandEvent& event)
{
  SwordApp.SelectRemoteSource();
  std::vector<SwordModuleInfo> mod_list = SwordApp.GetRemoteSourceModules();
  for(int n = 0; n < mod_list.size(); n++)
  {
    ModuleListCtrl->InsertItem(n, mod_list[n].Name);
    ModuleListCtrl->SetItem(n, 1, mod_list[n].Type);
    ModuleListCtrl->SetItem(n, 2, mod_list[n].Version);
  }
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
