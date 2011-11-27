#!/usr/bin/env python

#This is used to generate a bunch of symbols that are referred to in code.
#So don't do  if (argument.getSymbol() == Symbol::create(L"an-option")) ,
#define that symbol in this file.


#Format: "C++_Variable_Name Hectate_Symbol_Text"
words = [s.split(' ', 1) for s in (
  #Scope
  "ScopeSelf %SCOPE-SELF", #scope weakref to itself
  "ScopeParent %SCOPE-PARENT", #hard reference to scope's parent, if it had one
  "ScopeName %SCOPE-NAME", #optional scope name
  "ScopeRoot %SCOPE-ROOT", #access the root scope
  "ScopeStopRead %SCOPE-STOP-READ", #Children can't read
  "ScopeStopWrite %SCOPE-STOP-WRITE", #Children can't write

  #Arguments
  "Arguments arg", #set-arg
  "LoadEval eval", #load-arg
  "LoadQuote quote", #load-arg

  #Object
  "ObjectCall %CALL", #With [object] or [object args]
  "ObjectToString %TO-STRING", #With [print object]
  "ObjectInit %INIT", #When object is created
  "ObjectDelete %DELETE", #When object loses all references
  "ObjectClass %CLASS", #Class definition
  "ObjectClassName %NAME", #Name for this class

  #Iterators
  "IterNext next", #These guys are used by the Iterator ABC eval()
  "IterPrev prev",
  "IterValue value",
  "IterKey key",
  "IterSource source",
  "IterSeek seek",
  "IterReset reset",
  "IterGetSide side",
  "IterIsFirst is-first",
  "IterIsBody is-body",
  "IterIsLast is-last",
  "IterClone clone",

  "IterJoinerAdd add",

  #String DSL
  #Selectors
  "StringAll all",
  "StringAt at",
  "StringSlice slice",
  "StringCut cut",
  "StringFind find",
  "StringFindRegexp find-regexp",
  "StringBefore before",
  "StringAfter after",
  "StringShift shift",
  "StringShiftStart shift-start",
  "StringShiftEnd shift-end",
  "StringNoOp no-op",
  #Operators
  "StringUpdate update",
  "StringTransformUp upper",
  "StringTransformDown lower",
  "StringTransformTitle title",
  "StringTransformInverse invert",
  "StringCrop crop",
  "StringDelete delete",
  "StringReplace replace",
  "StringAppend append",
  "StringPrepend prepend",
  #Info
  "StringLength length",
  "StringHas has",
  "StringCount count",
  #Other
  "StringRangeSave range-save",
  "StringRangeLoad range-load",

)]

#Like above, except the 2nd item is literal C++
raw = [s.split(' ', 1) for s in (
  "string_join Syntax::default_string_joiner",
)]










warning = """
// ***************************************************
// This file was created automatically by genwords.py.
// Edit that script rather than this file.
// ***************************************************
"""

h_front = warning + """
#ifndef WORDS_H
#define WORDS_H

#include "Hectate/Types.h"
#include "Hectate/config.h"

namespace Word {
  void define();
  
"""
h_back = """}

#endif

"""

cpp_front = warning + """ 
#include "Hectate/words.h"

namespace Word {
"""
cpp_mid = """
  void define() {
"""
cpp_back = """  }
}

"""

def write_h(h):
  h.write(h_front)
  for var, name in words+raw:
    h.write('  extern Symbol %s;\n' % var);
  h.write(h_back)

def write_cpp(cpp):  
  cpp.write(cpp_front)
  for var, name in words+raw:
    cpp.write('  Symbol %s;\n' % var)
  cpp.write(cpp_mid)
  
  for var, name in words:
    cpp.write('    %s = Symbol::create(L"%s");\n' % (var, name))
  for var, name in raw:
    cpp.write('    %s = Symbol::create(%s);\n' % (var, name))
  
  cpp.write(cpp_back);

if __name__ == '__main__':
  import os, sys
  end = os.path.join(os.getcwd(), sys.argv[0])
  os.chdir(os.path.dirname(end))

  if 'h' in sys.argv:
    write_h(sys.stdout)
  elif 'cpp' in sys.argv:
    write_cpp(sys.stdout)
  else:  
    os.system("redo words.h words.cpp")


