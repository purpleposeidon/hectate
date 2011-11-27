#include "Hectate/FiberManager.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

__thread FiberManager *manager;

int index = 0;

void nice_test() {
  index++;
  int mycount = 0;
  while (mycount < 3) {
    //wcout << "Fiber #" << my_index << " has counted to " << mycount << endl;
    mycount++;
    int *x = new int;
    manager->yield();
    delete x;
  }
  //wcout << "Fiber #" << my_index << " has counted high enough!" << endl;
  manager->die();
  //wcout << "I shouldn't be here. What?" << endl;
}

void bad_test() {
  //wcout << "bad_test: start" << endl;
  for (int i = 3; i; i--) {
    //wcout << "bad_test: yield" << endl;
    manager->add(nice_test);
    manager->yield();
  }
  //wcout << "bad_test: start" << endl;
  manager->die();
}

int fibo(int n) {
  if (n == 0 || n == 1) {
    return 1;
  }
  return fibo(n-2)+fibo(n-1);
}

void death_by_recursion(unsigned int i) {
  death_by_recursion(i+1);
}

void simple_test() {
  fibo(25);
  manager->die();
}

void f(unsigned int i) {
  while (i--) {
    manager->add(nice_test);
  }
}


void poly(unsigned int i, FiberFunction func) {
  while (i--) {
    manager->add(func);
    wcout << manager->size() << endl;
  }
}

void hate() {
  int i = 5;
  while (1) {
    manager->yield();
    i--;
    if (i == 0) {
      //wcout << "Breedin'" << endl;
      manager->add(nice_test);
    }
    else {
      //wcout << "Hatin'   " << i << endl;
    }
    if (i == -3) {
      manager->die();
    }
    else {
      manager->yield();
    }
  }
}

int main() {
  manager = new FiberManager;
  int N = 1000;
  poly(N, simple_test);
  poly(N, bad_test);
  poly(N, nice_test);
  poly(N, hate);
  while (manager->size()) {
    manager->run();
  }
  delete manager;
  return 0;
}


