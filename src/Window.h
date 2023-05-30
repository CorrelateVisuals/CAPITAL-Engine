#pragma once
#include <GLFW/glfw3.h>

#include <iostream>

class Window {
 public:
  Window();
  ~Window();

  GLFWwindow* window;
  bool framebufferResized;

  struct Mouse {
    float pressDelayDuration = 0.18f;
    std::array<float, 2> leftCoords{off, off};
    std::array<float, 2> middleCoords{off, off};
    std::array<float, 2> rightCoords{off, off};
  } mouse;

  void mouseClick(GLFWwindow* win);

 private:
  void initWindow();
  static void windowResize(GLFWwindow* win, int width, int height);
};
