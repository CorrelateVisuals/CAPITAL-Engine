#include <iostream>
#include <string>

#include "Capital_Engine.h"
#include "Debug.h"

int main() {
  LOG("[ Starting CAPITAL engine ]");

  try {
    CapitalEngine CAPITAL;

    CAPITAL.mainLoop();
  } catch (const std::exception& e) {
    LOG(e.what());
    return EXIT_FAILURE;
  } catch (...) {
    LOG("!!! Unknown error caught in main() !!!");
  }

  LOG("[ CAPITAL engine terminated ]");
  return EXIT_SUCCESS;
}
