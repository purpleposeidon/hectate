
#include <iostream>
#include <clocale>

#include "Hectate/Parser.h"
#include "Hectate/Context.h"

#include "Hectate/HectateFiber.h"
#include "Hectate/FiberManager.h"



int main(int argc, char **argv) {
  new_manager();
  FiberManager *fm = get_manager();
  fm->add(hectate_fiber, argc, argv);
  while (fm->size()) {
    fm->run();
  }
  delete_manager();
  std::wcout << "Goodbye." << std::endl;
  return 0;
}

