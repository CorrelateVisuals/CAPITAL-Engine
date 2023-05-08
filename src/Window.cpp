#include <iostream>

#include "CAPITAL_Engine.h"
#include "Control.h"
#include "Window.h"

Window::Window() : window{nullptr}, framebufferResized{false} {
  _log.console("{ [X] }", "constructing Window");
  initWindow();
}

Window::~Window() {
  _log.console("{ [-] }", "destructing Window");
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

  // declare and initialize the timer variable outside the function
  static double timer = 0.0;
  static double pressTime = 0.0;

  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  xpos /= displayConfig.width;
  ypos /= displayConfig.height;

  if (oldState == GLFW_PRESS && newState == GLFW_RELEASE) {
    _log.console("{ --> }", "Mouse Click at", xpos, ":", ypos);
    timer = 0.0;  // reset the timer when the mouse button is released
  } else if (newState == GLFW_PRESS) {
    if (timer < 0.2) {
      if (pressTime == 0.0) {
        pressTime = glfwGetTime();
      }
      timer = glfwGetTime() - pressTime;
    } else {
      _log.console("{ --> }", "Mouse Down at", xpos, ":", ypos);
    }
  } else {
    pressTime =
        0.0;  // reset the press time when the mouse button is not pressed
  }
  oldState = newState;
}
