#include <iostream>

#include "CapitalEngine.h"
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

void Window::getMouseButtonType() {
  const int mouseButtonTypes[] = {GLFW_MOUSE_BUTTON_LEFT,
                                  GLFW_MOUSE_BUTTON_MIDDLE,
                                  GLFW_MOUSE_BUTTON_RIGHT};
  int buttonState = GLFW_RELEASE;
  for (const auto& mouseButtonType : mouseButtonTypes) {
    buttonState = glfwGetMouseButton(_window.window, mouseButtonType);
    if (buttonState == GLFW_PRESS) {
      mouse.buttonType = mouseButtonType;
      return;
    }
  }
}

void Window::mouseClick(GLFWwindow* win, int button) {
  static int oldState = GLFW_RELEASE;
  int newState = glfwGetMouseButton(win, button);
  static double timer = 0.0;
  static double pressTime = 0.0;
  double xpos, ypos;
  glfwGetCursorPos(win, &xpos, &ypos);
  xpos /= _control.display.width;
  ypos /= _control.display.height;

  switch (oldState) {
    case GLFW_PRESS:
      switch (newState) {
        case GLFW_RELEASE:
          if (button == GLFW_MOUSE_BUTTON_LEFT) {
            mouse.leftCoords = {xpos, ypos};
            _log.console("{ --> }", "Left Mouse Button clicked at", xpos, ":",
                         ypos);
          } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            mouse.middleCoords = {xpos, ypos};
            _log.console("{ --> }", "Middle Mouse Button clicked at", xpos, ":",
                         ypos);
          } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            mouse.rightCoords = {xpos, ypos};
            _log.console("{ --> }", "Right Mouse Button clicked at", xpos, ":",
                         ypos);
          }
          timer = 0.0;
          break;
        case GLFW_PRESS:
          if (timer < mouse.pressDelayDuration) {
            if (pressTime == 0.0) {
              pressTime = glfwGetTime();
            }
            timer = glfwGetTime() - pressTime;
          } else {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
              mouse.leftCoords = {xpos, ypos};
              _log.console("{ --> }", "Left Mouse Button down at", xpos, ":",
                           ypos);
            } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
              mouse.middleCoords = {xpos, ypos};
              _log.console("{ --> }", "Middle Mouse Button down at", xpos, ":",
                           ypos);
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
              mouse.rightCoords = {xpos, ypos};
              _log.console("{ --> }", "Right Mouse Button down at", xpos, ":",
                           ypos);
            }
          }
          break;
      }
      break;
    case GLFW_RELEASE:
      if (newState == GLFW_PRESS) {
        pressTime = glfwGetTime();
      } else {
        pressTime = 0.0;
      }
      break;
  }
  oldState = newState;
}
