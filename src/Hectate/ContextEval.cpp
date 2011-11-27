//More definitions for things in Context.h; but this stuff is related to evaluation

#include "Hectate/Context.h"
#include "Hectate/Types.h"
#include "Hectate/Scope.h"
#include "Hectate/words.h"
#include "Hectate/Object.h"
#include "Hectate/Iteration.h"
#include "Hectate/ArgManager.h"
#include "Hectate/IterationComponents.h"

#include <iostream>
#include <sstream>
#include <memory>

void eval_iter_node(Context &, Atom &arg0);
void eval_iter_string(Context &, Atom &arg0);
void eval_iter_scope(Context& context, Atom& arg0);
void eval_iter_generic(Context &, Atom &arg0);


template <class EvalType>
void Context::eval(EvalType what, bool throw_action) {
  evals++;
  if (throw_action) {
    //errors will interupt the caller
    eval_body(what);
    if (action() != RESULT) {
      throw HectatePropagate();
    }
  }
  else {
    //the caller will check for errors
    try {
      eval_body(what);
    }
    catch (HectatePropagate &e) {
      //Do nothing; some parent will take note of action()
      if (action() == RESULT) {
        std::wcerr << bold(L"NOTE: ") << L"HectatePropagate thrown, but we've got a RESULT" << std::endl;
      }
    }
    catch (HectateError &e) {
      //make it our error
      finish(ERROR, Atom::create(new String(e.get_msg())));
    }
  }
}

#include "Hectate/gdb_sucks.h"
void here_break() {}
void Context::eval_body(Atom &atom) {
  switch (atom.getType()) {
    case SYMBOL:
    case WEAKREF:
      finish(deref(atom));
      break;
      
    case NODE:
    {
      Atom bod = atom.getNode().makeIterator(atom);
      eval_body_iter(bod);
      break;
    }
      
    default:
      //return the given value
      finish(atom);
      break;
  }
}

void print_iter(Context &context, Iterator *iter) {
  std::wcout << L"Iterator that would be evaluated it if weren't for being printed:\n";
  while (1) {
    iter->seek(context, 1);
    if (context.action() != RESULT) {
      break;
    }
    std::wcout << context.result().toString(context) << L"\n → ";
  }
  std::wcout << L"\n";
}

struct ContextIterCall {
  ContextIterCall(Context *context, Atom &iter_src) : context(context) {
    //Start with a push
    //print_iter(*context, static_cast<Iterator*>(&iter_src.getAbstract()));
    //static_cast<Iterator*>(&iter_src.getAbstract())->reset(*context);
    context->push_call(iter_src);
  }
  ~ContextIterCall() {
    //cleanup with a pop
    context->pop_call();
  }
  Context *context;
};


void Context::eval_body_iter(Atom &iter_src) {
  //pick the right function to call for the type of arg0

  ContextIterCall rai(this, iter_src);

  ArgManager &arg = *call_stack.back();
  if (arg.done()) {
    return;
  }
  Atom arg0 = deref(arg.getQuote(L"expression function to evaluate")); //deref(argHead());
  
  
  switch (arg0.getType()) {
    case BUILTIN:
      arg0.getBuiltin()(*this);
      break;

    case OBJECT:
      arg0.getObject().eval(*this);
      break;

    case ABSTRACT:
      arg0.getAbstract().eval(*this);
      break;

      
    //We can handle these guys in this file
    case NODE:
      eval_iter_node(*this, arg0);
      break;

    case STRING:
      eval_iter_string(*this, arg0);
      break;

    case SCOPE:
      eval_iter_scope(*this, arg0);
      break;

    default:
      eval_iter_generic(*this, arg0);
      break;
  }
}


void eval_iter_node(Context &context, Atom &arg0) {
  //A node with a nested node as the first argument.
  //[[quote +] 2 3]
  //[pick-some-function arg1 arg2]
  
  //Create a new call, [[eval arg0] iter-args...]
  context.eval(arg0);
  auto rh = new ReheadIterator(context.result(), context.argAt(0)->getIterator());
  Atom to_eval = Atom::create(rh);
  context.eval_body_iter(to_eval);
}


void eval_iter_string(Context &context, Atom &arg0) {
  /*
   * [string] commands.
   * A little DSL. We've got "selectors" and "operators".
   * An operator uses the range defined by the selectors.
   * 
   * selectors:
   *    all, at INDEX, slice START END
   *    cut CUT - (if CUT > 0, "python"[0:CUT], else "python"[CUT:])
   *    find SEARCH - (uses the range),
   *    find-regexp EXPR
   *    before, after - change range from (l,r) to (0,l)
   *    shift, shift-start, shift-end
   *
   * operators:
   *    update FUNCTION - run FUNCTION with range contents, replace range with result
   *    transform TRANS-SYMBOL - upper, lower, title, inverse
   *    crop, delete
   *    replace REPLACEMENT
   *
   * other:
   *    range-save LEFT RIGHT, range-load LEFT RIGHT
   *    
   * */
  ArgManager arg = context.arg(L"[string]");
  String &s = arg0.getString();
  String::iterator left = s.begin(), right = s.end();
  auto wrap = [&](int l) {
    auto s_len = s.length();
    if (l < 0) {
      return (s_len - abs(l)) % s_len;
    }
    return l % s_len;
  };
  auto clip = [&](String::iterator &side) {
    if (side < s.begin()) {
      side = s.begin();
    }
    if (side > s.end()) {
      side = s.end();
    }
  }; //keep iterator between begin and end
  auto replace = [&](String repl) {
    String::size_type l_offset = left - s.begin();
    s = s.replace(left, right, repl);
    left = s.begin() + l_offset;
    right = left + repl.length();
  };
  bool was_op = false; //If the last command is an operator, select all
  while (!arg.done()) {
    const Symbol sym = arg.getCommand(L"[string command]").getSymbol();
    was_op = false;
    //selectors
    if (sym == Word::StringAll) {
      left = s.begin();
      right = s.end();
    }
    else if (sym == Word::StringAt) {
      int n = arg.getEval(L"[string at INDEX]").getNumber();
      left = s.begin();
      left += wrap(n);
      right = left;
      right++;
    }
    else if (sym == Word::StringSlice) {
      int l = arg.getEval(L"[string slice LEFT right]").getNumber();
      int r = arg.getEval(L"[string slice left RIGHT]").getNumber();
      left = s.begin() + wrap(l);
      right = s.begin() + wrap(r);
    }
    else if (sym == Word::StringCut) {
      int cut = arg.getEval(L"[string cut WHERE]").getNumber();
      //TODO: Test edges. And also the middle.
      if (cut > 0) {
        //[0, cut)
        left = s.begin();
        right = left + cut;
      }
      else {
        //[cut, end)
        right = s.end();
        left = right + cut;
      }
    }
    else if (sym == Word::StringFind) {
      String target = arg.getEval(L"[string find WHAT]").getString();
      String::size_type start = left - s.begin();
      auto where = s.find(target, start);
      if (where == String::npos) {
        throw HectateError(L"[string find]: not found");
      }
      left = s.begin() + where;
      right = left + target.size();
    }
    else if (sym == Word::StringBefore) {
      right = left;
      left = s.begin();
    }
    else if (sym == Word::StringAfter) {
      left = right;
      right = s.end();
    }
    else if (sym == Word::StringShift) {
      //add N to each side
      int n = arg.getEval(L"[string shift AMOUNT]").getNumber();
      left += n;
      right += n;
      clip(left);
      clip(right);
    }
    else if (sym == Word::StringShiftStart) {
      int n = arg.getEval(L"[string shift-start AMOUNT]").getNumber();
      left += n;
      clip(left);
    }
    else if (sym == Word::StringShiftEnd) {
      int n = arg.getEval(L"[string shift-end AMOUNT]").getNumber();
      right += n;
      clip(right);
    }
    else if (sym == Word::StringNoOp) {
    }
    //operators
    else if (sym == Word::StringUpdate) {
      {
        std::unique_ptr<Node> n(new Node);
        Atom arg0 = arg.getQuote(L"[string update TO-CALL]");
        n->push(arg0);
        n->push(Atom::create(new String(left, right)));
        Atom node = Atom::create(n.release());
        context.eval(node);
      }
      replace(context.result().getString());
      was_op = true;     
    }
    else if (sym == Word::StringReplace) {
      replace(arg.getEval(L"[string replace WHAT]").getString());
      was_op = true;
    }
    else if (sym == Word::StringDelete) {
      replace(String(L""));
      was_op = true;
    }
    else if (sym == Word::StringCrop) {
      s = String(left, right);
      left = s.begin();
      right = s.end();
    }
    //other ops
    else if (sym == Word::StringLength) {
      context.finish(Atom::create<Number>(right - left));
      return;
    }
    else if (sym == Word::StringHas) {
      String target = arg.getEval(L"[string has WHAT]").getString();
      Boolean ret = s.find(target) != String::npos;
      context.finish(Atom::create<Boolean>(ret));
      return;
    }
    else {
      throw HectateError(String(L"[string]: Unknown command: ") + sym.toString());
      //TODO: Implement everybody else
    }
  }
  if (was_op) {
    left = s.begin();
    right = s.end();
  }
  context.finish(Atom::create(new String(left, right)));
}


void eval_iter_scope(Context &context, Atom &arg0) {
  /*
  arg.reset();
  Atom scope_atom = context.getSymbol(arg.getWant(L"eval_iter_scope()", SYMBOL).getSymbol());
  */
  ArgManager arg = context.arg(L"eval_iter_scope()");

  
  Atom scope_atom = arg0;
  Scope *here = &scope_atom.getScope();
  if (arg.done()) {
    context.finish(scope_atom);
    return;
  }
  while (1) {
    Symbol sub_scope = arg.getWant(L"eval_iter_scope", SYMBOL).getSymbol();
    scope_atom = here->get(sub_scope);
    if (arg.done()) {
      //scope_atom doesn't have to actually be a scope then; it can be the scope's contents
      context.finish(scope_atom);
      return;
    }
    if (!scope_atom.isType(SCOPE)) {
      throw HectateError(L"scope path: Sub component isn't a scope");
    }
    here = &scope_atom.getScope();
  }
}


void eval_iter_generic(Context &context, Atom &arg0) {
  if (!context.arg(L"eval_node_generic").done()) {
    throw HectateError(arg0.toStringRepr(context) + String(L" can't take an argument"));
  }
  context.finish(arg0);
}











