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
  MyFrame()
      : wxFrame(nullptr, wxID_ANY, "wxWidgets Example", wxDefaultPosition,
                wxSize{480, 320}) {
    // Create location text control
    auto *locationTextCtrl =
        new wxTextCtrl{this, wxID_ANY, "https://github.com/webview/webview"};

    // Create go button control
    auto *goButton = new wxButton{this, wxID_ANY, "Go"};
    goButton->Bind(
        wxEVT_BUTTON, [this, locationTextCtrl](wxCommandEvent &event) {
          m_webview->navigate(locationTextCtrl->GetValue().ToStdString());
        });

    // Create counter static text control
    auto *counterText = new wxStaticText{
        this,          wxID_ANY,
        "0",           wxDefaultPosition,
        wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE};
    auto font{counterText->GetFont()};
    font.SetPointSize(72);
    counterText->SetFont(font);

    // Create webview instance
    m_webview = std::unique_ptr<webview::webview>{
        new webview::webview{false, GetHandle()}};
#ifdef wxHAS_NATIVE_WINDOW
    m_webviewWidget = new wxNativeWindow{
        this, wxID_ANY, static_cast<wxNativeWindowHandle>(m_webview->widget())};
#else
#error wxWidgets >= 3.1 is required for wxNativeWindow.
#endif

    m_webview->bind(
        "increment",
        [this, counterText](const std::string & /*req*/) -> std::string {
          counterText->SetLabel(wxString::Format("%d", ++m_counter));
          return "";
        });

    m_webview->set_html(html);

    // Set up UI layout
    auto *topSizer = new wxBoxSizer{wxHORIZONTAL};
    topSizer->Add(locationTextCtrl, 1, wxEXPAND);
    topSizer->Add(goButton, 0, wxEXPAND);

    auto *bottomSizer = new wxBoxSizer{wxHORIZONTAL};
    bottomSizer->Add(m_webviewWidget, 1, wxEXPAND);
    bottomSizer->Add(counterText, 1, wxALIGN_CENTER);

    auto *sizer = new wxBoxSizer{wxVERTICAL};
    sizer->Add(topSizer, 0, wxEXPAND);
    sizer->Add(bottomSizer, 1, wxEXPAND);
    SetSizer(sizer);
    Layout();
  }

  MyFrame(const MyFrame&) = delete;
  MyFrame(MyFrame&&) = delete;
  MyFrame& operator=(const MyFrame&) = delete;
  MyFrame& operator=(MyFrame&&) = delete;

  virtual ~MyFrame() {
    webview::detail::m_log << "~MyFrame(): enter" << std::endl;
    // Give wxWidgets a chance to dissociate itself with the webview widget
    // before the widget is destroyed internally in the webview library.
    m_webviewWidget->Destroy();
    webview::detail::m_log << "~MyFrame(): leave" << std::endl;
  }

private:
  wxNativeWindow *m_webviewWidget{};
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
