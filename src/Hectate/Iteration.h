
#ifndef ITERATION_H
#define ITERATION_H

typedef unsigned int MarkType;

class Iterator;
class Context;

class Iterable {
protected:
  Iterable() : iter_mark(0) {}
  void disturb_iter();
  
private:
  MarkType iter_mark; //Maybe make this atomic?
  friend class Iterator;
};

#include "Hectate/Forwards.h"
#include "Hectate/AbstractType.h"

class UncapableIteratorError; //TODO: Make this a HectateError
//this would be thrown when the data structure of an iterator does not support an operation.
//(Such as trying to seek backwards on a forward_list.)
//Would be put into the NI macro below, prolly

class Iterator : public AbstractType {
public:
  void eval(Context &context);
  virtual bool extraEval(Context &context) { return false; } //allows custom functionality for eval(). Return true if able to handle args
  
  virtual String toString(Context &context) = 0;
  virtual ~Iterator() {}
  
  virtual void seek(Context &context, int offset) = 0; //advance by offset and RESULT the value there (or BREAK). At minimum, implment with offset = 0 and 1.
#define NI(x) { throw HectateError(L"[iter " x L"]: Not implemented"); }
  virtual void reset(Context &context) NI(L"reset"); //seek to the begining
  virtual void getKey(Context &context) NI(L"key"); //key used to get what's pointed at. [it:source it:key] might get you .it
  virtual void getSource(Context &context) NI(L"source"); //RESULT some kind of source for the iterator
  virtual Iterator *clone(Context &context) NI(L"clone"); //RESULT a new iterator with the same state
  enum Side {LAST = -1, BODY = 0, FIRST = 1, SINGLE, EMPTY};
  virtual Side getSide(Context &context) NI(L"get-side");
#undef NI


protected:
  const Iterable *iter_src;
  MarkType iter_mark;
  void check_mark(); //make sure src->iter_mark == iter_mark
  void update_mark(); //let iter_mark = src->iter_mark
};



#endif /* ITERATION_H */

