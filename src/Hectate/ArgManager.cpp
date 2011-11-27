#include "Hectate/ArgManager.h"

#include "Hectate/Context.h"

//TODO: ArgumentError



ArgManager::ArgManager(Context &context, Atom &it_holder, const wchar_t *function)
  : context(context), it_holder(it_holder), function(function), is_done(false),
    buffer(ATOM_NONE), have_buffer(false) {
  it = dynamic_cast<Iterator*>(&it_holder.getAbstract());
  if (it == NULL) {
    throw HectateAbort(L"ArgManager::ArgManager(): it_holder is not an iterator");
  }
}

void ArgManager::update_function(const wchar_t *new_function) {
  function = new_function;
}



Atom ArgManager::require(const String &description, const String &note) {
  if (done()) {
    String msg = String() + L"ArgumentError: " + function + L" is missing an argument " + note + L": "
                 + description;
    throw HectateError(msg);
  }
  if (!done()) {
    get_raw();
  }
  if (context.action() != RESULT) {
    throw HectatePropagate();
  }
  return context.result();
}

Atom ArgManager::getQuote(const String &description) {
  return require(description, L"(quoted)");
}

Atom ArgManager::getEval(const String &description) {
  Atom value = require(description, L"(evaluated)");
  context.eval(value);
  return context.result();
}

Atom ArgManager::getCommand(const String &description) {
  Atom here = require(description, L"(command)");
  if (here.isType(SYMBOL)) {
    return here;
  }
  else {
    AtomType orig_type = here.getType();
    context.eval(here);
    if (context.result().isType(SYMBOL)) {
      return context.result();
    }
    else {
      throw HectateError(L"ArgumentError:" + String(function) + L" expected SYMBOL (got "
                         + type_name(orig_type) + L", which was evaluated to a "
                         + type_name(context.result().getType()) + L") for an argument (command): " + description);
    }
  }
}

Atom ArgManager::getWant(const String &description, AtomType type) {
  Atom here = require(description, L"(want " + type_name(type) + L")");
  if (here.isType(type)) {
    return here;
  }
  //have to convert...
  context.eval(here);
  if (!context.result().isType(type)) {
    if (type == STRING) {
      //we can recover!
      const String &s = context.result().toString(context);
      return Atom::create(new String(s));
    }
    throw HectateError(L"ArgumentError: " + String(function) + L" needs type of '" + type_name(type) + L"':" + description);
  }
  return context.result();
}


bool ArgManager::done() {
  fill_buffer();
  //if there is at least one more item available, then we aren't done
  return !have_buffer && is_done;
}

void ArgManager::beDone() {
  if (!done()) {
    throw HectateError(L"ArgumentError: " + String(function) + L" has extra arguments");
  }
}

Atom ArgManager::peek() {
  fill_buffer();
  if (done() || !have_buffer) {
    throw HectateError(L"ArgumentError: " + String(function) + L" peeked, but there's nothing");
  }
  return buffer;
}



void ArgManager::next() {
  auto save = context.saveFinish();
  get_raw();
}

void ArgManager::reset() {
  it->reset(context);
  is_done = false;
  have_buffer = false;
  buffer = ATOM_NONE;
}

Atom &ArgManager::getIterator() {
  return it_holder;
}

void ArgManager::it_seek() {
  it->seek(context, 1);
  /*
  if (context.hasAction(RESULT)) {
    std::wcerr << L"ArgManager::it_seek() got " << context.result().toString(context) << L"\n";
  }
  else {
    std::wcerr << L"ArgManager::it_seek(): !RESULT\n";
  }
  std::wcerr << L"\t\tfrom " << it_holder.toString(context) << L"\n";
  */
}


void ArgManager::get_raw() {
  if (have_buffer) {
    have_buffer = false;
    context.finish(buffer);
    buffer = ATOM_NONE;
    return;
  }
  it_seek();
  if (context.action() == BREAK) {
    is_done = true;
  }
  else {
    fill_buffer();
  }
}

void ArgManager::fill_buffer() {
  auto save = context.saveFinish();
  if (!have_buffer) {
    it_seek();
    if (context.action() == BREAK) {
      is_done = true;
    }
    else {
      have_buffer = true;
      buffer = context.result();
    }
  }
}


template<typename... Params>
void ArgManager::get(const Mode get_mode, Atom &store, const String &description, Params... rest) {
  if (done()) {
    throw HectateError(L"ArgumentError: " + String(function) + L" expected argument: " + description);
    //TODO: Add the rest
  }
  switch (get_mode) {
    case QUOTE:
      store = getQuote(description);
      break;
    case EVAL:
      store = getEval(description);
      break;
    case COMMAND:
      store = getCommand(description);
      break;
  }

  get(rest...);
}

void ArgManager::get() { } //base case






