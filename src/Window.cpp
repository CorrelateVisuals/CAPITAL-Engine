#include <iostream>

#include "CapitalEngine.h"
#include "Control.h"
#include "Window.h"

Window::Window() : window{nullptr}, framebufferResized{false} {
  _log.console("{ [-] }", "constructing Window");
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
  window = glfwCreateWindow(_control.display.width, _control.display.height,
                            _control.display.title, nullptr, nullptr);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, windowResize);
  _log.console("{ [*] }", "Window initialized with", _control.display.width,
               "*", _control.display.height);
}

void Window::windowResize(GLFWwindow* win, int width, int height) {
  auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
  app->framebufferResized = true;
  _control.display.width = width;
  _control.display.height = height;
  _log.console("{ [*] }", "Window resized to", width, "*", height);
}

void Window::mouseClick() {
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  float x = static_cast<float>(xpos) / _control.display.width;
  float y = static_cast<float>(ypos) / _control.display.height;

  static int oldState = GLFW_RELEASE;
  int newState = GLFW_RELEASE;
  static float timer = 0.0;
  static float pressTime = 0.0;
  static int buttonType = -1;
  const int mouseButtonTypes[] = {GLFW_MOUSE_BUTTON_LEFT,
                                  GLFW_MOUSE_BUTTON_MIDDLE,
                                  GLFW_MOUSE_BUTTON_RIGHT};

  for (const auto& mouseButtonType : mouseButtonTypes) {
    if (glfwGetMouseButton(window, mouseButtonType) == GLFW_PRESS) {
      newState = GLFW_PRESS;
      buttonType = mouseButtonType;
      break;
    }
  }

  switch (oldState) {
    case GLFW_PRESS:
      switch (newState) {
        case GLFW_RELEASE:
          if (buttonType == GLFW_MOUSE_BUTTON_LEFT) {
            mouse.leftCoords = {x, y};
            _log.console("{ --> }", "Left Mouse Button clicked at", x, ":", y);
          } else if (buttonType == GLFW_MOUSE_BUTTON_MIDDLE) {
            mouse.middleCoords = {x, y};
            _log.console("{ --> }", "Middle Mouse Button clicked at", x, ":",
                         y);
          } else if (buttonType == GLFW_MOUSE_BUTTON_RIGHT) {
            mouse.rightCoords = {x, y};
            _log.console("{ --> }", "Right Mouse Button clicked at", x, ":", y);
          }
          timer = 0.0;
          break;
        case GLFW_PRESS:
          timer = static_cast<float>(glfwGetTime()) - pressTime;
          if (timer >= mouse.pressDelayDuration) {
            if (buttonType == GLFW_MOUSE_BUTTON_LEFT) {
              mouse.leftCoords = {x, y};
              _log.console("{ --> }", "Left Mouse Button down at", x, ":", y);
            } else if (buttonType == GLFW_MOUSE_BUTTON_MIDDLE) {
              mouse.middleCoords = {x, y};
              _log.console("{ --> }", "Middle Mouse Button down at", x, ":", y);
            } else if (buttonType == GLFW_MOUSE_BUTTON_RIGHT) {
              mouse.rightCoords = {x, y};
              _log.console("{ --> }", "Right Mouse Button down at", x, ":", y);
            }
          }
          break;
      }
      break;
    case GLFW_RELEASE:
      if (newState == GLFW_PRESS) {
        pressTime = static_cast<float>(glfwGetTime());
      } else {
        pressTime = 0.0;
      }
      break;
  }
  oldState = newState;
}
