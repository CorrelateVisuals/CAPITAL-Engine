#include <iostream>
#include <string>

#include "CAPITAL_Engine.h"

int main() {
  try {
    CapitalEngine CAPITAL;
    CAPITAL.mainLoop();
  } catch (const std::exception& e) {
    _log.console(e.what());
    return EXIT_FAILURE;
  } catch (...) {
    _log.console("!!!!!!!!!", "unknown error caught in main()");
  }
  return EXIT_SUCCESS;
}
