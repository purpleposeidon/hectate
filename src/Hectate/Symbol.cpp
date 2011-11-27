#include "Hectate/Symbol.h"

#include <vector>
#include <map>
#include <wctype.h>
#include "Hectate/config.h"

typedef std::map<String, unsigned int> str2sym_type;
typedef std::vector<String> sym2str_type;
str2sym_type string2symbol; //a possible optimization: use a trie. (most likely not worthwhile.)
sym2str_type symbol2string;
//TODO: These'll prolly need locks some day?

void check_symbol_name(String s) {
  if (s.size() == 0) {
    throw HectateError(L"Tried to make token out of empty string");
  }
  for (int i = 0; s[i]; i++) {
    if (iswspace(s[i] || Syntax::invalid_symbol_chars.find(s[i]) != std::string::npos)) {
      throw HectateError(L"Invalid token");
    }
  }
}


Symbol Symbol::create(unsigned int ID) {
  Symbol ret;
  ret.id = ID;
  return ret;
}

Symbol Symbol::create(const wchar_t *name) {
  return create(String(name));
}

Symbol Symbol::create(String name) {
  str2sym_type::iterator it = string2symbol.find(name);
  if (it != string2symbol.end()) {
    return create(it->second);
  }
  else {
    check_symbol_name(name);
    unsigned int id = symbol2string.size();
    symbol2string.push_back(name);
    string2symbol[name] = id;
    return create(id);
  }
}

String Symbol::toString() const {
  return symbol2string[id];
}

bool Symbol::operator==(const Symbol &right) const {
  return id == right.id;
}

bool Symbol::operator<(const Symbol &right) const {
  return id < right.id;
}



//TODO: Enable
/*
#include "Hectate/words.h"
#include "Hectate/config.h"
#include "Context.h"

void Path::add(Symbol s) {
  items.push_back(s);
}

Path::PathResult Path::lookup(Context &context) const {
  Scope *here = &context.scope_stack.front();
  auto last = items.end() - 1;
  for (auto it = items.begin(); it != last; it++) {
    if (*it == Word::ScopeRoot) {
      here = &context.root_scope_atom.getScope();
    }
    else {
      
    }
  }
}

String Path::toString() const {
  String ret;
  for (auto it = items.begin(); it != items.end(); it++) {
    if (*it == Word::ScopeRoot) {
      ret += Syntax::path_sep[0];
    }
    else {
      ret += it->toString();
    }
  }
  return ret;
}
*/



