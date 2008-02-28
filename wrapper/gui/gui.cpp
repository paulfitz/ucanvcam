// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2008 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */


#include <wx/wx.h>
#include <wx/wxprec.h>
#include <wx/dcbuffer.h>
#include <wx/image.h>

#include <yarp/os/Time.h>
#include <yarp/os/Semaphore.h>
#include <yarp/sig/Image.h>

#include "Vcam.h"
#include "Effect.h"

#include <string>

using namespace std;

using namespace yarp::os;
using namespace yarp::sig;


extern Vcam& getVcam();

class MyApp: public wxApp
{
public:
    MyApp() {
    }

    virtual bool OnInit();
};

IMPLEMENT_APP(MyApp);

static Semaphore mutex(1);


class MyView : public wxWindow {
    DECLARE_CLASS(MyView)
    DECLARE_EVENT_TABLE()
private:
    ImageOf<PixelRgb> screen;
    wxBitmap bmp;
    Vcam& vcam;
    wxTimer timer;
public:
    MyView(wxWindow* parent, wxWindowID id, 
           const wxPoint& pos = wxDefaultPosition, 
           const wxSize& size = wxDefaultSize, 
           long style = 0, const wxString& name = wxPanelNameStr) :
        wxWindow(parent,id,pos,size,style,name), vcam(getVcam()) {
        printf("Making MyView\n");
        screen.setQuantum(1);
        screen.resize(320,240);
        screen.zero();
        wxImage img(screen.width(), screen.height(), 
                    (unsigned char *)((void*)(&(screen(0,0)))), true);
        bmp = wxBitmap(img);
        SetBackgroundColour (wxColour (0,80,60));
        ClearBackground();
        timer.SetOwner(this);
        timer.Start(30,false);
    }

    virtual bool OnInit() {
        printf("Initing MyView\n");
        return true;
    }


    void OnIdle(wxIdleEvent &) {
        // do nothing
    }

    void OnEraseBackground(wxEraseEvent &e) {
        // do nothing
    }

    void OnTimer(wxTimerEvent& e) {
        Refresh(FALSE);
    }

    void OnPaint(wxPaintEvent &e) {
        double now = Time::now();
        static double prev = now;
        static int ct = 0;

        if (vcam.isImage()) {
            prev = now;
            //printf("paint %06d\n",ct);
            ct++;

            ImageOf<PixelRgb> img;
            img.setQuantum(1);
            mutex.wait();
            vcam.getImage(img);
            mutex.post();

            // paint the screen
            bmp = wxBitmap(wxImage(img.width(), img.height(),
                    (unsigned char *)((void*)(img.getRawImage())), true));

            wxBufferedPaintDC dc(this, bmp);
 
        } else {
            wxBufferedPaintDC dc(this, bmp);
            //wxPaintDC dc(this);
        }

        // limit activity - is there a better way?
        Time::delay(0.02);

        wxGetApp().ProcessIdle();
    }
};


IMPLEMENT_CLASS(MyView, wxWindow)

BEGIN_EVENT_TABLE(MyView, wxWindow)
    EVT_IDLE(MyView::OnIdle)
    EVT_ERASE_BACKGROUND(MyView::OnEraseBackground)
    EVT_PAINT(MyView::OnPaint)
    EVT_TIMER(-1,MyView::OnTimer)
END_EVENT_TABLE()




class MyFrame: public wxFrame
{
    DECLARE_CLASS(MyFrame)
    DECLARE_EVENT_TABLE()

private:
    wxBoxSizer *topsizer;
    wxTextCtrl* m_textCtrl;
    Effect effect;
 
public:

    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnOK(wxCommandEvent& event);

    void OnChoice(wxCommandEvent& e) {
        string choice = e.GetString().c_str();
        printf("got a choice (frame): %s\n", choice.c_str());
        mutex.wait();
        effect.setEffect(choice.c_str());
        if (m_textCtrl!=NULL) {
            m_textCtrl->Clear();
            m_textCtrl->AppendText(wxString(effect.getConfiguration().toString().c_str(),
                                            wxConvUTF8));
        }
        mutex.post();
    }

    virtual bool OnInit();
enum
    {
        ID_Quit = 1,
        ID_About,
    };
};


bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame( _T("ucanvcam"), wxPoint(50,50), wxSize(450,340) );
    
    frame->OnInit();
    
    frame->Show(TRUE);
    SetTopWindow(frame);
    return TRUE;
};



IMPLEMENT_CLASS(MyFrame, wxFrame)

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Quit, MyFrame::OnQuit)
    EVT_MENU(ID_About, MyFrame::OnAbout)
    EVT_BUTTON(wxID_OK, MyFrame::OnOK)
    EVT_BUTTON(ID_Quit, MyFrame::OnQuit)
    EVT_CHOICE(-1,MyFrame::OnChoice)
END_EVENT_TABLE()


bool MyFrame::OnInit() {
    Bottle lst = effect.getEffects();
    printf("Effects are %s\n", lst.toString().c_str());


    topsizer = new wxBoxSizer( wxVERTICAL );

    MyView *view = new MyView(this, -1, 
                              wxDefaultPosition, wxSize(320,240));

    int nchoices = lst.size();
    wxString *choices = new wxString[nchoices];
    if (choices==NULL) {
        printf("memory allocation failure\n");
        exit(1);
    }
    for (int i=0; i<nchoices; i++) {
        choices[i] = lst.get(i).asString().c_str();
    }
    wxChoice* effectList = 
        new wxChoice(this,wxID_ANY,
                     wxDefaultPosition,
                     wxDefaultSize,
                     nchoices, 
                     choices);
    delete[] choices;
    choices = NULL;
    if (effectList!=NULL) {
        effectList->SetStringSelection("TickerTV");
    }
    //effectList->SetEditable(false);

    m_textCtrl = new wxTextCtrl(this, -1, wxEmptyString,
                                wxDefaultPosition, wxSize(320,30));

    wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
    
    //create two buttons that are horizontally unstretchable, 
    // with an all-around border with a width of 10 and implicit top alignment
    button_sizer->Add(
                      new wxButton( this, wxID_OK, _T("&Configure") ),
                      wxSizerFlags(0).Align(wxALIGN_RIGHT).Border(wxALL, 10));       
    
    button_sizer->Add(
                      new wxButton( this, ID_Quit, _T("E&xit") ),
                      wxSizerFlags(0).Align(wxALIGN_RIGHT).Border(wxALL, 10));    

    wxSizerFlags tflags = 
        wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxLEFT|wxRIGHT|wxTOP, 10);
    wxSizerFlags flags = 
        wxSizerFlags(0).Align(wxALIGN_CENTER).Border(wxALL, 10);
    topsizer->Add(view,flags);
    topsizer->Add(effectList,flags);
    topsizer->Add(new wxStaticText(this,-1,_T("configuration")),tflags);
    topsizer->Add(m_textCtrl, flags);

    topsizer->Add(button_sizer,wxSizerFlags(0).Align(wxALIGN_RIGHT));

    SetSizer(topsizer);
    topsizer->SetSizeHints(this);

    if (m_textCtrl!=NULL) {
        m_textCtrl->Clear();
        m_textCtrl->AppendText(wxString(effect.getConfiguration().toString().c_str(),
                                        wxConvUTF8));
    }

    return true;
}

void MyFrame::OnOK(wxCommandEvent& ev) {
    std::string str;
    std::string vocab;
    if (m_textCtrl!=NULL) {
        str = m_textCtrl->GetValue().mb_str();
        printf("Configuring with %s\n", str.c_str());
        mutex.wait();
        Property p;
        p.fromString(str.c_str());
        effect.setConfiguration(p);
        m_textCtrl->Clear();
        m_textCtrl->AppendText(wxString(effect.getConfiguration().toString().c_str(),
                                        wxConvUTF8));
        mutex.post();
    }
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
    m_textCtrl = NULL;

    wxMenu *menuFile = new wxMenu;

    menuFile->Append( ID_About, _T("&About...") );
    menuFile->AppendSeparator();
    menuFile->Append( ID_Quit, _T("E&xit") );

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, _T("&File") );

    SetMenuBar( menuBar );

    CreateStatusBar();
    SetStatusText( _T("Welcome to ucanvcam!") );
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    //wxCloseEvent ev;
    //wxPostEvent(this,ev);
    Close(TRUE);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Welcome to ucanvcam! GUI does not exist yet"),
                 _T("About ucanvcam"), wxOK | wxICON_INFORMATION, this);
}

