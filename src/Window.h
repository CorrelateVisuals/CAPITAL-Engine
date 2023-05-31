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
    struct Button {
      glm::vec2 position{off, off};
    };
    std::array<Button, 3> button;
  } mouse;

  void mouseClick();

 private:
  void initWindow();
  static void windowResize(GLFWwindow* win, int width, int height);
};
