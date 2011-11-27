#include "Hectate/functions.h"

#include <iostream>

#include <signal.h>
#include <unistd.h>
#include "Hectate/builtins_info.h"



void h_crash(Context &context) {
  //TODO: Catch this
  //Should do an ABORT
  kill(getpid(), SIGSEGV);
  throw HectateError(L"h_crash(): still alive!");
  throw HectateError(L"h_crash(): I'm still alive!");
}

void h_leak(Context &context) {
  //TODO: Get memory from context
  const unsigned int to_leak = 16;
  new char[to_leak];
  std::wcout << L"Leaked " << to_leak << " bytes of memory.\n";
  std::wcout << L"(TODO: allocate memory via Context)\n";
  context.finish();
}

void h_watch(Context &context) {
  //Sets a watch on arg1
  ArgManager arg = context.arg(L"[debug/watch]");
  arg.getEval(L"[debug/watch MEMORY-VALUE]").watch(true);
}

void h_builtin_backtrace(Context &context) {
  //gets a C++ backtrace
  ArgManager arg = context.arg(L"[debug/builtin-backtrace]");
  bool raw_backtrace;
  if (!arg.done()) {
    raw_backtrace = arg.getEval(L"debug/builtin-backtrace NO-DEMANGLE?").convert(context, BOOLEAN).getBoolean();
  }
  arg.beDone();
  context.finish(Atom::create(new String(builtin_backtrace(raw_backtrace))));
}


void h_hang_sleep(Context &context) {
  //sleep forever
  //TODO: Interrupt
  std::wcout << L"Sleeping forever\n";
  while (1) {
    sleep(60);
  }
}

void h_hang_busy(Context &context) {
  //endless loop
  //TODO: Interrupt
  //Should be stopped by a watchdog, or sigalarm or something (use setitimer)
  std::wcout << L"Entering endless loop\n";
  while (1) {}
}

void h_hang_yield(Context &context) {
  //endless yield
  //Should be stopped by an eval_step limiter
  std::wcout << L"Entering endless loop with yielding\n";
  while (1) {
    context.wait();
  }
}

Iterator *watchable;
void h_ruin(Context &context) {
  ArgManager arg = context.arg(L"[ruin]");
  Atom ai = arg.getIterator();
  ai.watch(true);
  context.setSymbol(Symbol::create(L"DOOM"), ai);
  watchable = dynamic_cast<Iterator*>(&ai.getAbstract());
}

void h_wat(Context &context) {
  context.pushScope();
  context.popScope();
}

class DebugLibrary : public FunctionLibrary {
public:
  DebugLibrary() {
    setLister([](Scope &root) {
      Scope *debug = root.subscope(L"debug");
      debug->set(L"builtin-backtrace", h_builtin_backtrace);
      debug->set(L"crash", h_crash);
      debug->set(L"leak", h_leak);
      debug->set(L"watch", h_watch);
      debug->set(L"hang-sleep", h_hang_sleep);
      debug->set(L"hang-busy", h_hang_busy);
      debug->set(L"hang-yield", h_hang_yield);

      debug->set(L"ruin", h_ruin);
      debug->set(L"wat", h_wat);
    });
  }
} debug_lib;

