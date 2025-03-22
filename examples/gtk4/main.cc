/**
 * UI Framework Integration Example for GTK4.
 *
 * This example is provided as a proof of concept. Since the underlying browser
 * engine used in the webview library is WebKitGTK, you may consider using it
 * directly instead.
 */

#include "webview/webview.h"

#include <gtk/gtk.h>

#include <memory>
#include <string>

struct app_context_t {
  std::unique_ptr<webview::webview> w;
  int counter{};
  GtkEntry *location_entry{};
};

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

static void activate(GtkApplication *app, gpointer user_data) {
  auto *app_context{static_cast<app_context_t *>(user_data)};

  // Create top-level window
  auto *window{gtk_application_window_new(app)};
  gtk_window_set_title(GTK_WINDOW(window), "GTK4 Example");
  gtk_window_set_default_size(GTK_WINDOW(window), 480, 320);

  // Create location entry
  auto *location_entry{gtk_entry_new()};
  app_context->location_entry = GTK_ENTRY(location_entry);
  gtk_editable_set_text(GTK_EDITABLE(location_entry),
                        "https://github.com/webview/webview");

  // Create counter label with custom styling
  auto *counter_label{
      gtk_label_new(std::to_string(app_context->counter).c_str())};
  const char *counter_label_css_classes[] = {"counter", nullptr};
  gtk_widget_set_css_classes(counter_label, counter_label_css_classes);
  auto *display{gdk_display_get_default()};
  auto *style_provider{gtk_css_provider_new()};
  gtk_style_context_add_provider_for_display(
      display, GTK_STYLE_PROVIDER(style_provider),
      GTK_STYLE_PROVIDER_PRIORITY_USER - 1);
  std::string css{".counter { font-size: 72pt; }"};
  gtk_css_provider_load_from_string(style_provider, css.c_str());

  // Create go button
  auto *go_button{gtk_button_new_with_label("Go")};
  g_signal_connect(G_OBJECT(go_button), "clicked",
                   G_CALLBACK(+[](GtkButton * /*self*/, gpointer user_data) {
                     auto *app_context{static_cast<app_context_t *>(user_data)};
                     auto *url{gtk_entry_buffer_get_text(gtk_entry_get_buffer(
                         GTK_ENTRY(app_context->location_entry)))};
                     app_context->w->navigate(url);
                   }),
                   app_context);

  // Create container for the webview widget
  auto *web_container{gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)};
  gtk_box_set_homogeneous(GTK_BOX(web_container), TRUE);

  // Create webview instance
  app_context->w = std::unique_ptr<webview::webview>{
      new webview::webview{false, web_container}};

  app_context->w->bind(
      "increment", [=](const std::string & /*req*/) -> std::string {
        gtk_label_set_text(GTK_LABEL(counter_label),
                           std::to_string(++app_context->counter).c_str());
        return "";
      });

  app_context->w->set_html(html);

  // Set up UI layout
  auto *top_box{gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0)};
  gtk_widget_set_hexpand(top_box, TRUE);

  gtk_box_append(GTK_BOX(top_box), location_entry);
  gtk_widget_set_hexpand(location_entry, TRUE);

  gtk_box_append(GTK_BOX(top_box), go_button);

  auto *bottom_box{gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0)};
  gtk_widget_set_vexpand(bottom_box, TRUE);
  gtk_widget_set_hexpand(bottom_box, TRUE);
  gtk_box_set_homogeneous(GTK_BOX(bottom_box), TRUE);

  gtk_box_append(GTK_BOX(bottom_box), web_container);
  gtk_widget_set_vexpand(web_container, TRUE);
  gtk_widget_set_hexpand(web_container, TRUE);

  gtk_box_append(GTK_BOX(bottom_box), counter_label);
  gtk_widget_set_vexpand(counter_label, TRUE);
  gtk_widget_set_hexpand(counter_label, TRUE);

  auto *box{gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)};
  gtk_widget_set_vexpand(box, TRUE);
  gtk_widget_set_hexpand(box, TRUE);
  gtk_box_append(GTK_BOX(box), top_box);
  gtk_box_append(GTK_BOX(box), bottom_box);

  gtk_window_set_child(GTK_WINDOW(window), box);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  app_context_t app_context;
  auto *app{gtk_application_new(
      "dev.webview.example",
      static_cast<GApplicationFlags>(
          0) /*G_APPLICATION_FLAGS_NONE or G_APPLICATION_DEFAULT_FLAGS*/)};
  g_signal_connect(app, "activate", G_CALLBACK(activate), &app_context);
  auto status{g_application_run(G_APPLICATION(app), argc, argv)};
  g_object_unref(app);
  return status;
}
