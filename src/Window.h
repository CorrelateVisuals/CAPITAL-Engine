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
    std::array<glm::vec2, 3> coords{glm::vec2(off, off), glm::vec2(off, off),
                                    glm::vec2(off, off)};
  } mouse;

  void mouseClick();

 private:
  void initWindow();
  static void windowResize(GLFWwindow* win, int width, int height);
};
