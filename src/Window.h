#pragma once
#include <GLFW/glfw3.h>

#include <iostream>

class Window {
 public:
  Window();
  ~Window();

  GLFWwindow* window;
  bool framebufferResized = false;

  void initWindow();
  static void windowResize(GLFWwindow* window, int width, int height);
  void mouseClick(GLFWwindow* window, int button);
};

inline Window mainWindow;
