// CompassTest.h - wxWidgets application and frame definitions.

/*
MIT License

Copyright (c) 2023 Dave Nadler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// not all ports have support for EVT_CONTEXT_MENU yet, don't define
// USE_CONTEXT_MENU for those which don't
#if defined(__WXMOTIF__) || defined(__WXX11__)
    #define USE_CONTEXT_MENU 0
#else
    #define USE_CONTEXT_MENU 1
#endif

#include "LayoutGeneratedFiles/CompassLayout_Base_Classes.h"
#include "MMC5983MA.hpp"

// Define my application type
class MyApp: public wxApp
{
public:
    MyApp() { }
    virtual bool OnInit() wxOVERRIDE;

private:
    wxDECLARE_NO_COPY_CLASS(MyApp);
};
extern MyApp* pMyApp; // global singleton application pointer

// Define my frame type based on the base class generated using wxFormBuilder layout
class MyFrame: public MyFrame_Base
{
public:
    MyFrame(const wxString& title);
    virtual ~MyFrame();

protected:
    virtual void OnQuit(wxCommandEvent& event) override;
    virtual void OnAbout(wxCommandEvent& event) override;

    // Start, stop, and execute timer callback
    virtual void StartStopClicked(wxCommandEvent& event) override;
    virtual void OnTimer(wxTimerEvent& WXUNUSED(event)) override { Make_A_Measurement(); };

private:
    wxLog *m_logOld = 0;
    void Make_A_Measurement();

    wxDECLARE_NO_COPY_CLASS(MyFrame);
};
