#include "webview.h"
#include <memory>
#include <wx/nativewin.h>
#include <wx/wx.h>

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
    m_webview = std::unique_ptr<webview::webview>{
        new webview::webview{false, GetHandle()}};
#ifdef wxHAS_NATIVE_WINDOW
    auto *webviewWidget = new wxNativeWindow{
        this, wxID_ANY, static_cast<wxNativeWindowHandle>(m_webview->widget())};
#else
#error wxWidgets >= 3.1 is required for wxNativeWindow.
#endif

    auto *locationTextCtrl =
        new wxTextCtrl{this, wxID_ANY, "https://github.com/webview/webview"};
    auto *goButton = new wxButton{this, wxID_ANY, "Go"};
    goButton->Bind(
        wxEVT_BUTTON, [this, locationTextCtrl](wxCommandEvent &event) {
          m_webview->navigate(locationTextCtrl->GetValue().ToStdString());
        });

    auto *counterText = new wxStaticText{
        this,          wxID_ANY,
        "0",           wxDefaultPosition,
        wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE};
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

    m_webview->bind(
        "increment",
        [this, counterText](const std::string & /*req*/) -> std::string {
          counterText->SetLabel(wxString::Format("%d", ++m_counter));
          return "";
        });

    m_webview->set_html(html);
  }

private:
  std::unique_ptr<webview::webview> m_webview;
  int m_counter{};
};

class MyApp : public wxApp {
public:
  bool OnInit() override {
    auto *frame = new MyFrame();
    frame->Show(true);
    return true;
  }
};

wxIMPLEMENT_APP(MyApp);
