#include "Hectate/Iteration.h"

#include "Hectate/words.h"
#include "Hectate/Context.h"

#include <iostream>

void Iterable::disturb_iter() {
  iter_mark++;
}



void Iterator::check_mark() {
  if (iter_src && iter_mark != iter_src->iter_mark) {
    throw HectateError(L"iterator invalidated by change in source");
  }
}

void Iterator::update_mark() {
  if (iter_src) {
    iter_mark = iter_src->iter_mark;
  }
}



void Iterator::eval(Context &context) {
  context.clear();
  check_mark();
  if (extraEval(context)) {
    return;
  }
  Symbol sym;
  ArgManager arg = context.arg(L"Iterator::eval()");
  if (arg.done()) {
    sym = Word::IterNext;
  }
  else {
    sym = arg.getQuote(L"[iter command]").getSymbol();
  }
  //unfortunately, Word::items aren't constant enough to make a happy switch statement
  if (sym == Word::IterNext) {
    seek(context, 1);
  }
  else if (sym == Word::IterPrev) {
    seek(context, -1);
  }
  else if (sym == Word::IterValue) {
    seek(context, 0);
  }
  else if(sym == Word::IterKey) {
    getKey(context);
  }
  else if (sym == Word::IterSource) {
    getSource(context);
  }
  else if (sym == Word::IterSeek) {
    int offset = arg.getEval(L"[iter seek OFFSET]").getNumber();
    seek(context, offset);
  }
  else if (sym == Word::IterReset) {
    reset(context);
    context.finish();
  }
  else if (sym == Word::IterGetSide) {
    //TODO: Symbols or something, rather than a Number
    context.finish(Atom::create<Number>(getSide(context)));
  }
  else if (sym == Word::IterIsFirst) {
    auto s = getSide(context);
    context.finish(Atom::create<Boolean>(s == FIRST || s == SINGLE));
  }
  else if (sym == Word::IterIsBody) {
    auto s = getSide(context);
    context.finish(Atom::create<Boolean>(s == BODY));
  }
  else if (sym == Word::IterIsLast) {
    auto s = getSide(context);
    context.finish(Atom::create<Boolean>(s == LAST || s == SINGLE));
  }
  else if (sym == Word::IterClone) {
    Iterator *copy = clone(context);
    context.finish(Atom::create(copy));
  }
  else {
    throw HectateError(String(L"[iter]: Unknown function: ") + sym.toString());
  }
}


