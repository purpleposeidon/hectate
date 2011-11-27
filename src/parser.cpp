#include <iostream>
using namespace std;

#include "Hectate/loader.h"
#include "Hectate/init.h"
#include "Hectate/Node.h"
#include "Hectate/Context.h"

void test_parse() {
  Context test_context(NULL);
  while (Node *parse = read_node()) {
    wcout << "Pretty parse:" << endl;
    wcout << parse->toString(test_context) << endl;
    wcout << endl << "Simple parse:" << endl;
    wcout << parse->toStringSimple(test_context) << endl;
    delete parse;
  }
}



int main() {
  init_hectate();
  //test_parse();
  try {
    test_parse();
  }
  catch (HectateError &e) {
    wcout << e.get_msg() << endl;
  }
  return 0;
}
