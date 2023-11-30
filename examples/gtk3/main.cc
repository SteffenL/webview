/**
 * UI Framework Integration Example for GTK3.
 *
 * This example is provided as a proof of concept. Since the underlying browser
 * engine used in the webview library is WebKitGTK, you may consider using it
 * directly instead.
 */

#include "webview.h"
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
  gtk_window_set_title(GTK_WINDOW(window), "GTK3 Example");
  gtk_window_set_default_size(GTK_WINDOW(window), 480, 320);

  // Create location entry
  auto *location_entry{gtk_entry_new()};
  app_context->location_entry = GTK_ENTRY(location_entry);
  gtk_entry_set_text(GTK_ENTRY(location_entry),
                     "https://github.com/webview/webview");

  // Create counter label with custom styling
  auto *counter_label{
      gtk_label_new(std::to_string(app_context->counter).c_str())};
  auto *style_provider = gtk_css_provider_new();
  auto *style_context{gtk_widget_get_style_context(counter_label)};
  gtk_style_context_add_provider(style_context,
                                 GTK_STYLE_PROVIDER(style_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER - 1);
  std::string css{"label { font-size: 72pt; }"};
  gtk_css_provider_load_from_data(style_provider, css.c_str(), css.size(),
                                  nullptr);

  // Create go button
  auto *go_button{gtk_button_new_with_label("Go")};
  g_signal_connect(G_OBJECT(go_button), "clicked",
                   G_CALLBACK(+[](GtkButton *self, gpointer user_data) {
                     auto *app_context{static_cast<app_context_t *>(user_data)};
                     auto *url{gtk_entry_buffer_get_text(gtk_entry_get_buffer(
                         GTK_ENTRY(app_context->location_entry)))};
                     app_context->w->navigate(url);
                   }),
                   app_context);

  // Create webview instance
  app_context->w = std::unique_ptr<webview::webview>{
      new webview::webview{false, GTK_WINDOW(window)}};

  app_context->w->bind(
      "increment", [=](const std::string & /*req*/) -> std::string {
        gtk_label_set_text(GTK_LABEL(counter_label),
                           std::to_string(++app_context->counter).c_str());
        return "";
      });

  app_context->w->set_html(html);

  // Set up UI layout
  auto *top_box{gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0)};
  gtk_box_pack_start(GTK_BOX(top_box), GTK_WIDGET(location_entry), TRUE, TRUE,
                     0);
  gtk_box_pack_start(GTK_BOX(top_box), GTK_WIDGET(go_button), FALSE, TRUE, 0);

  auto *bottom_box{gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0)};
  gtk_box_set_homogeneous(GTK_BOX(bottom_box), TRUE);
  gtk_box_pack_start(GTK_BOX(bottom_box), GTK_WIDGET(app_context->w->widget()),
                     TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(bottom_box), counter_label, TRUE, TRUE, 0);

  auto *box{gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)};
  gtk_box_pack_start(GTK_BOX(box), top_box, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), bottom_box, TRUE, TRUE, 0);

  gtk_container_add(GTK_CONTAINER(window), box);
  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  app_context_t app_context;
  auto *app{
      gtk_application_new("dev.webview.example", G_APPLICATION_FLAGS_NONE)};
  g_signal_connect(app, "activate", G_CALLBACK(activate), &app_context);
  auto status{g_application_run(G_APPLICATION(app), argc, argv)};
  g_object_unref(app);
  return status;
}
