
#include "Hectate/functions.h"


namespace Action {
  Symbol Sym_RESULT, Sym_RETURN, Sym_BREAK, Sym_REDO, Sym_ERROR;
  
  bool can_handle(EvalAction action) {
    //EvalActions that hectate code is allowed to interfere with
    switch (action) {
      case RESULT:
      case RETURN:
      case BREAK:
      case REDO:
      case ERROR:
        return true;
      default:
        return false;
    }
  }
  EvalAction convert(Symbol sym) {
    if (sym == Action::Sym_RESULT) return RESULT;
    if (sym == Action::Sym_RETURN) return RETURN;
    if (sym == Action::Sym_BREAK) return BREAK;
    if (sym == Action::Sym_REDO) return REDO;
    if (sym == Action::Sym_ERROR) return ERROR;
    throw HectateError(L"Action::convert(): Symbol is not an ACTION");
  }
  Symbol convert(EvalAction action) {
    switch (action) {
      case RESULT: return Sym_RESULT;
      case RETURN: return Sym_RETURN;
      case BREAK: return Sym_BREAK;
      case REDO: return Sym_REDO;
      case ERROR: return Sym_ERROR;
      default: throw HectateError(L"Action::convert(): action is invalid");
    }
  }

  void init() {
    Sym_RESULT = Symbol::create(L"RESULT");
    Sym_RETURN = Symbol::create(L"RETURN");
    Sym_BREAK = Symbol::create(L"BREAK");
    Sym_REDO = Symbol::create(L"REDO");
    Sym_ERROR = Symbol::create(L"ERROR");
  }
};




void h_do(Context &context) {
  //TODO: Store intermediate results in % in the local scope, and also save/restore the original value maybe?
  ArgManager arg = context.arg(L"{do}");
  context.finish();
  while (!arg.done()) {
    context.eval(arg.getQuote(L"{do WHAT_?}"), false);
    if (context.hasAction(RETURN)) {
      context.finish(RESULT, context.result());
      return;
    }
    context.propagate();
  }
}

void h_then(Context &context) {
  //like {do}, but don't catch RETURN.
  ArgManager arg = context.arg(L"{then}");
  context.finish();
  while (!arg.done()) {
    arg.getEval(L"{then WHAT_?}");
  }
}


void h_if(Context &context) {
  //(if |condition expression|_? else?)
  ArgManager arg = context.arg(L"(if)");
  context.finish();
  while (!arg.done()) {
    Atom condition = arg.getEval(L"(if |CONDITION evaluate|_? ELSE)");
    if (condition.convert(context, BOOLEAN).getBoolean()) {
      if (arg.done()) {
        //oh, this was actually an 'else'. Done.
        return;
      }
      arg.getEval(L"(if |condition EVALUATE|_? else)");
      return;
    }
    else {
      if (!arg.done()) {
        arg.next(); //skip!
      }
    }
  }
}

void h_while(Context &context) {
  //(while condition body)
  ArgManager arg = context.arg(L"(while)");
  Atom condition = arg.getQuote(L"(while CONDITION body)");
  Atom body = arg.getQuote(L"(while condition BODY)");
  while (1) {
    context.eval(condition);
    if (context.result().getBoolean() == false) {
      return;
    }
    context.eval(body, false);
    if (context.action() == BREAK) {
      context.finish(context.result());
      return;
    }
    context.propagate();
  }
}


void h_catch(Context &context) {
  //(catch to-eval |action-type handler|_?)
  ArgManager arg = context.arg(L"(catch)");
  Atom to_eval = arg.getQuote(L"(catch TO-EVAL)");
  auto action = context.action();
  auto result = context.result();
  if (!Action::can_handle(action)) {
    context.propagate();
    return;
  }

  //find something to handle this action
  while (!arg.done()) {
    Symbol sym = arg.getCommand(L"(catch to-eval |ACTION-TYPE handler|_?)").getSymbol();
    EvalAction here_action = Action::convert(sym);
    if (here_action == action) {
      arg.getEval(L"(catch to-eval |action-type HANDLER|_?)");
      return;
    }
    else {
      arg.next(); //skip over handler
    }
  }

  //else, there was no handler for it.
  context.finish(action, result);
}

void h_ignore(Context &context) {
  //[ignore action-type code]
  //TODO: Accept multiple action-types?
  ArgManager arg = context.arg(L"[ignore]");
  EvalAction to_ignore = Action::convert(arg.getCommand(L"[ignore ACTION-TYPE code]").getSymbol());
  Atom code = arg.getQuote(L"[ignore action-type CODE]");
  context.eval(code, false);
  if (context.action() == to_ignore) {
    context.finish();
    return;
  }
}

void h_result(Context &context) {
  ArgManager arg = context.arg(L"[result]");
  if (arg.done()) {
    context.finish();
  }
  else {
    Atom what = arg.getEval(L"[result WHAT?]");
    arg.beDone();
    context.finish(what);
  }
}

void h_return(Context &context) {
  ArgManager arg = context.arg(L"[return]");
  if (arg.done()) {
    context.finish(RETURN);
  }
  else {
    Atom what = arg.getEval(L"[return WHAT]");
    arg.beDone();
    context.finish(RETURN, what);
  }
}

void h_break(Context &context) {
  ArgManager arg = context.arg(L"[break]");
  if (arg.done()) {
    context.finish();
  }
  else {
    Atom what = arg.getEval(L"[break WHAT?]");
    arg.beDone();
    context.finish(BREAK, what);
  }
}

void h_error(Context &context) {
  ArgManager arg = context.arg(L"[error]");
  if (arg.done()) {
    throw HectateError(L"[error]");
  }
  else {
    Atom msg = arg.getEval(L"[error MESSAGE?]");
    arg.beDone();
    context.finish(ERROR, msg);
  }
}

class FlowLibrary : public FunctionLibrary {
public:
  FlowLibrary() {
    setLister([](Scope &root) {
      Action::init();
      Scope *lib = root.subscope(L"flow");
      
      //various MOTIONS
      lib->set(L"do", h_do);
      lib->set(L"then", h_then);
      lib->set(L"if", h_if);
      lib->set(L"while", h_while);

      //SPRING into ACTION
      lib->set(L"return", h_return);
      lib->set(L"result", h_result);
      lib->set(L"break", h_break);
      lib->set(L"error", h_error);

      //SPRING out of ACTION
      lib->set(L"catch", h_catch);
      lib->set(L"ignore", h_ignore);
    });
  }
} flow;