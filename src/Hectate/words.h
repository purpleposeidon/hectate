
// ***************************************************
// This file was created automatically by genwords.py.
// Edit that script rather than this file.
// ***************************************************

#ifndef WORDS_H
#define WORDS_H

#include "Hectate/Types.h"
#include "Hectate/config.h"

namespace Word {
  void define();
  
  extern Symbol ScopeSelf;
  extern Symbol ScopeParent;
  extern Symbol ScopeName;
  extern Symbol ScopeRoot;
  extern Symbol ScopeStopRead;
  extern Symbol ScopeStopWrite;
  extern Symbol Arguments;
  extern Symbol LoadEval;
  extern Symbol LoadQuote;
  extern Symbol ObjectCall;
  extern Symbol ObjectToString;
  extern Symbol ObjectInit;
  extern Symbol ObjectDelete;
  extern Symbol ObjectClass;
  extern Symbol ObjectClassName;
  extern Symbol IterNext;
  extern Symbol IterPrev;
  extern Symbol IterValue;
  extern Symbol IterKey;
  extern Symbol IterSource;
  extern Symbol IterSeek;
  extern Symbol IterReset;
  extern Symbol IterGetSide;
  extern Symbol IterIsFirst;
  extern Symbol IterIsBody;
  extern Symbol IterIsLast;
  extern Symbol IterClone;
  extern Symbol IterJoinerAdd;
  extern Symbol StringAll;
  extern Symbol StringAt;
  extern Symbol StringSlice;
  extern Symbol StringCut;
  extern Symbol StringFind;
  extern Symbol StringFindRegexp;
  extern Symbol StringBefore;
  extern Symbol StringAfter;
  extern Symbol StringShift;
  extern Symbol StringShiftStart;
  extern Symbol StringShiftEnd;
  extern Symbol StringNoOp;
  extern Symbol StringUpdate;
  extern Symbol StringTransformUp;
  extern Symbol StringTransformDown;
  extern Symbol StringTransformTitle;
  extern Symbol StringTransformInverse;
  extern Symbol StringCrop;
  extern Symbol StringDelete;
  extern Symbol StringReplace;
  extern Symbol StringAppend;
  extern Symbol StringPrepend;
  extern Symbol StringLength;
  extern Symbol StringHas;
  extern Symbol StringCount;
  extern Symbol StringRangeSave;
  extern Symbol StringRangeLoad;
  extern Symbol string_join;
}

#endif

