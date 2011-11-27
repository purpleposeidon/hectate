
#ifndef ARGMANAGER_H
#define ARGMANAGER_H

#include "Hectate/Forwards.h"
#include "Hectate/Atom.h"
//#include "Hectate/Context.h"
//#include "Hectate/Iteration.h"


enum Mode {
  QUOTE, EVAL, COMMAND
};
#include <iostream>
class ArgManager {
public:
  ArgManager(Context &context, Atom &it_holder, const wchar_t *function);
  void update_function(const wchar_t *new_function);

  Atom getQuote(const String &description); //Returns the argument exactly as given. [quote [print 'stuff']]
  Atom getEval(const String &description); //Runs the argument and returns the result
  Atom getCommand(const String &description); //Expects a symbol. ['silly string' length]. Could also have ['ss' [function-selector]]
  Atom getWant(const String &description, AtomType type);

  Atom peek();

  void get(); //base case
  template<typename... Params>
  void get(const Mode get_mode, Atom &store, const String &description, Params... rest);

  void next(); //skip an argument
  void reset(); //return to arg0

  bool done(); //return true if there are no more items left
  void beDone(); //create an error if not done.
  Atom &getIterator();
  
private:
  Context &context;
  Iterator *it;
  Atom it_holder;
  const wchar_t *function;
  bool is_done;
  Atom buffer;
  bool have_buffer;
  void get_raw();
  void fill_buffer();
  void it_seek();
  Atom require(const String &description, const String &note);
};

/*
 * [quote] missing an argument: item to quote
 * 
 *
 * */

  

#endif /* ARGMANAGER_H */

