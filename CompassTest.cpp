/////////////////////////////////////////////////////////////////////////////
// Name:        CompassTest.cpp
// Purpose:     Windows test code for Qwiic-attached MMC5983MA compass chip
// Author:      Dave Nadler
// Modified by:
// Created:     17-November-2023
// Copyright:   (c) 2023 Dave Nadler
// License:     MIT license
/////////////////////////////////////////////////////////////////////////////

/*
MIT License

Copyright (c) 2023-2025 Dave Nadler

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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#ifdef __BORLANDC__
  #pragma hdrstop
#endif
#ifndef WX_PRECOMP
  #include "wx/wx.h"
#endif

#include "wx/timer.h"           // for wxStopWatch
#include "wx/settings.h"
#include "wx/sizer.h"
#include "wx/sysopt.h"
#include "wx/numdlg.h"
#include "wx/selstore.h"
#include "wx/version.h"

// USB-to-I2C adapters to drive Qwiic sensors (Microchip MCP2221, FT232H)
#include "MMC5983MA_IO_WindowsQwiic_MCP2221.hpp"
#include "MMC5983MA_IO_WindowsQwiic_FT232H.hpp"
// #define USE_MCP2221

#include "MMC5983MA.hpp"

#include "CompassTest.h"


// ----------------------------------------------------------------------------
// Globals for compass including IO support via MCP2221
// ----------------------------------------------------------------------------

#ifdef USE_MCP2221
  class MMC5983MA_C_local: public MMC5983MA_C<MMC5983MA_IO_WindowsQwiic_MCP2221_C> {
    public:
      MCP2221& GetMCP2221() { return dev.mcp2221; };
  } compass;
#else
  class MMC5983MA_C_local: public MMC5983MA_C<MMC5983MA_IO_WindowsQwiic_FT232H_C> {
  } compass;
#endif


// ----------------------------------------------------------------------------
// MyApp
// ----------------------------------------------------------------------------
wxIMPLEMENT_APP(MyApp);
MyApp* pMyApp;

// `Main program' equivalent, creating windows and returning main app frame
bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    pMyApp = this; // because application data must be accessible elsewhere and its a singleton

    // Create the main frame window
    MyFrame *frame = new MyFrame("Dave's Compass Test");

    // Show the frame
    frame->Show(true);

    return true;
}


// ----------------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------------

// My frame constructor
MyFrame::MyFrame(const wxString& title)
       : MyFrame_Base(NULL, wxID_ANY, title)
{
    // Give it an icon
    SetIcon(wxICON(sample));
    // Route logging output to multiline text control set up for viewing log
    m_logOld = wxLog::SetActiveTarget(new wxLogTextCtrl(m_textCtrl_For_Logging));
}

MyFrame::~MyFrame()
{
    m_timer_TakeCompassReading.Stop();
    delete wxLog::SetActiveTarget(m_logOld);
 }

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageDialog dialog(this, "Dave's Qwiic-attached MMC5983MA compass test\nDave Nadler (c) 2023-2025\n" 
        "\nBuilt with " + wxGetLibraryVersionInfo().GetDescription(),
        "About Compass Test");
    dialog.ShowModal();
}

void MyFrame::StartStopClicked(wxCommandEvent&) {
    if (!compass.initialized) {
        // Init:
        // - locate and open an MCP2221 USB-to-Qwiic device
        // - resets, verifies compass product ID over I2C bus, throws on failure
        try {
            compass.Init();
        } catch(const std::exception& e) {
            wxMessageDialog dialog(this, e.what(), "Error connecting to compass");
            dialog.ShowModal();
            return;
        }
        #ifdef USE_MCP2221
          wxLogMessage("%d MCP2221s found connected to this PC; first one opened OK.", compass.GetMCP2221().connectedDevices);
        #endif
        assert(compass.initialized);
        wxLogMessage("Compass initialized AOK");
        wxLogMessage("=======================================");
    };
    if (m_timer_TakeCompassReading.IsRunning()) {
        m_timer_TakeCompassReading.Stop();
    } else {
        m_timer_TakeCompassReading.Start(1000);
    }
}

void MyFrame::Make_A_Measurement() {
    const double nominalFieldmG = 512.63; // ~ strength of Earth's field at Dave's desk; see below.
    wxLogMessage("Measure_XYZ_Field_WithResetSet...");
    compass.Measure_XYZ_Field_WithResetSet();
    wxLogMessage("-----------");
    wxLogMessage("Compass: SET/RESET offsets (zero-point, nominal 0x20000): x%05lx, x%05lx, x%05lx", compass.offset[0], compass.offset[1], compass.offset[2]);
    wxLogMessage("Compass: sensors (adjusted for offset): x%05lx, x%05lx, x%05lx", compass.field[0], compass.field[1], compass.field[2]);
    //
    // WAG as to sign and X vs. Y orientation
    double heading = 180.0 - atan2(-compass.field[0], -compass.field[1]) * 180 / M_PI;
    wxString report_Heading;
    report_Heading.Printf("Compass: %6.2f", heading);
    m_CompassResult_staticText->SetLabelText(report_Heading);
    wxLogMessage(report_Heading);
    //
    // Detail into m_CompassOffsets_staticText
    double offset_mG[3]; double averageOffset_mG = 0.0;
    for (int i = 0; i < 3; i++) {
        offset_mG[i] = (double)(((int)compass.offset[i])-0x20000) / 16.384;
        averageOffset_mG += abs(offset_mG[i])/3;
    };
    wxString report_offset;
    report_offset.Printf("RESET/SET Offset mG: Average=%6.2f (%3.0f%% of nominal %6.2fmG), X=%6.2f, Y=%6.2f, Z=%6.2f",
        averageOffset_mG, 100.0 * averageOffset_mG / nominalFieldmG, nominalFieldmG,
        offset_mG[0], offset_mG[1], offset_mG[2] );
    m_CompassOffsets_staticText->SetLabelText(report_offset);
    report_offset.Replace("%", "%%"); // so wxLog doesn't expand percentage as a printf-style format specifier
    wxLogMessage(report_offset);
    //
    // Earth's magnetic field intensity is roughly between 25,000 - 65,000 nT (.25 - .65 gauss). That's 250-650 mG.
    /*
     * From WMM, for 42N,71W,0MSL (100nT = 1mG)
     *      Total   Horizontal       North       East     Vertical  Declination  Inclination
     *  51,263 nT    20,728 nT   20,104 nT   -5047 nT    46,885 nT      -14.09°       66.15°
     */
    double sensors_mG[3]; double totalField_mG = 0.0;
    for (int i = 0; i < 3; i++) {
        sensors_mG[i] = (double)compass.field[i] /
            ((double)MMC5983MA_C_local::CountsPerGauss/1000.0); // ie 16.384
        totalField_mG += pow(sensors_mG[i],2);
    };
    totalField_mG = sqrt(totalField_mG);
    wxString report_FieldStrength;
    report_FieldStrength.Printf("Field mG: Total=%6.2f (%3.0f%% of nominal %6.2fmG), X=%6.2f, Y=%6.2f, Z=%6.2f",
        totalField_mG, 100*totalField_mG/nominalFieldmG, nominalFieldmG,
        sensors_mG[0], sensors_mG[1], sensors_mG[2] );
    m_CompassDetail_staticText->SetLabel(report_FieldStrength);
    report_FieldStrength.Replace("%", "%%"); // so wxLog doesn't expand percentage as a printf-style format specifier
    wxLogMessage(report_FieldStrength);
    //
    static double minReadings_mG[3] = { 0.0, 0.0, 0.0 }, maxReadings_mG[3] = {0.0, 0.0, 0.0}, avgReadings_mG[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < 3; i++) {
        if (sensors_mG[i] < minReadings_mG[i]) minReadings_mG[i] = sensors_mG[i];
        if (sensors_mG[i] > maxReadings_mG[i]) maxReadings_mG[i] = sensors_mG[i];
        avgReadings_mG[i] = (minReadings_mG[i] + maxReadings_mG[i]) / 2;
    };
    wxString report_MinMax;
    report_MinMax.Printf("XYZ mG [Min,Max]: [%6.2f,%6.2f] [%6.2f,%6.2f] [%6.2f,%6.2f]",
        minReadings_mG[0], maxReadings_mG[0], minReadings_mG[1], maxReadings_mG[1], minReadings_mG[2], maxReadings_mG[2]);
    m_CompassMinMax_staticText->SetLabel(report_MinMax);
    wxLogMessage(report_MinMax);
    //
    wxString report_AvgMinMax;
    report_AvgMinMax.Printf("XYZ averageMinMax mG (observed additional offset): %6.2f, %6.2f, %6.2f", avgReadings_mG[0], avgReadings_mG[1], avgReadings_mG[2]);
    m_ObservedCompassOffsets_staticText->SetLabel(report_AvgMinMax);
    wxLogMessage(report_AvgMinMax);
    //
    // compass.Measure_XYZ_Field_WithAutoSR();



    wxLogMessage("=======================================");
    // No effect: this->Layout();
    // No effect: SendSizeEvent();
}

// ----------------------------------------------------------------------------
// Diagnostic output for MMC5983MA_C_local (MMC5983MA_C register IO trace)
// ----------------------------------------------------------------------------
// ToDo: Clean this up to eliminate cut-and-past; should be static methods?
int MMC5983MA_IO_WindowsQwiic_MCP2221_C::DiagPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
#if 0
    // PrintfV returns a too-long length wxString with junk after correct string except missing trailing newline.
    // Confused about character encoding in use; deranged by newline character in format string?
    wxString logText;
    int r = logText.PrintfV(format, args);
    logText.Replace("\n", ""); // avoid extra blank lines in log window
    wxLogMessage(logText);
#else
    char tmp[200];
    int r = vsnprintf(tmp, sizeof(tmp), format, args);
    wxString tmp2(tmp);
    tmp2.Replace("\n", ""); // avoid extra blank lines in log window
    wxLogMessage(tmp2);
#endif
    va_end(args);
    return r;
};
int MMC5983MA_IO_WindowsQwiic_FT232H_C::DiagPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
#if 0
    // PrintfV returns a too-long length wxString with junk after correct string except missing trailing newline.
    // Confused about character encoding in use; deranged by newline character in format string?
    wxString logText;
    int r = logText.PrintfV(format, args);
    logText.Replace("\n", ""); // avoid extra blank lines in log window
    wxLogMessage(logText);
#else
    char tmp[200];
    int r = vsnprintf(tmp, sizeof(tmp), format, args);
    wxString tmp2(tmp);
    tmp2.Replace("\n", ""); // avoid extra blank lines in log window
    wxLogMessage(tmp2);
#endif
    va_end(args);
    return r;
};

