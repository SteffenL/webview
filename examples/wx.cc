#include "webview.h"
#include <wx/wx.h>
#include <wx/nativewin.h>
#include <memory>


/*class WebViewWidget : public wxNativeWindow {
public:
    explicit WebViewWidget(wxWindow* parent) : wxNativeWindow() {
        // When creating the native window, we must specify the valid parent
        // and while we don't have to specify any position if it's going to be
        // laid out by sizers, we do need the size.
        const wxSize size = FromDIP(wxSize(140, 30));

        HWND hwnd = ::CreateWindowW(
                        L"BUTTON",
                        L"Press me to do it",
                        WS_CHILD | WS_VISIBLE | BS_SPLITBUTTON,
                        0, 0, size.x, size.y,
                        (HWND)parent->GetHWND(), 0, nullptr, nullptr);
        if ( !hwnd )
        {
            wxLogError("Creating split button failed.");
            return;
        }

        (void)Create(parent, wxID_ANY, hwnd);
    }

    virtual ~WebViewWidget() {
        Disown();
    }

protected:
    virtual bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override {
        
        return true;
    }
};*/

class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "Hello World") {
        auto* left = new wxPanel{this};
        auto* right = new wxPanel{this};
        auto hwnd = static_cast<HWND>(left->GetHWND());
        m_webview = std::make_unique<webview::webview>(false, &hwnd);
        m_webview->set_ready_callback([this] {
            m_webview->navigate("https://github.com/webview/webview");
        });
        auto *sizer = new wxBoxSizer{wxHORIZONTAL};
        sizer->Add(left, 1, wxEXPAND);
        sizer->Add(right, 1, wxEXPAND);
        SetSizer(sizer);
        Layout();
    }

private:
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
