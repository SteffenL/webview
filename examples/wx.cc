#include "webview.h"
#include <wx/wx.h>
#include <wx/nativewin.h>
#include <memory>

class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets Example") {
        auto* top = new wxPanel{this};
        auto *topSizer = new wxBoxSizer{wxHORIZONTAL};

        auto* locationTextCtrl = new wxTextCtrl{top, wxID_ANY, "https://github.com/webview/webview"};
        locationTextCtrl->Bind(wxEVT_UPDATE_UI, [this] (wxUpdateUIEvent& event) {
            event.Enable(m_ready);
        });

        auto* goButton = new wxButton{top, wxID_ANY, "Go"};
        goButton->Bind(wxEVT_UPDATE_UI, [this] (wxUpdateUIEvent& event) {
            event.Enable(m_ready);
        });
        goButton->Bind(wxEVT_BUTTON, [this, locationTextCtrl] (wxCommandEvent& event) {
            m_webview->navigate(locationTextCtrl->GetValue().ToStdString());
        });

        topSizer->Add(locationTextCtrl, 1, wxEXPAND);
        topSizer->Add(goButton, 0, wxEXPAND);
        top->SetSizer(topSizer);

        auto hwnd = static_cast<HWND>(GetHWND());
        m_webview = std::make_unique<webview::webview>(false, &hwnd);
        auto* bottom = new wxNativeWindow{this, wxID_ANY, reinterpret_cast<HWND>(m_webview->widget())};

        auto *sizer = new wxBoxSizer{wxVERTICAL};
        sizer->Add(top, 0, wxEXPAND);
        sizer->Add(bottom, 1, wxEXPAND);
        SetSizer(sizer);
        Layout();

        m_webview->set_ready_callback([this] {
            m_webview->set_html("Hello, webview!");
            m_ready = true;
        });
    }

private:
    bool m_ready{};
    std::unique_ptr<webview::webview> m_webview;
};
 
class MyApp : public wxApp
{
public:
    bool OnInit() override {
        auto *frame = new MyFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
