
#include "Hectate/functions.h"
#include "Hectate/Context.h"
#include "Hectate/loader.h"
#include "Hectate/words.h"
#include "Hectate/AbstractType.h"

#include <iostream>
#include <vector>





void h_quote(Context &context) {
  ArgManager arg = context.arg(L"[quote]");
  if (arg.done()) {
    throw HectateError(L"[quote] needs an argument");
  }
  const Atom &val = arg.getQuote(L"[quote WHAT]");
  arg.beDone();
  context.finish(val);
}


void h_set(Context &context) {
  //get symbol to set
  ArgManager arg = context.arg(L"[set]");
  Symbol to_set = arg.getCommand(L"[set SYMBOL value?]").getSymbol();
  Atom set_value = ATOM_NONE;
  if (!arg.done()) {
    set_value = arg.getEval(L"[set SYMBOL what]");
    arg.beDone();
  }
  context.setSymbol(to_set, set_value);
  context.finish(set_value);
}

//TODO: `"string `format syntax" should default to creating an iterator rather than string-join?
void h_string_join(Context &context) {
  ArgManager arg = context.arg(L"[string-join]");
  String result = L"";
  while (!arg.done()) {
    result += arg.getWant(L"[string-join STRING_?]", STRING).getString();
  }
  context.finish(RESULT, Atom::create(new String(result)));
}

void h_read_node(Context &context) {
  context.arg(L"[read-node]").beDone();
  Node *n = read_node();
  if (n == NULL) {
    throw HectateError(L"[read-node]: Got NULL node");
  }
  context.finish(RESULT, Atom::create(n));
}

void h_ls(Context &context) {
  context.arg(L"[ls]").beDone();
  context.map_scope_stack([&](Scope &s) {
    std::wcout << s.toString(context) << std::endl;
    return false;
  });
  context.finish();
}





//h_print
void h_console_print(Context &context) {
  ArgManager arg = context.arg(L"[print]");
  while (!arg.done()) {
    std::wcout << arg.getWant(L"[print STRING_?]", STRING).getString();
  }
  context.finish();
}

class SetArgResult : public AbstractType {
public:
  SetArgResult(Atom &result) : result(result) {}
  
  virtual void eval(Context &context) {
    context.finish(RESULT, result);
  }

  virtual String toString(Context &context) {
    return String(L"[set-arg] RESULT wrapper of ") + result.toStringRepr(context);
  }

  virtual MapAction map(Container::MapFunction function) {
    return function(result);
  }
  
private:
  Atom result;
};

void h_set_arg(Context &context) {
  //NOTE: This function can be written in terms of a simpler function that gets the arguments passed to the call stack at arbitrary points
  ArgManager arg = context.arg(L"[set-arg]");
  Atom to_eval = arg.getQuote(L"[set-arg TO-RUN]");
  
  struct ArgSave {
    Context &context;
    bool had_arg;
    Atom old_arg;
    
    ArgSave(Context &context) : context(context) {
      had_arg = context.hasSymbol(Word::Arguments);
      if (had_arg) {
        old_arg = context.getSymbol(Word::Arguments);
      }
    }
    ~ArgSave() {
      if (had_arg) {
        context.setSymbol(Word::Arguments, old_arg);
      }
      else {
        context.unsetSymbol(Word::Arguments);
      }
    }
  } arg_saver(context); //save and restore the original [arg]
  ArgManager *prev_arg = context.argAt(1);
  if (prev_arg == NULL) {
    throw HectateError(L"[set-arg]: no outer expression");
  }
  context.setSymbol(Word::Arguments, prev_arg->getIterator());
  context.eval(to_eval, false);
  if (context.action() == RESULT) {
    //This function is always used like
    //[[set-arg BODY] ARGS]. And arg0 needs to return something for ARGS to be called with.
    //So, this will be a wrapper object whose mission is to return the result and ignore all args.
    context.finish(Atom::create(new SetArgResult(context.result())));
  }
}


void h_load_arg(Context &context) {
  //TODO: Does [load-arg] have a shitty name?
  ArgManager &arg = context.arg(L"[load-arg]");
  ArgManager src(context, context.getSymbol(Word::Arguments), L"[load-arg] puller");
  typedef std::pair<Symbol, Atom> pair;
  std::vector<pair> item_list;
  
  Atom code;
  while (1) {
    Atom next = arg.getQuote(L"[load-arg |LOAD-METHOD name|_? CODE]");
    if (arg.done()) {
      //It's code
      code = next;
      break;
    }
    else {
      Symbol cmd = next.getSymbol();
      Symbol name = arg.getCommand(L"[load |load-method NAME|_? code]").getSymbol();
      if (cmd == Word::LoadEval) {
        item_list.push_back(pair(name, src.getEval(L"[load eval]")));
      }
      else if (cmd == Word::LoadQuote) {
        item_list.push_back(pair(name, src.getQuote(L"[load quote]")));
      }
      else {
        throw HectateError(L"[load]: load-method must be 'eval' or 'quote'.");
      }
    }
  }

  for (auto it = item_list.begin(); it != item_list.end(); it++) {
    context.setSymbol(it->first, it->second);
  }

  context.eval(code);
}

void h_function(Context &context) {
  //[function <evaluated arg list [quoted] [args] [in] [brackets]> body]
  ArgManager &arg = context.arg(L"[function]");
  ArgManager *above_arg = context.argAt(1);

  //create a scope with arguments
  Node &arguments = arg.getQuote(L"[function <ARG LIST> body]").getNode();
  typedef std::pair<Symbol, Atom> Pair;
  std::vector<Pair> scope;
  for (auto it = arguments.begin(); it != arguments.end(); it++) {
    Atom name = *it;
    if (name.isType(SYMBOL)) {
      scope.push_back(Pair(name.getSymbol(), above_arg->getEval(L"[function <ARG> body] eval'd argument")));
      continue;
    }
    else if (name.isType(NODE)) {
      Node &n = name.getNode();
      if (n.size() != 1) {
        throw HectateError(L"[function <[ARG] [LIST]> body]: Items for quoting be singleton");
      }
      scope.push_back(Pair(name.getSymbol(), above_arg->getQuote(L"[function <[ARG]> body] quoted argument")));
    }
    throw HectateError(L"[function <ARG LIST> body]: arg list must have only symbol and <symbol>");
  }

  struct ScopeHolder {
    ScopeHolder(Context &c) : context(c) {
      context.pushScope();
    }
    ~ScopeHolder() {
      context.popScope();
    }
    Context &context;
  } SH(context);
  
  for (auto it = scope.begin(); it != scope.end(); it++) {
    context.setSymbol(it->first, it->second);
  }

  //Now run that bod
  Atom body = arg.getQuote(L"[function <arg list> BODY]");
  context.eval(body);
}

void h_scope(Context &context) {
  //[scope code]: create a scope to run code in
  ArgManager arg = context.arg(L"[scope]");
  Atom code = arg.getQuote(L"[scope CODE]");
  arg.beDone();
  struct RAI {
    RAI(Context &context) : context(context) {
      context.pushScope();
    }
    ~RAI() {
      context.popScope();
    }
    Context &context;
  } rai(context);
  context.eval(code);
}

void h_scope_freeze(Context &context) {
  //[scope-freeze scope code]
  /* Store scope's freeze status
   * .code
   * restore scope's freeze status.
   *
   * Careful: If status isn't restored properly, frozen scopes could be unfrozen
   * */
}

void h_scope_freeze_forever(Context &context) {
  //[scope-freeze-forever scope]
  //Careful: Only allow on scopes that we have write-access to.
}

void h_include(Context &context) {
  ArgManager arg = context.arg(L"[include]");
  auto &includes = context.getIncludes();
  while (!arg.done()) {
    if (includes.size() > Limit::include_length) {
      throw HectateError(L"[include]: Too many scopes in include list");
    }
    Atom s = arg.getEval(L"[include WHAT_?]").unweaken();
    if (!s.isType(SCOPE)) {
      throw HectateError(L"[include]: argument is not a scope");
    }
    includes.insert(s);
  }
  context.finish();
}

void h_exclude(Context &context) {
  ArgManager arg = context.arg(L"[exclude]");
  auto &includes = context.getIncludes();
  while (!arg.done()) {
    Atom scope_atom = arg.getEval(L"[exclude SCOPE_?]").unweaken();
    if (!scope_atom.isType(SCOPE)) {
      continue; //just ignore it
    }
    Scope &s = scope_atom.getScope();
    for (auto it = includes.begin(); it != includes.end(); it++) {
      if (s == it->getScope()) {
        it = includes.erase(it);
      }
    }
  }
  context.finish();
}

void h_include_show(Context &context) {
  context.arg(L"[include-show]").beDone();
  auto &includes = context.getIncludes();
  for (auto it = includes.begin(); it != includes.end(); it++) {
    std::wcout << it->getScope().toString(context) << L"\n";
  }
  if (includes.size() == 0) {
    std::wcout << L"No includes";
  }
  context.finish();
}

class BasicLibrary : public FunctionLibrary {
public:
  BasicLibrary() {
    setLister([](Scope &root) {
      Scope *basic = &root; //root.subscope(L"basic");
      basic->set(L"quote", h_quote);
      //basic->set(L"scope", h_scope);
      basic->set(L"ls", h_ls); //basic->set(L"dir", h_root_dir);
      basic->set(L"set", h_set);
      basic->set(L"string-join", h_string_join);
      basic->set(L"read-node", h_read_node); //TODO: Make this something proper
      basic->set(L"print", h_console_print);
      basic->set(L"set-arg", h_set_arg);
      basic->set(L"load-arg", h_load_arg);
      basic->set(L"scope", h_scope);
      basic->set(L"function", h_function);
      //basic->set(L"scope-freeze", h_scope_freeze);
      //basic->set(L"scope-freeze-forever", h_scope_freeze_forever);
      //scope-new
      basic->set(L"include", h_include);
      basic->set(L"exclude", h_exclude);
      basic->set(L"include-show", h_include_show);
    });
  }
} basic;

