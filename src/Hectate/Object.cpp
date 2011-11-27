#include "Hectate/Object.h"

#include "Hectate/words.h"
#include "Hectate/Context.h"
#include "Hectate/functions.h"

//TODO: Do arguments! We'll need an iterator! :D


void Object::eval(Context &context) {
  //call %CALL
  map_scopes([&](Scope *scope) {
    if (scope->has(Word::ObjectCall)) {
      context.eval(scope->get(Word::ObjectCall));
      return true;
    }
    return false;
  });
}

String Object::toString(Context &context) {
  String result;
  bool found_result = false;
  
  map_scopes([&](Scope *scope) {
    Atom r;
    
    //look for a %TO-STRING function
    if (scope->load(Word::ObjectToString, r)) {
      context.eval(r);
      result = context.result().convert(context, STRING).getString();
      found_result = true;
      return true;
    }
    
    //try `"`%NAME instance"
    if (scope->load(Word::ObjectClassName, r)) {
      result = r.convert(context, STRING).getString() + L" instance";
      found_result = true;
      return true;
    }
    return false;
  });
  if (found_result) {
    return result;
  }

  return String(L"<object>"); //some generic label
}

Container::MapAction Object::map(std::function<MapAction(Atom &)> function) {
  //Required for implementing the Container interface
  //just pass it onto the scope
  return contents.map(function);
}




template <typename F>
void Object::map_scopes(F func) {
  Scope *this_scope = &contents;
  while (1) {
    if (func(this_scope)) {
      break;
    }
    if (this_scope->has(Word::ObjectClass)) {
      this_scope = &(this_scope->get(Word::ObjectCall).getScope());
    }
    else {
      break;
    }
  }
}


//friend function defs
void h_object_new(Context &context) {
  //[object/new base_class args]
  ArgManager arg = context.arg(L"[object/new]");
  if (arg.done()) {
    //just [object/new]; so we don't need a parent class
    Atom obj = Atom::create(new Object);
    context.finish(obj);
  }
  else {
    //we've got at least a parent class, and possibly also optional arguments
    const Atom &a = arg.getEval(L"[object/new base-class]");
    Object *parent = NULL;
    if (!a.isType(OBJECT)) {
      context.eval(a);
      if (!context.result().isType(OBJECT)) {
        throw HectateError(L"[object/new base-class]: Argument is not a class object");
      }
      parent = &context.result().getObject();
    }
    else {
      parent = &a.getObject();
    }

    //parent needs to have %INIT
    Atom init_function;
    bool found_init = false;
    parent->map_scopes([&](Scope *scope) {
      found_init = scope->load(Word::ObjectInit, init_function);
      return found_init;
    });
    if (!found_init) {
      throw HectateError(L"[object/new]: class object does not have %INIT");
    }
    
    Atom obj = Atom::create(new Object);
    context.eval(init_function); //XXX TODO: call args properly
    context.finish(obj);
  }
}

void h_object_get(Context &context) {
  //[object/get object key]
  ArgManager arg = context.arg(L"[object/get]");
  Scope &cont = arg.getEval(L"[object/get OBJECT key]").getObject().contents;
  Symbol sym = arg.getCommand(L"[object/get object KEY]").getSymbol();
  context.finish(cont.get(sym));
}


void h_object_set(Context &context) {
  //[object/set object key value]
  ArgManager arg = context.arg(L"[object/set]");
  Scope &cont = arg.getEval(L"[object/set OBJECT key value]").getObject().contents;
  Symbol sym = arg.getCommand(L"[object/set object KEY value]").getSymbol();
  Atom val = arg.getEval(L"[object/set object key VALUE]");
  cont.set(sym, val);
  context.finish();
}


class ObjectLibrary : public FunctionLibrary {
public:
  ObjectLibrary() {
    setLister([=](Scope &root) {
      Scope *object = root.subscope(L"object");
      object->set(L"new", h_object_new);
      object->set(L"get", h_object_get);
      object->set(L"set", h_object_set);
    });
  }
} object_library;

