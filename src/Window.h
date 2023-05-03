#pragma once
#include <GLFW/glfw3.h>

#include <iostream>

class Window {
 public:
  Window();
  ~Window();

  GLFWwindow* window;
  bool framebufferResized;

  void initWindow();
  void mouseClick(GLFWwindow* window, int button);

 private:
  static void windowResize(GLFWwindow* window, int width, int height);
};
