#ifndef SCOPE_H
#define SCOPE_H
#include "Hectate/common.h"
#include "Hectate/Atom.h"
#include "Hectate/Container.h"

#include <map>
typedef std::map<Symbol, Atom> ScopeMap;


class Scope : public Container {
public:
  Scope();
  ~Scope();
  Scope &operator=(Scope &) const = delete;
  Scope(const Scope &) = delete;
  bool operator==(const Scope &right) const;
  void setSelf(Atom &atom); //'atom' refers to us; create an entry for that atom
  
  void set(String name, Atom value);
  void set(Symbol symbol, Atom value);
  void set(const wchar_t *name, HectateBuiltin function);
  Atom &get(const wchar_t *name);
  Atom &get(Symbol symbol);
  bool load(Symbol symbol, Atom &dest); //Set dest, return true if we have 'symbol' //XXX TODO: Use this instead of has && get!
  bool has(const wchar_t *name);
  bool has(Symbol symbol);
  bool unset(Symbol symbol);
  void clear();
  
  Scope *subscope(const wchar_t *name = NULL);
  String toString(Context &context);
  
  ScopeMap::iterator begin();
  ScopeMap::iterator end();
  virtual MapAction map(Container::MapFunction function);
  
private:
  ScopeMap contents;
  
  void lock(); //TODO: Put in a mutex
  void unlock();
};




class SymbolNotFound : public HectateError {
public:
  SymbolNotFound(Symbol s);
  ~SymbolNotFound() throw () {}
  virtual String &get_msg();
private:
  Symbol sym;
  String m;
  bool made;
};

#endif /* SCOPE_H */

