#include <iostream>
#include <string>

#include "CapitalEngine.h"

int main() {
  try {
    CapitalEngine CAPITAL;
    CAPITAL.mainLoop();
  } catch (const std::exception& e) {
    _log.console(e.what());
    return EXIT_FAILURE;
  } catch (...) {
    throw std::runtime_error("!ERROR! Unknown error caught in main()");
  }
  return EXIT_SUCCESS;
}
