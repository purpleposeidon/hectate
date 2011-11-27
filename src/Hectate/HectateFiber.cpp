#include "HectateFiber.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <exception>
#include <mutex>

#include "Hectate/Context.h"
#include "Hectate/loader.h"
#include "Hectate/functions.h"
#include "Hectate/init.h"
#include "Hectate/common.h"
#include "Hectate/words.h"
#include "Hectate/Parser.h"
#include "Hectate/gdb_sucks.h"

#define thread_local __thread //TODO: When g++0x supports it, delete
thread_local FiberManager *fm;
Scope *root;
Atom root_atom;

void new_manager() {
  static std::mutex f_mutex;
  std::lock_guard<std::mutex> _(f_mutex);
  init_hectate();
  fm = new FiberManager;
}

void delete_manager() {
  static std::mutex f_mutex;
  std::lock_guard<std::mutex> _(f_mutex);
  delete fm;
  fm = NULL;
}

FiberManager *get_manager() {
  //return the FiberManager for this thread
  return fm;
}


Atom &setup_root() {
  static std::mutex f_mutex;
  std::lock_guard<std::mutex> _(f_mutex);
  if (root == NULL) {
    root = new Scope;
    root->set(Word::ScopeName, Atom::create(Word::ScopeRoot));
    root_atom.set(root);
    register_all_functions(*root);
    root->makeRoot();
    root->freeze(true);
  }
  return root_atom;
}

void run_file(const char *name, Context &context, bool do_file_error = true) {
  StringStream contents;
  contents << L"";
  std::wifstream file(name);
  if (!file.is_open()) {
    if (do_file_error) {
      StringStream msg;
      msg << L"File does not exist: " << name;
      throw HectateError(msg.str());
    }
    return;
  }
  
  contents << file.rdbuf();
  file.close();
  

  Atom node_atom;
  try {
    node_atom = Atom::create(Parser().parseFile(contents.str()));
  }
  catch (ParseError &e) {
    std::wcout << bold(L"parse failed") << L" in file " << name << L": " << e.get_msg();
    return;
  }
  context.run(node_atom);
}

void REPL(Context &context) {
  while (1) {
    //read
    Node *n = read_node(); //this is freed when node_atom is destroyed
    if (n == NULL) {
      return;
    }
    
    //eval
    Atom node_atom = Atom::create(n);
    context.run(node_atom);
    
    //print
    EvalAction action = context.action();
    Atom &result = context.result();
    if (result.isType(NONE)) {
      if (action != RESULT) {
        std::wcout << bold(eval_action_name(action));
      }
      std::wcout << std::endl;
    }
    else {
      if (action == RESULT) {
        std::wcout << L"→ ";
      }
      else {
        std::wcout << bold(eval_action_name(action)) << ": ";
      }
      std::wcout << type_name(result.getType()) << L" ";
      std::wcout << result.toString(context) << std::endl;
    }
    
    //Go looping! Hayo!
  }
}

void hectate_fiber_body(FiberManager *my_manager) {
  Context context(my_manager);
  context.setRoot(setup_root());
  my_context = &context;
  
  auto arg = my_manager->getArgs();

  run_file("./init.h6x", context, false);
  
  for (int i = 1; i < arg.argc; i++) {
    if (strcmp(arg.argv[i], "-") == 0) {
      REPL(context);
    }
    else {
      run_file(arg.argv[i], context);
    }
  }

  if (arg.argc <= 1) {
    //have no args
    REPL(context);
  }
  
}

void hectate_fiber() {
  FiberManager *my_manager = get_manager();
  #if CATCH_ERRORS
    try {
      hectate_fiber_body(my_manager);
    }
    catch (HectateError &e) {
      std::wcout << L"hectate_fiber() terminated by HectateError: " << e.get_msg() << std::endl;
    }
    catch (std::exception &e) {
      std::wcout << L"hectate_fiber() terminated by std::exception: " << e.what() << std::endl;
    }
    catch (...) {
      std::wcout << L"hectate_fiber() terminated by unknown error" << std::endl;
    }
  #else
    hectate_fiber_body(my_manager);
  #endif

  //unnatural exit from function
  my_manager->die();
}

