#pragma once
#include <GLFW/glfw3.h>

#include <iostream>

class Window {
 public:
  Window();
  ~Window();

  GLFWwindow* window;
  bool framebufferResized;

  void mouseClick(GLFWwindow* window, int button);

 private:
  void initWindow();
  static void windowResize(GLFWwindow* window, int width, int height);
};
inline Window mainWindow;
