#include "Hectate/Types.h"

bool uses_memory(AtomType type) {
  //NOTE: Could optimize? Make a fat array, return uses_memory_array[type] (Or make all mem users have a bit set)
  switch (type) {
    case NODE:
    case STRING:
    case SCOPE:
    case WEAKREF:
    case OBJECT:
    case ABSTRACT:
    case PATH:
      return true;
    default:
      return false;
  }
}

bool is_container(AtomType type) {
  switch (type) {
    case NODE:
    case SCOPE:
    case OBJECT:
    case ABSTRACT:
      return true;
    default:
      return false;
  }
}


const String type_name(AtomType type) {
  //XXX not consistent with naming/capitalization
  switch (type) {
    case NONE: return String(L"None");
    case BOOLEAN: return String(L"Boolean");
    case NODE: return String(L"Node");
    case SYMBOL: return String(L"Symbol");
    case NUMBER: return String(L"Number");
    case TYPE_NAME: return String(L"TypeName");
    case STRING: return String(L"String");
    case BUILTIN: return String(L"Builtin");
    case SCOPE: return String(L"Scope");
    case WEAKREF: return String(L"Weakref");
    case OBJECT: return String(L"Object");
    case ABSTRACT: return String(L"AbstractType");
    case PATH: return String(L"SymbolPath");
    default: return String(L"Unknown");
  }
}

