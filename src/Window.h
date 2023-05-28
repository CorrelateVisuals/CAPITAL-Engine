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
    int buttonType = parameterOff;
    float pressDelayDuration = 0.12f;
    std::array<double, 2> leftCoords = {parameterOff, parameterOff};
    std::array<double, 2> middleCoords = {parameterOff, parameterOff};
    std::array<double, 2> rightCoords = {parameterOff, parameterOff};
  } mouse;

  void getMouseButtonType();
  void mouseClick(GLFWwindow* win, int button);

 private:
  void initWindow();
  static void windowResize(GLFWwindow* win, int width, int height);
};
