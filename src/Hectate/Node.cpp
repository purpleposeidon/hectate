#include "Hectate/Node.h"

#include "Hectate/common.h"
#include "Hectate/Atom.h"
#include "words.h"
#include "Hectate/Context.h"
#include <iostream>

Node::Node() : Iterable() {
  close_bracket = open_bracket = L'\0';
}

Node::~Node() {
  while (length()) {
    pop();
  }
}

void Node::set(unsigned int index, Atom &a) {
  if (index >= length()) {
    throw HectateError(L"Node::set() index out of range");
  }
  if (items[index].isContainer()) {
    Atom victim = items[index];
    items[index] = ATOM_NONE;
    dropped(victim);
  }
  items[index] = properClaim(a);
}


void Node::pop() {
  Atom victim = items.back();
  items.pop_back();
  dropped(victim);
  disturb_iter();
}

void Node::push(Atom a) {
  items.push_back(properClaim(a));
}

void Node::push(Node *other) {
  for (items_type::iterator it = other->items.begin(); it != other->items.end(); it++) {
    push(*it);
  }
}

Atom &Node::getMod(unsigned int index) {
  //XXX: Can this be overloaded to be a non-const getMod?
  //XXX: What if the item is a container and gets destroyed?
  //I think, uh, "Don't do that"
  if (index >= length()) {
    throw HectateError(L"Node::get() index out of range");
  }
  return items[index];
}

const Atom &Node::get(unsigned int index) {
  return getMod(index);
}


Container::MapAction Node::map(Container::MapFunction function) {
  for (auto it = items.begin(); it != items.end(); it++) {
    if (function(*it) == Container::MapBreak) {
      return MapBreak;
    }
  }
  return MapContinue;
}

unsigned int Node::length() {
  return items.size();
}

String Node::toStringSimple(Context &context) {
  String s;
  s += L"[";
  bool tail = false;
  for (items_type::iterator it = items.begin(); it != items.end(); it++) {
    if (tail) {
      s += L" ";
    }
    tail = true;
    if (it->isType(NODE)) {
      s += it->getNode().toStringSimple(context);
    }
    else {
      s += it->toString(context);
    }
  }
  s += L"]";
  return s;
}

bool match(String to_match, wchar_t c) {
  return to_match.find(c) != std::string::npos;
}

String Node::toString(Context &context) {
  //TODO: Match, idiot!
  //TODO: Use Syntax:: items.
  items_type::iterator it = items.begin();
  String s;
  if (match(Syntax::format, open_bracket)) {
    if (items.size() == 0) {
      s += L"[]";
      return s;
    }
    s += open_bracket;
    
    if (it->isType(SYMBOL) && it->getSymbol() == Word::string_join) {
      s += L"\"";
    }
    else {
      s += it->toString(context);
      s += L":\"";
    }
    for (it++; it != items.end(); it++) {
      Atom a = *it;
      if (a.isType(STRING)) {
        //trim the ends
        String add = a.toString(context, true);
        s += add.substr(1, add.length() - 2);
      }
      else {
        s += open_bracket;
        s += a.toString(context);
      }
    }
    s += L'"';
  }
  else if (match(Syntax::join, open_bracket) || match(Syntax::call, open_bracket)) {
    if (match(Syntax::call, open_bracket)) {
      s += L'.';
    }
    bool first = true;
    unsigned int count = 0;
    for ( ; it != items.end(); it++) {
      count++;
      if (first) {
        first = false;
      }
      else {
        if (count == items.size() && items.back().isType(NODE) && items.back().getNode().open_bracket == L'.') {
          //it's a dot? Let it decide
        }
        else {
          s += L':';
        }
        //if (count == items.size() && items.back().isType(NODE) && items.back().getNode().open_bracket == L'.') //it's a dot?
      }
      s += it->toString(context);
    }
  }
  else if (match(Syntax::path_sep, open_bracket)) {
    if (it->isType(SYMBOL) && it->getSymbol() == Word::ScopeRoot) {
      //start with /
      s += open_bracket;
      it++;
    }
    bool first = true;
    for ( ; it != items.end(); it++) {
      if (first) {
        first = false;
      }
      else {
        s += open_bracket;
      }
      s += it->toString(context);
    }
  }
  else {
    if (open_bracket) {
      s += open_bracket;
    }
    bool first = true;
    for ( ; it != items.end(); it++) {
      if (first) {
        first = false;
      }
      else {
        if (it->isType(NODE) && match(Syntax::call, it->getNode().open_bracket)) {
          //do nothing!
        }
        else {
          s += L" ";
        }
      }
      s += it->toString(context);
    }
    if (close_bracket) {
      s += close_bracket;
    }
  }
  return s;
}

void Node::setBrackets(wchar_t open, wchar_t close) {
  open_bracket = open;
  close_bracket = close;
}

items_type::iterator Node::begin() {
  return items.begin();
}

items_type::iterator Node::end() {
  return items.end();
}

items_type::iterator Node::erase(items_type::iterator it) {
  disturb_iter();
  return items.erase(it);
}

items_type::iterator Node::insert(items_type::iterator it, Atom &value) {
  disturb_iter();
  return items.insert(it, value);
}

std::size_t Node::size() const {
  return items.size();
}








class NodeIterator : public Iterator {
public:
  NodeIterator(Node *src, Atom &atom) : src(src), atom_src(atom) {
    iter_src = src;
    index = -1;
    update_mark();
  }

  virtual void reset(Context& context) {
    index = -1;
  }
  
  virtual ~NodeIterator() { }

  virtual void seek(Context &context, int offset) {
    //we must result or break 'next'
    index += offset;
    if (index < 0) {
      context.finish(BREAK);
      index = -1;
    }
    else if (index >= len()) {
      context.finish(BREAK);
      index = len();
    }
    else {
      context.finish(src->get(index));
    }
  }

  virtual void getKey(Context &context) {
    context.finish(Atom::create<Number>(index));
  }

  virtual void getSource(Context& context) {
    context.finish(atom_src);
  }

  virtual Side getSide(Context &context) {
    if (len() == 0) {
      return EMPTY;
    }
    else if (len() == 1) {
      return SINGLE;
    }
    else if (index == 0) {
      return FIRST;
    }
    else if (index == len() - 1) {
      return LAST;
    }
    else {
      return BODY;
    }
  }
  
  virtual Iterator *clone(Context &context) {
    return new NodeIterator(src, atom_src);
  }

  
  //implementations for AbstractType functions
  virtual String toString(Context &context) {
    StringStream out;
    out << L"NodeIterator:";
    int i = 0;
    if (index == -1) {
      out << L" *BEGIN";
    }
    for (auto it = src->begin(); it != src->end(); it++) {
      out << L" ";
      if (i == index) {
        out << L"*";
      }
      out << it->toString(context);
      i++;
    }
    if (index >= (int)src->size()) {
      out << L" *END";
    }
    return out.str();
  }

private:
  Node *src;
  Atom atom_src;
  int index;

  int len() {
    return static_cast<int>(src->length());
  }
};

Atom Node::makeIterator(Atom &src) {
  if (&src.getNode() != this) {
    throw HectateError(L"Node::makeIterator: src is not us");
  }
  return Atom::create(new NodeIterator(this, src));
}


#include "Hectate/functions.h"

void h_node_iter(Context &context) {
  ArgManager arg = context.arg(L"[node/iter]");
  Atom node = arg.getEval(L"[node/iter NODE]");
  if (!node.isType(NODE)) {
    throw HectateError(L"[node/iter]: argument must be a node");
  }
  context.finish(node.getNode().makeIterator(node));
}

//TODO: Implement these functions (at least if they'll be at all useful...)
void h_node_get(Context &context) { throw HectateError(L"function not implemented"); }
void h_node_set(Context &context) { throw HectateError(L"function not implemented"); }
void h_node_append(Context &context) { throw HectateError(L"function not implemented"); }
void h_node_join(Context &context) { throw HectateError(L"function not implemented"); }

class NodeFunctions : public FunctionLibrary {
public:
  NodeFunctions() {
    setLister([](Scope &root) {
      Scope *node = root.subscope(L"node");
      node->set(L"iter", h_node_iter);
      node->set(L"get", h_node_get);
      node->set(L"set", h_node_set);
      node->set(L"append", h_node_append);
      node->set(L"join", h_node_join);
    });
  }
} node_functions;


