///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "CompassLayout_Base_Classes.h"

///////////////////////////////////////////////////////////////////////////

MyFrame_Base::MyFrame_Base( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* topSizer;
	topSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* ButtonAndResultSizer;
	ButtonAndResultSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxHORIZONTAL );

	m_StartStopButton = new wxButton( ButtonAndResultSizer->GetStaticBox(), wxID_ANY, wxT("Start/Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonAndResultSizer->Add( m_StartStopButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	m_CompassResult_staticText = new wxStaticText( ButtonAndResultSizer->GetStaticBox(), wxID_ANY, wxT("Heading 42.0 degrees"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_CompassResult_staticText->Wrap( -1 );
	bSizer2->Add( m_CompassResult_staticText, 0, 0, 5 );

	m_CompassDetail_staticText = new wxStaticText( ButtonAndResultSizer->GetStaticBox(), wxID_ANY, wxT("Field strength X,Y,Z, Total"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_CompassDetail_staticText->Wrap( -1 );
	bSizer2->Add( m_CompassDetail_staticText, 0, 0, 5 );

	m_CompassOffsets_staticText = new wxStaticText( ButtonAndResultSizer->GetStaticBox(), wxID_ANY, wxT("Offset values in mG"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_CompassOffsets_staticText->Wrap( -1 );
	bSizer2->Add( m_CompassOffsets_staticText, 0, 0, 5 );


	ButtonAndResultSizer->Add( bSizer2, 1, wxALL|wxEXPAND, 5 );


	topSizer->Add( ButtonAndResultSizer, 1, wxEXPAND, 5 );

	m_textCtrl_For_Logging = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	m_textCtrl_For_Logging->SetMinSize( wxSize( -1,300 ) );

	topSizer->Add( m_textCtrl_For_Logging, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( topSizer );
	this->Layout();
	m_menubar = new wxMenuBar( 0 );
	MyFileMenu = new wxMenu();
	wxMenuItem* m_menuItem_About;
	m_menuItem_About = new wxMenuItem( MyFileMenu, wxID_ANY, wxString( wxT("&About") ) , wxEmptyString, wxITEM_NORMAL );
	MyFileMenu->Append( m_menuItem_About );

	wxMenuItem* m_menuItem_Exit;
	m_menuItem_Exit = new wxMenuItem( MyFileMenu, wxID_ANY, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	MyFileMenu->Append( m_menuItem_Exit );

	m_menubar->Append( MyFileMenu, wxT("File") );

	this->SetMenuBar( m_menubar );

	m_timer_TakeCompassReading.SetOwner( this, wxID_ANY );

	this->Centre( wxBOTH );

	// Connect Events
	m_StartStopButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyFrame_Base::StartStopClicked ), NULL, this );
	MyFileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MyFrame_Base::OnAbout ), this, m_menuItem_About->GetId());
	MyFileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MyFrame_Base::OnQuit ), this, m_menuItem_Exit->GetId());
	this->Connect( wxID_ANY, wxEVT_TIMER, wxTimerEventHandler( MyFrame_Base::OnTimer ) );
}

MyFrame_Base::~MyFrame_Base()
{
	// Disconnect Events
	m_StartStopButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyFrame_Base::StartStopClicked ), NULL, this );
	this->Disconnect( wxID_ANY, wxEVT_TIMER, wxTimerEventHandler( MyFrame_Base::OnTimer ) );

}
