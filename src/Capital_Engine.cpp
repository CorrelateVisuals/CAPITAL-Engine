#include <iostream>

#include "Capital_Engine.h"
#include "Debug.h"
#include "Window.h"

CapitalEngine::CapitalEngine() {
  LOG("... constructing CapitalEngine");
  initVulkan();
  LOG("... CapitalEngine constructed");
}

CapitalEngine::~CapitalEngine() {
  LOG("... destructing CapitalEngine");

  LOG("... CapitalEngine destructed");
}

void CapitalEngine::mainLoop() {
  while (!glfwWindowShouldClose(mainWindow.window)) {
    glfwPollEvents();
    mainWindow.mouseClick(mainWindow.window, GLFW_MOUSE_BUTTON_LEFT);
  }
}

void CapitalEngine::initVulkan() {
  LOG("... initializing CapitalEngine");

  LOG("... CapitalEngine initialized");
}
