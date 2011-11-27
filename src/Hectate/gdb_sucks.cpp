#include "Hectate/gdb_sucks.h"
#include <iostream>
#include "Node.h"

Context *my_context;

void toString(Atom a) {
  std::wcerr << a.toString(*my_context) << std::endl;
}

void toString(Symbol sym) {
  std::wcerr << sym.toString() << std::endl;
}

void toString(Node *n) {
  std::wcerr << n->toString(*my_context) << std::endl;
  std::wcerr << n->toStringSimple(*my_context) << std::endl;
}