
#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "Hectate/Scope.h"
//include these as a convenience for guys in functions/
#include "Hectate/Context.h"
#include "Hectate/Atom.h"
#include <functional>

void register_all_functions(Scope &root); //Has each library 


typedef std::function<void(Scope&)> libregister;


class FunctionLibrary {
public:
  ~FunctionLibrary();
  void setLister(libregister list_function);
private:
  libregister mine;
};


#endif /* FUNCTIONS_H */

