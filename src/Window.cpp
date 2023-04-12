#include <iostream>

#include "Debug.h"
#include "Settings.h"
#include "Window.h"

Window::Window() : window{nullptr}, framebufferResized{false} {
  LOG("... constructing window");
  initWindow();
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
  LOG("... window terminated");
}

void Window::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(displayConfig.width, displayConfig.height,
                            displayConfig.windowTitle, nullptr, nullptr);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, windowResize);
  LOG("... window initialized with", displayConfig.width, displayConfig.height);
}

void Window::windowResize(GLFWwindow* window, int width, int height) {
  auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
  displayConfig.width = width;
  displayConfig.height = height;
  LOG("... window resized to", width, height);
}

void Window::mouseClick(GLFWwindow* window, int button) {
  static int oldState = GLFW_RELEASE;
  int newState = glfwGetMouseButton(window, button);

  if (oldState == GLFW_PRESS && newState == GLFW_RELEASE) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    xpos /= displayConfig.width;
    ypos /= displayConfig.height;
    LOG("> > mouse input at", xpos, ypos);
  }
  oldState = newState;
}
