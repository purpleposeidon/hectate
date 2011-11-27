#include "Hectate/Scope.h"
#include "Hectate/Types.h"
#include "Hectate/words.h"

#include <sstream>
#include <assert.h>
#include <queue>


Scope::Scope() {
}

Scope::~Scope() {
  if (isFrozen()) {
    freeze(false);
  }
  clear();
}

bool Scope::operator==(const Scope& right) const {
  return &contents == &right.contents;
}


void Scope::setSelf(Atom &atom) {
  assert(atom.isType(SCOPE));
  assert(&atom.getScope() == this);
  set(Word::ScopeSelf, atom); //This ought to work just fine? Should be converted into a weakref
  //set(Word::ScopeSelf, atom.makeWeakref()); //this is the nice way to do it
}





void Scope::set(Symbol symbol, Atom value) {
  if (isFrozen()) {
    throw HectateError(L"Scope::set(): Scope is frozen");
  }
  auto lock = getLock();
  auto orig = contents.find(symbol);
  if (orig != contents.end()) {
    dropped(orig->second);
    contents.erase(orig);
  }

  contents[symbol] = properClaim(value);
}

void Scope::set(String name, Atom value) {
  set(Symbol::create(name), value);
}



void Scope::set(const wchar_t *name, HectateBuiltin value) {
  Atom a = Atom::create(value);
  set(Symbol::create(name), a);
}

Atom &scope_get(Scope &scope, Symbol sym) {
  //XXX for gdb, which is a piece of crap
  return scope.get(sym);
}

Atom &Scope::get(Symbol symbol) {
  ScopeMap::iterator sym = contents.find(symbol);
  if (sym == contents.end()) {
    throw SymbolNotFound(symbol);
  }
  return sym->second;
}

Atom &Scope::get(const wchar_t *name) {
  return get(Symbol::create(name));
}

bool Scope::load(Symbol symbol, Atom &dest) {
  auto sym = contents.find(symbol);
  if (sym == contents.end()) {
    return false;
  }
  dest = sym->second;
  return true;
}


bool Scope::has(const wchar_t *name) {
  return has(Symbol::create(name));
}

bool Scope::has(Symbol symbol) {
  return contents.find(symbol) != contents.end();
}

Scope *Scope::subscope(const wchar_t *name) {
  if (isFrozen()) {
    throw HectateAbort(L"Scope::subscope(): Scope is frozen");
  }
  Symbol scope_symbol = Symbol::create(name);
  if (has(scope_symbol)) {
    return &get(scope_symbol).getScope();
  }
  else {
    Atom sub = Atom::create(new Scope);
    /*Atom sub;
    sub.set(new Scope);*/
    Scope &scope = sub.getScope();
    set(name, sub);
    Atom a = Atom::create(new String(name));
    scope.set(Word::ScopeName, a);
    if (!has(Word::ScopeSelf)) {
      throw HectateError(L"Scope::subscope(): Parent scope doesn't have a Word::ScopeSelf");
    }
    scope.set(Word::ScopeParent, get(Word::ScopeSelf));
    return &scope;
  }
}

bool Scope::unset(Symbol sym) {
  if (isFrozen()) {
    throw HectateError(L"Scope::unset(): Scope is frozen");
  }
  return contents.erase(sym);
}

void Scope::clear() {
  if (isFrozen()) {
    throw HectateError(L"Scope::clear(): Scope is frozen");
  }
  for (auto it = contents.begin(); it != contents.end(); it++) {
    dropped(it->second);
  }
  contents.clear();
}






ScopeMap::iterator Scope::begin() { return contents.begin(); }
ScopeMap::iterator Scope::end() { return contents.end(); }

Container::MapAction Scope::map(Container::MapFunction function) {
  for (auto it = begin(); it != end(); it++) {
    if (function(it->second) == Container::MapBreak) {
      return MapBreak;
    }
  }
  return MapContinue;
}


String Scope::toString(Context &context) {
  StringStream out;

  //Header
  out << L"Scope";
  if (has(Word::ScopeName)) {
    out << " " << get(Word::ScopeName).toString(context);
  }
  bool r = has(Word::ScopeStopRead), w = has(Word::ScopeStopWrite);
  if (r && w && !isFrozen()) {
    out << L", unreadable and unwritable by child scopes";
  }
  else if (r) {
    out << L", unreadable by child scopes";
  }
  else if (w && !isFrozen()) {
    out << L", unwritable by child scopes";
  }
  if (isFrozen()) {
    out << L", unwritable";
  }
  out << L":" << std::endl;

  unsigned int max_left = 20;
  typedef std::pair<String, String> pair;
  std::queue<pair> output;
  for (ScopeMap::iterator it = contents.begin(); it != contents.end(); it++) {
    String left = it->first.toString();
    String right;
    right = it->second.toStringRepr(context);
    output.push(pair(left, right));
    if (left.length() > max_left) {
      max_left = left.length();
    }
  }
  //Contents
  for (ScopeMap::iterator it = contents.begin(); it != contents.end(); it++) {
    Symbol s = it->first;
    Atom a = it->second;
    auto init_width = out.width();
    out << L" ";
    out.width(max_left);
    out << s.toString() << L": ";
    out.width(init_width);
    
    if (a.isType(SCOPE)) {
      out << L"<scope>";
    }
    else {
      //XXX TODO: For consideration: Hide symbols beginning with % (and give dir-% or dir-all something.)
      out << a.toStringRepr(context);
    }
    out << std::endl;
  }
  return out.str();
}






#include <iostream>
SymbolNotFound::SymbolNotFound (Symbol s) : sym(s), made(false) {
  std::wcout << L"SymbolNotFound: " << s.toString() << std::endl;
}

String& SymbolNotFound::get_msg() {
  if (!made) {
    m = String(L"Symbol '") + sym.toString() + String(L"' not found in any available scope");
    made = true;
  }
  return m;
}













