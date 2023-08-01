#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

class Window {
 public:
  Window();
  ~Window();

  GLFWwindow* window;
  bool framebufferResized;

  struct Mouse {
    float pressDelay = 0.18f;
    float speed = 0.5f;

    struct Button {
      glm::vec2 position;
    };
    std::array<Button, 3> buttonClick;
    std::array<Button, 3> buttonDown;
    std::array<Button, 3> previousButtonDown;
  } mouse;

  void setMouse();

 private:
  void initWindow();
  static void windowResize(GLFWwindow* win, int width, int height);
};
