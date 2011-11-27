#include "Hectate/IterationComponents.h"

#include "Hectate/words.h"
#include "Hectate/Context.h"


//SingleIterator definitions

SingleIterator::SingleIterator(Atom &item) : item(properClaim(item)), index(-1) { }

SingleIterator::~SingleIterator() {
  dropped(item);
}


String SingleIterator::toString(Context &context) {
  return String(L"SingleIterator " + item.toString(context));
}

Atom SingleIterator::create(Atom &item) {
  return Atom::create(new SingleIterator(item));
}


Iterator* SingleIterator::clone(Context &context) {
  auto r = new SingleIterator(item);
  r->index = index;
  return r;
}

Iterator::Side SingleIterator::getSide(Context &context) {
  return SINGLE;
}

void SingleIterator::getSource(Context &context) {
  context.finish(item);
}

void SingleIterator::reset(Context &context) {
  index = -1;
}

void SingleIterator::seek(Context &context, int offset) {
  index += offset;
  if (offset == 0) {
    context.finish(item);
  }
  else {
    context.finish(BREAK);
  }
  index = index < 0 ? -1 : (index > 0 ? 1 : 0);
}

Container::MapAction SingleIterator::map(MapFunction func) {
  return func(item);
}



////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//JoinIterator definitions

JoinIterator::JoinIterator() : index(0) {
  items.setBrackets(L'<', L'>');
}

void JoinIterator::reset(Context &context) {
  index = 0;
  for (auto it = items.begin(); it != items.end(); it++) {
    get_iter(*it)->reset(context);
  }
  //..and hope that this iterator doesn't refer to itself!
}

void JoinIterator::add(Atom &item) {
  Iterator *i = dynamic_cast<Iterator *>(&item.getAbstract());
  if (i == NULL) {
    throw HectateError(L"[iter-joiner add]: Item is not an iterator");
  }
  items.push(item);
}


bool JoinIterator::extraEval(Context& context) {
  ArgManager arg = context.arg(L"JoinIterator::extraEval");
  if (arg.done()) {
    return false;
  }
  const Atom &a = arg.getQuote(L"[iter command]");
  if (!a.isType(SYMBOL)) {
    return false;
  }
  const Symbol sym = a.getSymbol();
  if (sym == Word::IterJoinerAdd) {
    Atom item = arg.getEval(L"[iter-joiner add ITEM]");
    add(item);
    return true;
  }
  return false;
}

String JoinIterator::toString(Context &context) {
  StringStream out;
  out << L"JoinIterator:";
  int i = 0;
  for (auto it = items.begin(); it != items.end(); it++) {
    out << L" <";
    if (i == index) {
      out << L"#";
    }
    out << it->toString(context) << L">";
    i++;
  }
  return out.str();
}

Iterator *JoinIterator::get_iter(Atom &a) {
  //gets the iterator stored in atom
  Iterator *ret = dynamic_cast<Iterator*>(&a.getAbstract());
  if (ret == NULL) {
    throw HectateError(L"JoinIterator::here(): Item is supposed to be an Iterator! How did it change?");
  }
  return ret;
}


void JoinIterator::seek(Context &context, int offset) {
  if (index >= (int)items.size()) {
    //since we can't go backwards, we're stuck here forever. FOREVER!
    context.finish(BREAK);
    index = items.size();
    return;
  }
  if (index < 0) {
    context.finish(BREAK);
    index = -1;
    return;
  }
  
  if (offset == 0) {
    get_iter(items.getMod(index))->seek(context, 0);
  }
  else if (offset == 1) {
    get_iter(items.getMod(index))->seek(context, 1);
  }
  else {
    throw HectateError(L"[iter-join seek]: Can only seek by 0 or 1");
  }
  if (context.action() == BREAK) {
    index++;
    seek(context, 0);
    //hopefully no recursion depth!
    //The top of this function should handle hitting the end
  }
}
  
Iterator::Side JoinIterator::getSide(Context &context) {
  Iterator::Side super_side = BODY;
  if (index == 0) {
    super_side = FIRST;
  }
  else if (index == (int)items.size() - 1) {
    super_side = LAST;
  }
  if (items.size() == 1) {
    super_side = SINGLE;
  }
  if (super_side == BODY) {
    //we don't need to ask
    return BODY;
  }
  
  Iterator::Side side = get_iter(items.getMod(index))->getSide(context);
  //check for liars
  if (super_side == FIRST && (side == SINGLE || side == LAST)) {
    return BODY;
  }
  if (super_side == LAST && (side == SINGLE || side == FIRST)) {
    return BODY;
  }
  return side;
}


Container::MapAction JoinIterator::map(MapFunction func) {
  for (auto it = items.begin(); it != items.end(); it++) {
    if (get_iter(*it)->map(func) == MapAction::MapBreak) {
      return MapBreak;
    }
  }
  return MapContinue;
}





ReheadIterator::ReheadIterator(Atom &_head, Atom &_rest_atom) : index(-1), rest(NULL) {
  head = properClaim(_head);
  rest_atom = properClaim(_rest_atom);
  rest = static_cast<Iterator*>(&rest_atom.getAbstract());
  if (rest == NULL) {
    throw HectateError(L"ReheadIterator: rest_atom is not an iterator");
  }
}

ReheadIterator::~ReheadIterator() {
  dropped(head);
  dropped(rest_atom);
}


String ReheadIterator::toString(Context &context) {
  StringStream out;
  out << L"ReheadIterator over " << head.toString(context) << L" " << rest->toString(context);
  return out.str();
}

Container::MapAction ReheadIterator::map(MapFunction func) {
  if (func(head) == Container::MapBreak) {
    return MapBreak;
  }
  if (rest) {
    return rest->map(func);
  }
  return MapContinue;
}


void ReheadIterator::seek(Context &context, int offset) {
  if (!(offset == 0 || offset == 1)) {
    throw HectateError(L"ReheadIterator::seek(): offset must be 0 or 1");
  }
  index += offset;
  if (index < 0) {
    context.finish(BREAK);
  }
  else if (index == 0) {
    context.finish(head);
    /*{
      auto save = context.saveFinish();
      rest->seek(context, offset);
    }*/
  }
  else {
    rest->seek(context, offset);
  }
}

Iterator::Side ReheadIterator::getSide(Context &context) {
  if (index <= 0) {
    return FIRST;
  }
  auto s = rest->getSide(context);
  switch (s) {
    case FIRST: return BODY;
    case SINGLE: return LAST;
    case EMPTY: return LAST;
    default: return s;
  }
}

void ReheadIterator::reset(Context &context) {
  index = -1;
  rest->reset(context);
}




