#include <webview.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <cstdlib>
#include <iostream>

int main()
{
    if (!glfwInit())
        return EXIT_FAILURE;

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    auto *window = glfwCreateWindow(800, 600, "Hello Webview!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    auto hwnd = glfwGetWin32Window(window);
    webview::webview w(false, &hwnd);
    if (w.window() != hwnd)
        std::cout << "not the same\n"; // not emitted, they are the same
    w.navigate("https://github.com/webview/webview/issues/938");
    // w.run(); // with or without wont avoid crash

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        w.process_events();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}