#include "Hectate/functions.h"

Number toNum(Context &context, const Atom &atom) {
  return atom.convert(context, NUMBER).getNumber();
}

Atom fromNum(Number n) {
  return Atom::create<Number>(n);
}

void h_add(Context &context) {
  ArgManager arg = context.arg(L"[+]");
  Number result = 0.0;
  while (!arg.done()) {
    result += toNum(context, arg.getEval(L"[+ NUMBER_?]"));
  }
  context.finish(fromNum(result));
}

void h_sub(Context &context) {
  ArgManager arg = context.arg(L"[-]");
  if (arg.done()) {
    context.finish(fromNum(-1.0));
    return;
  }
  Number result = toNum(context, arg.getEval(L"[- NUMBER_?]"));
  if (arg.done()) {
    context.finish(fromNum(-result));
    return;
  }
  
  while (!arg.done()) {
    result -= toNum(context, arg.getEval(L"[- NUMBER_?"));
  }
  
  context.finish(fromNum(result));
}

void h_mul(Context &context) {
  ArgManager arg = context.arg(L"[*]");
  Number result = 1.0;
  while (!arg.done()) {
    result *= toNum(context, arg.getEval(L"[* NUMBER_?]"));
  }
  context.finish(Atom::create<Number>(result));
}


void h_div(Context &context) {
  ArgManager arg = context.arg(L"[div]");
  if (arg.done()) {
    //[div]: return the golden ratio (This is Lojban's fault)
    auto phi = 1.6180339887498948482045868343656381177203091798057628621354; //and this is WolframAlpha's
    context.finish(fromNum(phi));
  }
  Number result = toNum(context, arg.getEval(L"[div NUMBER_?]"));
  if (arg.done()) {
    //[div a]: return 1/a
    context.finish(fromNum(1.0/result));
    return;
  }

  while (!arg.done()) {
    result /= toNum(context, arg.getEval(L"[div NUMBER_?"));
  }

  context.finish(fromNum(result));
}

void h_all(Context &context) {
  ArgManager arg = context.arg(L"[all]");
  while (!arg.done()) {
    if (arg.getEval(L"[all BOOLEAN_?]").convert(context, BOOLEAN).getBoolean() == false) {
      context.finish(Atom::create<Boolean>(false));
      return;
    }
  }
  context.finish(Atom::create<Boolean>(true));
}

void h_any(Context &context) {
  ArgManager arg = context.arg(L"[any]");
  while (!arg.done()) {
    if (arg.getEval(L"[any BOOLEAN_?]").convert(context, BOOLEAN).getBoolean() == true) {
      context.finish(Atom::create<Boolean>(true));
      return;
    }
  }
  context.finish(Atom::create<Boolean>(false));
}

void h_numeric_equal(Context &context) {
  ArgManager arg = context.arg(L"[/math/=]");
  Atom val = arg.getEval(L"[/math/= VAL val_]");;
  context.finish(Atom::create<Boolean>(true));
  
  if (val.isType(BOOLEAN)) {
    Boolean first = val.getBoolean();
    while (!arg.done()) {
      Boolean cmp = arg.getEval(L"[/math/= val VAL_]").getBoolean();
      if (cmp != first) {
        context.finish(Atom::create<Boolean>(false));
        return;
      }
    }
    context.finish(Atom::create<Boolean>(true));
  }
  else if (val.isType(NUMBER)) {
    Number first = val.getNumber();
    while (!arg.done()) {
      Number cmp = arg.getEval(L"[/math/= val VAL_]").getNumber();
      if (cmp != first) {
        context.finish(Atom::create<Boolean>(false));
        return;
      }
    }
    context.finish(Atom::create<Boolean>(true));
  }
  else {
    throw HectateError(L"[/math/=]: Can only compare Booleans or Numbers");
  }
}

class MathLibrary : public FunctionLibrary {
public:
  MathLibrary() {
    setLister([](Scope &root) {
      Scope &math = *root.subscope(L"math");
      //cannonical names
      math.set(L"+", h_add);
      math.set(L"-", h_sub);
      math.set(L"*", h_mul);
      math.set(L"div", h_div);
      math.set(L"all", h_all);
      math.set(L"any", h_any);
      math.set(L"=", h_numeric_equal);

      //english variants
      math.set(L"sum", h_add);
      math.set(L"difference", h_sub);
      math.set(L"product", h_mul);
      math.set(L"quotient", h_div);
      
      //fancy unicode stuff
      math.set(L"∑", h_add); //need the product thing too, Π or something
      math.set(L"÷", h_div);
      math.set(L"×", h_mul);
    });
    //XXX: Need [∀ domain [function domain-element]] and [∃ domain [function domain-element]]
  }
} math;

