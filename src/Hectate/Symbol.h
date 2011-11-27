#ifndef SYMBOL_H
#define SYMBOL_H

#include "Hectate/Forwards.h"

class Symbol {
public:
  static Symbol create(unsigned int ID);
  static Symbol create(const wchar_t *name);
  static Symbol create(String name);
  String toString() const;
  bool operator==(const Symbol &right) const;
  bool operator<(const Symbol &right) const;
private:
  unsigned int id;
};

/*
#include <vector>
#include <utility>

class Path {
public:
  typedef std::pair<Scope &, Symbol> PathResult;
  PathResult lookup(Context &context) const; //Go through the scopes until we find the scope with our last item.
  String toString() const;
  void add(Symbol s);
private:
  std::vector<Symbol> items;
};
*/


#endif /* SYMBOL_H */

