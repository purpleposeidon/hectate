//See also ContextEval.cpp

#include "Hectate/Context.h"

#include <cstring>
#include <cstdlib>

#include "Hectate/Scope.h"
#include "Hectate/config.h"
#include "Hectate/words.h"
#include "Hectate/Node.h"
#include "Atom.h"


const wchar_t *eval_action_name(EvalAction a) {
  const wchar_t *names[] = {
    L"NO ACTION",
    L"RESULT",
    L"RETURN",
    L"BREAK",
    L"REDO",
    L"ERROR",
    L"ABORT",
  };
  return names[a];
}

const wchar_t *eval_action_description(EvalAction a) {
  const wchar_t *descs[] = {
    L"no action set",
    L"evaluation result",
    L"[return] used to end a {do}",
    L"End of (while), (repeat), …",
    L"re-evaluate node",
    L"from [error-throw] or a builtin",
    L"uncatchable error",
  };
  return descs[a];
}


Context::Context(FiberManager *fm) : fiber_manager(fm) {
  pushScope();
  scope_stack.front().getContainer().makeRoot();
  finish();
}

Context::~Context() {
}



Atom &Context::scope_symbol(Symbol sym) {
  if (sym == Word::ScopeRoot) {
    return root_scope_atom;
  }

  if (sym == Word::ScopeSelf) {
    return scope_stack.back();
  }

  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    Scope &scope = it->getScope();
    if (scope.has(sym)) {
      return scope.get(sym);
    }
  }

  for (auto it = includes.begin(); it != includes.end(); ++it) {
    Scope &scope = it->getScope();
    if (scope.has(sym)) {
      return scope.get(sym);
    }
  }

  return root_scope_atom.getScope().get(sym);
}

bool Context::hasSymbol(Symbol sym) {
  if (sym == Word::ScopeRoot) {
    return true;
  }

  if (sym == Word::ScopeSelf) {
    return !scope_stack.empty();
  }

  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    if (it->getScope().has(sym)) {
      return true;
    }
  }

  for (auto it = includes.begin(); it != includes.end(); ++it) {
    if (it->getScope().has(sym)) {
      return true;
    }
  }

  return root_scope_atom.getScope().has(sym);
}

Atom &Context::getSymbol(Symbol sym) {
  return scope_symbol(sym);
}

void Context::updateSymbol(Symbol to_set, const Atom &value) {
  //XXX TODO: Could just use scope_symbol(to_set) = value; :P
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    Scope &scope = it->getScope();
    if (scope.has(to_set)) {
      scope.set(to_set, value);
      return;
    }
  }
  throw new HectateError(L"Context::updateSymbol(): symbol does not exist in any accessible scope");
}

void Context::setSymbol(Symbol to_set, const Atom &value) {
  scope_stack.back().getScope().set(to_set, value);
}

void Context::unsetSymbol(Symbol to_unset) {
  scope_stack.back().getScope().unset(to_unset);
}


bool Context::hasAction(EvalAction action) {
  return action == block_action;
}

Atom &Context::result() {
  return block_result;
}

EvalAction Context::action() {
  return block_action;
}

void Context::clear() {
  block_action = NO_ACTION;
  block_result = ATOM_NONE;
}

void Context::error(String err) {
  block_action = ERROR;
  block_result = Atom::create(new String(err));
}

void Context::error(const wchar_t* err) {
  block_action = ERROR;
  block_result = Atom::create(new String(err));
}

void Context::propagate() throw (HectatePropagate) {
  if (block_action != RESULT) {
    throw HectatePropagate();
  }
}








void Context::setRoot(Atom &atom) {
  if (!atom.isType(SCOPE)) {
    throw HectateError(L"Context::setRoot(): atom is not a Scope");
  }
  root_scope_atom = atom;
}



ArgManager &Context::arg(const wchar_t *function_name) {
  ArgManager *a = call_stack.back().get();
  a->update_function(function_name);
  return *a;
}

ArgManager *Context::argAt(unsigned int before) {
  if (before >= call_stack.size()) {
    return NULL;
  }
  auto it = call_stack.end() - 1;
  it -= before;
  return it->get();
}




void Context::push_call(Atom &iter) {
  if (call_stack.size() > Limit::eval_depth) {
    throw HectateError(L"Context::push_call(): Max eval depth exceeded");
  }
  if (dynamic_cast<Iterator*>(&iter.getAbstract()) == NULL) {
    throw HectateAbort(L"Context::push_call(): Argument is not an iterator");
  }
  auto p = std::shared_ptr<ArgManager>(new ArgManager(*this, iter, L"context evaluator"));
  call_stack.push_back(p);
}

void Context::pop_call() {
  if (call_stack.size() == 0) {
    throw HectateAbort(L"Context::pop_call(): empty stack");
  }
  call_stack.pop_back();
}

bool Context::isComplete() {
  return call_stack.size() == 0;
}





void Context::wait() {
  fiber_manager->yield();
}

void Context::run(Atom &to_eval) {
  eval(to_eval, false);
}




void Context::pushScope() {
  Atom pushing = Atom::create(new Scope);
  pushing.getContainer().makeRoot();
  scope_stack.push_back(pushing);
}

void Context::popScope() {
  scope_stack.pop_back();
}

void Context::need(AtomType t) {
  if (block_action != RESULT) {
    throw HectatePropagate();
  }
  need(t, block_result);
}

void Context::need(AtomType t, Atom& to_convert) {
  finish(to_convert.convert(*this, t));
}

Atom Context::deref(const Atom &orig) {
  if (orig.isType(SYMBOL)) {
    return getSymbol(orig.getSymbol()).unweaken();
  }
  return orig.unweaken();
}


bool Context::want(AtomType t) {
  if (block_action != RESULT) {
    throw HectatePropagate();
  }
  return want(t, block_result);
}

bool Context::want(AtomType t, Atom& to_convert) {
  try {
    finish(to_convert.convert(*this, t));
    return true;
  }
  catch (UnconvertibleType &e) {
    return false;
  }
}

void Context::finish(EvalAction action, Atom atom, bool force) {
  block_action = action;
  block_result = atom;
  if (block_action == NO_ACTION && !force) {
    throw HectateError(L"Context::finish() with NO_ACTION");
  }
}

Context::FinishSaver Context::saveFinish() {
  return FinishSaver(*this);
}


