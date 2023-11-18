///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/button.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/menu.h>
#include <wx/timer.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MyFrame_Base
///////////////////////////////////////////////////////////////////////////////
class MyFrame_Base : public wxFrame
{
	private:

	protected:
		wxButton* m_StartStopButton;
		wxStaticText* m_CompassResult_staticText;
		wxStaticText* m_CompassDetail_staticText;
		wxStaticText* m_CompassOffsets_staticText;
		wxStaticText* m_CompassMinMax_staticText;
		wxTextCtrl* m_textCtrl_For_Logging;
		wxMenuBar* m_menubar;
		wxMenu* MyFileMenu;
		wxTimer m_timer_TakeCompassReading;

		// Virtual event handlers, overide them in your derived class
		virtual void StartStopClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTimer( wxTimerEvent& event ) { event.Skip(); }


	public:

		MyFrame_Base( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 833,763 ), long style = wxCAPTION|wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );

		~MyFrame_Base();

};

