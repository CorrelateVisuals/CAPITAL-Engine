#include <iostream>
#include <unordered_map>

#include "CapitalEngine.h"
#include "Control.h"
#include "Library.h"
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
  int newState = GLFW_RELEASE;
  static int buttonType = -1;
  const static int mouseButtonTypes[]{GLFW_MOUSE_BUTTON_LEFT,
                                      GLFW_MOUSE_BUTTON_RIGHT,
                                      GLFW_MOUSE_BUTTON_MIDDLE};

  for (const int& mouseButtonType : mouseButtonTypes) {
    if (glfwGetMouseButton(window, mouseButtonType) == GLFW_PRESS) {
      newState = GLFW_PRESS;
      buttonType = mouseButtonType;
      break;
    }
  }

  if (buttonType != -1) {
    static int oldState = GLFW_RELEASE;
    double xpos, ypos;
    static float timer = 0.0f;
    static float pressTime = 0.0f;

    glfwGetCursorPos(window, &xpos, &ypos);
    const float x = static_cast<float>(xpos) / _control.display.width;
    const float y = static_cast<float>(ypos) / _control.display.height;

    static const std::unordered_map<int, std::string> buttonMappings = {
        {GLFW_MOUSE_BUTTON_LEFT, "{ --> } Left Mouse Button"},
        {GLFW_MOUSE_BUTTON_RIGHT, "{ --> } Right Mouse Button"},
        {GLFW_MOUSE_BUTTON_MIDDLE, "{ --> } Middle Mouse Button"}};

    switch (oldState) {
      case GLFW_PRESS: {
        if (newState == GLFW_RELEASE) {
          const std::unordered_map<int, std::string>::const_iterator&
              buttonMapping = buttonMappings.find(buttonType);
          if (buttonMapping != buttonMappings.end()) {
            const std::string& message = buttonMapping->second;
            mouse.buttonClick[buttonType].position = glm::vec2{x, y};

            _log.console(message + " clicked at",
                         mouse.buttonClick[buttonType].position.x, ":",
                         mouse.buttonClick[buttonType].position.y);
            timer = 0.0f;
          }
        } else {
          const float currentTime = static_cast<float>(glfwGetTime());
          const float timer = currentTime - pressTime;
          if (timer >= mouse.pressDelay) {
            const std::unordered_map<int, std::string>::const_iterator&
                buttonMapping = buttonMappings.find(buttonType);
            if (buttonMapping != buttonMappings.end()) {
              const std::string& message = buttonMapping->second;
              glm::vec2 normalizedCoords =
                  glm::vec2(x, y) * glm::vec2(2.0f, 2.0f) -
                  glm::vec2(1.0f, 1.0f);
              mouse.buttonDown[buttonType].position +=
                  normalizedCoords * mouse.speed;

              //_log.console(message + " moved to",
              //             mouse.buttonDown[buttonType].position.x, ":",
              //             mouse.buttonDown[buttonType].position.y);
            }
          }
        }
        break;
      }
      case GLFW_RELEASE: {
        pressTime =
            (newState == GLFW_PRESS) ? static_cast<float>(glfwGetTime()) : 0.0f;
        break;
      }
    }
    oldState = newState;
  }
}
