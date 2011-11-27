#include "Hectate/Parser.h"
#include "Hectate/Context.h"
#include "Hectate/HectateFiber.h"

int PARALLEL = 1;
FiberManager *f;
bool tests_remain = true;

void run_tests() {
  while (f->size()) {
    f->run();
  }
}

int tester_index = 0;

void tester_fiber() {
  int index = tester_index;
  tester_index++;
  if (index < 10) {
    return;
  }
  switch (index) {
    default:
      tests_remain = false;
      break;
  }
}

void add_test() {
  f->add(tester_fiber);
  if (!PARALLEL) {
    run_tests();
  }
}

int main() {
  new_manager();
  f = get_manager();


  while (tester_index < 200) {
    add_test();
  }

  if (PARALLEL) {
    run_tests();
  }

  delete_manager();
  return 0;
}

