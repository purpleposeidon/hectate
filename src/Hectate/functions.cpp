#include "Hectate/functions.h"

#include <list>

typedef std::list<libregister *> list_type;
list_type *libs = NULL; //stores functions from each module
//This must be a pointer because we couldn't rely on a list_type being initialized before a FunctionLibrary.

#include <iostream>

void FunctionLibrary::setLister(libregister function) {
  mine = function;
  if (libs == NULL) {
    libs = new list_type();
  }
  libs->push_back(&mine);
}

FunctionLibrary::~FunctionLibrary() {
  if (libs) {
    libs->remove(&mine);
    if (libs->size() == 0) {
      delete libs;
      libs = NULL;
    }
  }
}


void register_all_functions(Scope &root) {
  if (libs == NULL) {
    throw HectateAbort(L"libs list not initialized?");
    //Maybe FunctionLibrary's are being defined, but libs is getting set to NULL afterwards?
    //(Or there just aren't any FL defined)
  }
  if (libs->size() == 0) {
    throw HectateAbort(L"No functions");
  }
  for (auto it = libs->begin(); it != libs->end(); it++) {
    (**it)(root);
  }
}
