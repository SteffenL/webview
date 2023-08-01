#include "webview.h"
#include <wx/wx.h>
#include <wx/nativewin.h>
#include <memory>

constexpr const auto html =
    R"html(<button id="increment">Tap me</button>
<script>
  const [incrementElement] = document.querySelectorAll("#increment");
  document.addEventListener("DOMContentLoaded", () => {
    incrementElement.addEventListener("click", () => {
      window.increment();
    });
  });
</script>)html";

class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets Example") {
        auto* locationTextCtrl = new wxTextCtrl{this, wxID_ANY, "https://github.com/webview/webview"};
        locationTextCtrl->Bind(wxEVT_UPDATE_UI, [this] (wxUpdateUIEvent& event) {
            event.Enable(m_ready);
        });

        auto* goButton = new wxButton{this, wxID_ANY, "Go"};
        goButton->Bind(wxEVT_UPDATE_UI, [this] (wxUpdateUIEvent& event) {
            event.Enable(m_ready);
        });
        goButton->Bind(wxEVT_BUTTON, [this, locationTextCtrl] (wxCommandEvent& event) {
            m_webview->navigate(locationTextCtrl->GetValue().ToStdString());
        });

        auto hwnd = static_cast<HWND>(GetHWND());
        m_webview = std::make_unique<webview::webview>(false, &hwnd);

        auto* webviewWidget = new wxNativeWindow{this, wxID_ANY, static_cast<HWND>(m_webview->widget())};

        auto* counterText = new wxStaticText{this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE};
        auto font{counterText->GetFont()};
        font.SetPointSize(72);
        counterText->SetFont(font);

        auto *topSizer = new wxBoxSizer{wxHORIZONTAL};
        topSizer->Add(locationTextCtrl, 1, wxEXPAND);
        topSizer->Add(goButton, 0, wxEXPAND);

        auto *bottomSizer = new wxBoxSizer{wxHORIZONTAL};
        bottomSizer->Add(webviewWidget, 1, wxEXPAND);
        bottomSizer->Add(counterText, 1, wxALIGN_CENTER);

        auto *sizer = new wxBoxSizer{wxVERTICAL};
        sizer->Add(topSizer, 0, wxEXPAND);
        sizer->Add(bottomSizer, 1, wxEXPAND);
        SetSizer(sizer);
        Layout();

        m_webview->set_ready_callback([this, counterText] {
            m_webview->bind("increment", [this, counterText](const std::string & /*req*/) -> std::string {
                counterText->SetLabel(wxString::Format("%d", ++m_counter));
                return "";
            });
            m_webview->set_html(html);
            m_ready = true;
        });
    }

private:
    bool m_ready{};
    std::unique_ptr<webview::webview> m_webview;
    int m_counter{};
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
