#include <iostream>
#include <string>

#include "Capital_Engine.h"
#include "Debug.h"

int main() {
  try {
    CapitalEngine CAPITAL;
    CAPITAL.mainLoop();
  } catch (const std::exception& e) {
    LOG(e.what());
    return EXIT_FAILURE;
  } catch (...) {
    LOG("!!!!!!!!!", "unknown error caught in main()");
  }
  return EXIT_SUCCESS;
}
