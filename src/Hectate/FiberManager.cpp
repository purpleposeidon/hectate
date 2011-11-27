#include "FiberManager.h"

#include <cstdlib>
#include <stdexcept>
#include <cstring>

#include <iostream>
#include <assert.h>
#include <signal.h>

#include "Hectate/common.h"

void swap(ucontext_t *old, ucontext_t *new_) {
  //std::wcout << "Swapping context " << old << " for " << new_ << std::endl;
  //std::wcout << new_->uc_flags << std::endl;
  swapcontext(old, new_);
}


FiberManager::FiberManager() {}

FiberManager::~FiberManager() {
  if (size()) {
    std::wcerr << L"FiberManager::~FiberManager: FiberManager destructed while there are still running fibers! (Did you remember to use FiberManager.die() ?)" << std::endl;
    //Would throw an exception, but this can be reached by a fiber returning. Throwing an exception in that case causes an infinite loop.
    /*
     * There are two ways that we'd be in this situation.
     * 1) Whatever original thread that was doing FiberManager.run() got interrupted
     * 2) A fiber returned without doing .die().
     *
     * TODO: In the second case, we could try running whatever comes next...
     * At least give it a try anyways.
     * */
  }
}

void FiberManager::add(FiberFunction new_fiber) {
  add(new_fiber, 0, NULL);
}

void FiberManager::add(FiberFunction new_fiber, int argc, char **argv) {
  if (size() > max_fiber_count) {
    std::wcerr << size() << " fibers" << std::endl;
    throw HectateAbort(L"FiberManager::add(): Number of fibers exceeds maximum");
  }
  ucontext_t *new_context = new ucontext_t;
  getcontext(new_context);
  new_context->uc_link = NULL;
  new_context->uc_stack.ss_sp = malloc(stack_size);
  new_context->uc_stack.ss_size = stack_size;
  new_context->uc_stack.ss_flags = 0;
  
  if (new_context->uc_stack.ss_sp == NULL) {
    throw std::bad_alloc();
  }
  memset(new_context->uc_stack.ss_sp, 0, stack_size);

  makecontext(new_context, new_fiber, 0);
  fibers.push_front(ContextItem(new_context, argc, argv));
}

unsigned int FiberManager::size() {
  return fibers.size();
}

void FiberManager::run() {
  /* XXX: This has to switch to the master context, and then to the next context.
   * It would be nice to switch directly to the following context, but if we do that,
   * it gets memory errors. Hopefully it does not give us a performance penalty.
   */
  if (size() == 0) {
    return;
  }
  
  it = fibers.begin();
  while (it != fibers.end()) {
    fiber_context = it->ucontext;
    swap(&master_context, fiber_context);
    if (fiber_died) {
      free(fiber_context->uc_stack.ss_sp);
      delete fiber_context;
      it = fibers.erase(it);
    }
    else {
      it++;
    }
  }
}


void FiberManager::yield() {
  fiber_died = false;
  swap(fiber_context, &master_context);
}

void FiberManager::die() {
  fiber_died = true;
  swap(fiber_context, &master_context);
}

FiberManager::Parameter FiberManager::getArgs() {
  return it->args;
}


