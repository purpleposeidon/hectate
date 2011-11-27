
// ***************************************************
// This file was created automatically by genwords.py.
// Edit that script rather than this file.
// ***************************************************
 
#include "Hectate/words.h"

namespace Word {
  Symbol ScopeSelf;
  Symbol ScopeParent;
  Symbol ScopeName;
  Symbol ScopeRoot;
  Symbol ScopeStopRead;
  Symbol ScopeStopWrite;
  Symbol Arguments;
  Symbol LoadEval;
  Symbol LoadQuote;
  Symbol ObjectCall;
  Symbol ObjectToString;
  Symbol ObjectInit;
  Symbol ObjectDelete;
  Symbol ObjectClass;
  Symbol ObjectClassName;
  Symbol IterNext;
  Symbol IterPrev;
  Symbol IterValue;
  Symbol IterKey;
  Symbol IterSource;
  Symbol IterSeek;
  Symbol IterReset;
  Symbol IterGetSide;
  Symbol IterIsFirst;
  Symbol IterIsBody;
  Symbol IterIsLast;
  Symbol IterClone;
  Symbol IterJoinerAdd;
  Symbol StringAll;
  Symbol StringAt;
  Symbol StringSlice;
  Symbol StringCut;
  Symbol StringFind;
  Symbol StringFindRegexp;
  Symbol StringBefore;
  Symbol StringAfter;
  Symbol StringShift;
  Symbol StringShiftStart;
  Symbol StringShiftEnd;
  Symbol StringNoOp;
  Symbol StringUpdate;
  Symbol StringTransformUp;
  Symbol StringTransformDown;
  Symbol StringTransformTitle;
  Symbol StringTransformInverse;
  Symbol StringCrop;
  Symbol StringDelete;
  Symbol StringReplace;
  Symbol StringAppend;
  Symbol StringPrepend;
  Symbol StringLength;
  Symbol StringHas;
  Symbol StringCount;
  Symbol StringRangeSave;
  Symbol StringRangeLoad;
  Symbol string_join;

  void define() {
    ScopeSelf = Symbol::create(L"%SCOPE-SELF");
    ScopeParent = Symbol::create(L"%SCOPE-PARENT");
    ScopeName = Symbol::create(L"%SCOPE-NAME");
    ScopeRoot = Symbol::create(L"%SCOPE-ROOT");
    ScopeStopRead = Symbol::create(L"%SCOPE-STOP-READ");
    ScopeStopWrite = Symbol::create(L"%SCOPE-STOP-WRITE");
    Arguments = Symbol::create(L"arg");
    LoadEval = Symbol::create(L"eval");
    LoadQuote = Symbol::create(L"quote");
    ObjectCall = Symbol::create(L"%CALL");
    ObjectToString = Symbol::create(L"%TO-STRING");
    ObjectInit = Symbol::create(L"%INIT");
    ObjectDelete = Symbol::create(L"%DELETE");
    ObjectClass = Symbol::create(L"%CLASS");
    ObjectClassName = Symbol::create(L"%NAME");
    IterNext = Symbol::create(L"next");
    IterPrev = Symbol::create(L"prev");
    IterValue = Symbol::create(L"value");
    IterKey = Symbol::create(L"key");
    IterSource = Symbol::create(L"source");
    IterSeek = Symbol::create(L"seek");
    IterReset = Symbol::create(L"reset");
    IterGetSide = Symbol::create(L"side");
    IterIsFirst = Symbol::create(L"is-first");
    IterIsBody = Symbol::create(L"is-body");
    IterIsLast = Symbol::create(L"is-last");
    IterClone = Symbol::create(L"clone");
    IterJoinerAdd = Symbol::create(L"add");
    StringAll = Symbol::create(L"all");
    StringAt = Symbol::create(L"at");
    StringSlice = Symbol::create(L"slice");
    StringCut = Symbol::create(L"cut");
    StringFind = Symbol::create(L"find");
    StringFindRegexp = Symbol::create(L"find-regexp");
    StringBefore = Symbol::create(L"before");
    StringAfter = Symbol::create(L"after");
    StringShift = Symbol::create(L"shift");
    StringShiftStart = Symbol::create(L"shift-start");
    StringShiftEnd = Symbol::create(L"shift-end");
    StringNoOp = Symbol::create(L"no-op");
    StringUpdate = Symbol::create(L"update");
    StringTransformUp = Symbol::create(L"upper");
    StringTransformDown = Symbol::create(L"lower");
    StringTransformTitle = Symbol::create(L"title");
    StringTransformInverse = Symbol::create(L"invert");
    StringCrop = Symbol::create(L"crop");
    StringDelete = Symbol::create(L"delete");
    StringReplace = Symbol::create(L"replace");
    StringAppend = Symbol::create(L"append");
    StringPrepend = Symbol::create(L"prepend");
    StringLength = Symbol::create(L"length");
    StringHas = Symbol::create(L"has");
    StringCount = Symbol::create(L"count");
    StringRangeSave = Symbol::create(L"range-save");
    StringRangeLoad = Symbol::create(L"range-load");
    string_join = Symbol::create(Syntax::default_string_joiner);
  }
}

