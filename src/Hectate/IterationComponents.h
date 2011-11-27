
#ifndef ITERATIONCOMPONENTS_H
#define ITERATIONCOMPONENTS_H

#include "Hectate/Iteration.h"
#include "Hectate/Node.h"

class SingleIterator : public Iterator {
  //Just return a single item; iterates over a single atom
public:
  SingleIterator(Atom &item);
  static Atom create(Atom &item);
  
  virtual ~SingleIterator();
  virtual String toString(Context &context);

  virtual void seek(Context &context, int offset);
  virtual void reset(Context &context);
  virtual void getSource(Context &context);
  virtual Iterator *clone(Context &context);
  virtual Iterator::Side getSide(Context &context);

  virtual MapAction map(MapFunction func);

private:
  Atom item;
  int index;
};



class JoinIterator : public Iterator {
public:
  JoinIterator();
  void add(Atom &item);
  virtual bool extraEval(Context &context); //expose 'add'
  virtual String toString(Context &context);

  virtual void seek(Context &context, int offset);
  virtual Iterator::Side getSide(Context &context);
  virtual void reset(Context &context);

  virtual MapAction map(MapFunction func);

private:
  Node items;
  int index;

  Iterator *get_iter(Atom &a);
};



class ReheadIterator : public Iterator {
  //An iterator for nodes that replace the first item with something else.
public:
  ReheadIterator(Atom &head, Atom &rest_atom);
  virtual ~ReheadIterator();
  virtual String toString(Context &context);

  virtual void seek(Context &context, int offset);
  virtual Iterator::Side getSide(Context &context);
  virtual void reset(Context &context);

  virtual MapAction map(MapFunction func);

private:
  Atom head, rest_atom;
  int index;
  Iterator *rest;
  int get_length();
};

#endif /* ITERATIONCOMPONENTS_H */

