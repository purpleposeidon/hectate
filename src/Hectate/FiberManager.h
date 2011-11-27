
#ifndef FIBERMANAGER_H
#define FIBERMANAGER_H

#include <ucontext.h>
#include <list>

typedef void (FiberFunction)(void);

class FiberManager {
public:
  FiberManager();
  ~FiberManager();
  //Fiber management functions
  void add(FiberFunction new_fiber, int argc, char **argv); //Creates a new fiber
  void add(FiberFunction new_fiber); //Creates a new fiber
  unsigned int size(); //returns how many running fibers there are
  void run(); //Called by hectate thread to run all of our fibers
  void crash_handler();

  //Fiber control functions
  void yield(); //Called by a fiber to run the next one
  void die(); //Called by a fiber that has finished.

  struct Parameter {
    Parameter(int argc, char **argv) : argc(argc), argv(argv) {}
    int argc;
    char **argv;
  };

  Parameter getArgs();

  
private:
  const static unsigned int stack_size = SIGSTKSZ*100;
  const static unsigned int max_fiber_count = 500;


  struct ContextItem {
    ContextItem(ucontext_t *ucontext, int argc, char **argv) : ucontext(ucontext), args(argc, argv) {}
    ucontext_t *ucontext;
    Parameter args;
  };
  typedef std::list<ContextItem> uc_list;
  
  ucontext_t master_context; //the original, primary context is stored here.
  ucontext_t *fiber_context;
  uc_list fibers;
  uc_list::iterator it;
  bool fiber_died;
  void pop_and_run();
};

#endif /* FIBERMANAGER_H */

