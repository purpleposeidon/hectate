#include "init.h"

#include <iostream>
#include <clocale>

#include "Hectate/words.h"
#include "Hectate/Scope.h"

void init_hectate() {
  bool init = false;
  if (init) {
    return;
  }
  init = true;

  //get locale
  setlocale(LC_ALL, "");
  std::wcout << std::endl; // :|

  //initialize symbols
  Word::define();

}

