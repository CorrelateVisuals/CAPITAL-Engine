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
    std::array<float, 2> leftCoords = {parameterOff, parameterOff};
    std::array<float, 2> middleCoords = {parameterOff, parameterOff};
    std::array<float, 2> rightCoords = {parameterOff, parameterOff};
  } mouse;

  void mouseClick(GLFWwindow* win);

 private:
  void initWindow();
  static void windowResize(GLFWwindow* win, int width, int height);
};
