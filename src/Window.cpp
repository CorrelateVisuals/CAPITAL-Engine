#include <iostream>

#include "CAPITAL_Engine.h"
#include "Control.h"
#include "Window.h"

Window::Window() : window{nullptr}, framebufferResized{false} {
  _log.console("{ [X] }", "constructing Window");
  initWindow();
}

Window::~Window() {
  _log.console("{ [X] }", "destructing Window");
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(displayConfig.width, displayConfig.height,
                            displayConfig.windowTitle, nullptr, nullptr);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, windowResize);
  _log.console("{ [*] }", "Window initialized with", displayConfig.width, "*",
               displayConfig.height);
}

void Window::windowResize(GLFWwindow* window, int width, int height) {
  auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
  displayConfig.width = width;
  displayConfig.height = height;
  _log.console("{ [*] }", "Window resized to", width, "*", height);
}

void Window::mouseClick(GLFWwindow* window, int button) {
  static int oldState = GLFW_RELEASE;
  int newState = glfwGetMouseButton(window, button);

  if (oldState == GLFW_PRESS && newState == GLFW_RELEASE) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    xpos /= displayConfig.width;
    ypos /= displayConfig.height;
    _log.console("{ in }", "Mouse Click at", xpos, ":", ypos);
  }
  oldState = newState;
}
