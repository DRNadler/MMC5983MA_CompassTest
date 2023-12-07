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
	this->SetSizeHints( wxSize( 700,-1 ), wxDefaultSize );

	wxFlexGridSizer* topSizer;
	topSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	topSizer->AddGrowableRow( 1 );
	topSizer->SetFlexibleDirection( wxBOTH );
	topSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	wxFlexGridSizer* ButtonAndResult_fgSizer;
	ButtonAndResult_fgSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
	ButtonAndResult_fgSizer->AddGrowableCol( 1 );
	ButtonAndResult_fgSizer->SetFlexibleDirection( wxBOTH );
	ButtonAndResult_fgSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_StartStopButton = new wxButton( this, wxID_ANY, wxT("Start/Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonAndResult_fgSizer->Add( m_StartStopButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* ResultSummary_bSizer;
	ResultSummary_bSizer = new wxBoxSizer( wxVERTICAL );

	ResultSummary_bSizer->SetMinSize( wxSize( 700,-1 ) );
	m_CompassResult_staticText = new wxStaticText( this, wxID_ANY, wxT("Heading 42.0 degrees"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_CompassResult_staticText->Wrap( -1 );
	ResultSummary_bSizer->Add( m_CompassResult_staticText, 0, 0, 5 );

	m_CompassDetail_staticText = new wxStaticText( this, wxID_ANY, wxT("Field strength X,Y,Z, Total"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_CompassDetail_staticText->Wrap( -1 );
	ResultSummary_bSizer->Add( m_CompassDetail_staticText, 0, 0, 5 );

	m_CompassOffsets_staticText = new wxStaticText( this, wxID_ANY, wxT("RESET/SET Offset mG: Average, ie = 88.87 ( 17% of nominal 512.63mG), X=  8.48, Y=-243.23, Z= 14.89"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_CompassOffsets_staticText->Wrap( -1 );
	ResultSummary_bSizer->Add( m_CompassOffsets_staticText, 0, 0, 5 );

	m_CompassMinMax_staticText = new wxStaticText( this, wxID_ANY, wxT("Min/Max XYZ values in mG"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_CompassMinMax_staticText->Wrap( -1 );
	ResultSummary_bSizer->Add( m_CompassMinMax_staticText, 0, 0, 5 );

	m_ObservedCompassOffsets_staticText = new wxStaticText( this, wxID_ANY, wxT("Offset XYZ from Min/Max values in mG"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_ObservedCompassOffsets_staticText->Wrap( -1 );
	ResultSummary_bSizer->Add( m_ObservedCompassOffsets_staticText, 0, 0, 5 );


	ButtonAndResult_fgSizer->Add( ResultSummary_bSizer, 1, wxALL|wxEXPAND, 5 );


	topSizer->Add( ButtonAndResult_fgSizer, 1, wxEXPAND, 5 );

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
