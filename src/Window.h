#pragma once
#include <GLFW/glfw3.h>

#include <iostream>

class Window {
 public:
  Window();
  ~Window();

  GLFWwindow* window;
  bool framebufferResized;

  struct MousePositions {
    int buttonType = -1;
    std::array<double, 2> leftCoords = {0.5, 0.5};
    std::array<double, 2> middleCoords = {0.5, 0.5};
    std::array<double, 2> rightCoords = {0.5, 0.5};
  } mouse;

  void initWindow();

  void getMouseButtonTypePressed();
  void mouseClick(GLFWwindow* window, int button);

 private:
  static void windowResize(GLFWwindow* window, int width, int height);
};
