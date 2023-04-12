#include <windows.h>
#include <iostream>
#include <string>

#include "Capital_Engine.h"
#include "Debug.h"

int main() {
  LOG("\n[ Starting CAPITAL engine ]\n");

  try {
    CapitalEngine CAPITAL;

    CAPITAL.mainLoop();
  } catch (const std::exception& e) {
    LOG(e.what());
    return EXIT_FAILURE;
  } catch (...) {
    LOG("!!! unknown error caught in main() !!!");
  }

  LOG("\n[ CAPITAL engine terminated ]\n");
  return EXIT_SUCCESS;
}
