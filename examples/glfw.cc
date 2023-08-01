#include <webview.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

int main() {
    if (!glfwInit()) {
        return -1;
    }

    auto *window{glfwCreateWindow(640, 480, "GLFW Example", nullptr, nullptr)};
    if (!window) {
        glfwTerminate();
        return -1;
    }

    auto hwnd = glfwGetWin32Window(window);
    webview::webview w(false, &hwnd);

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
