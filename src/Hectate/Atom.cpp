#include "Hectate/Atom.h"

#include <assert.h>
#include <iostream>
#include <math.h>
#include <assert.h>

#include "Hectate/common.h"
#include "Hectate/config.h"
#include "Hectate/Parser.h"

#include "Hectate/Node.h"
#include "Hectate/Scope.h"
#include "builtins_info.h"
#include "Hectate/Object.h"

String ut_msg = L"Unconvertible type";
String &UnconvertibleType::get_msg() {
  return ut_msg;
}


struct Memory {
public:
  Memory() : refcount(0), weak_refcount(0), external(false), type(NONE) {
    value.node = NULL;
    #if MEMWATCH
    watched = false;
    #endif
  }
  ~Memory() {
    if (refcount != 0) {
      throw HectateAbort(L"Tried deleting memory with non-zero refcounts");
    }
    if (weak_refcount != 0) {
      throw HectateAbort(L"Tried deleting memory with non-zero weak_refcounts");
    }
    //Just for now: these three lines make it obvious when there's a memory management fail
    refcount = -1337;
    weak_refcount = -1337;
    //value.node = NULL;
    #if MEMWATCH
    if (watched) {
      std::wcout << this << L" deleted\n";
    }
    #endif
  }
  RefCountType refcount, weak_refcount;
  bool external;
  AtomType type;
  union {
    Node *node;
    String *string;
    Scope *scope;
    Object *object;
    AbstractType *abstract;
  } value;
  #if MEMWATCH
  bool watched;
  #endif
};










Atom::Atom() : type(NONE) {
  value.boolean = false;
}

Atom::Atom(Number n) : type(NUMBER) {
  value.number = n;
}

Atom::Atom(Boolean b) : type(BOOLEAN) {
  value.boolean = b;
}

Atom::Atom(const Atom &orig) : type(orig.type), value(orig.value) {
  addRef();
}

Atom::~Atom() {
  subRef();
  //clear();
}

const Atom &Atom::operator=(const Atom &right) {
  if (this == &right) {
    //XXX Do I need to addRef()?
    std::wcerr << L"[NOTE] Atom::operator=(): self-assignment\n";
    return right;
  }
  //NOTE: The order of the next two lines is probably important.
  right.addRef();
  this->clear();
  this->type = right.type;
  this->value = right.value;
  return *this;
  /*subRef();
  Atom &o = const_cast<Atom&>(orig); //XXX... is this right?
  type = o.type;
  value = o.value;
  addRef();
  return o;*/
}


bool Atom::operator==(const Atom &right) const {
  if (type != right.type) {
    return false;
  }
  return value.mem == right.value.mem;
}

bool Atom::operator<(const Atom &right) const {
  if (type != right.type) {
    return type < right.type;
  }
  return value.mem < right.value.mem;
}



void Atom::manage_memory() const throw () {
  //delete the memory if necessary
  assert(uses_memory(type) || type == WEAKREF);
  if (value.mem->refcount < 0 || value.mem->weak_refcount < 0) {
    throw HectateAbort(L"Atom::manage_memory(): (weak?) reference count is negative");
  }
  if (value.mem->refcount == 0 && value.mem->value.node) {
    //There are no more strong references to the object; delete any memory associated with it
    //Free the memory
/*#if MEMWATCH
    if (value.mem->watched) {
      std::wcerr << L"delete " << value.mem << std::endl;
    }
#endif*/
    auto victim = value.mem->value;
    value.mem->value.node = NULL;
    value.mem->refcount = 1;
    if (!value.mem->external) {
      //If it's external, don't actually delete
      switch (value.mem->type) {
        case NODE:
          delete victim.node;
          break;
        case STRING:
          delete victim.string;
          break;
        case SCOPE:
          delete victim.scope;
          break;
        case OBJECT:
          delete victim.object;
          break;
        case ABSTRACT:
          delete victim.abstract;
          break;
        default:
          throw HectateAbort(L"Atom::manage_memory(): No more strong references, but deleting for a type we don't recognize");
          break;
      }
    }
    value.mem->refcount = 0;
  }

  if (value.mem->weak_refcount == 0 && value.mem->refcount == 0) {
    //There are no more weak or strong references to the object; delete the memory
    if (value.mem->value.node != NULL) {
      throw HectateAbort(L"Atom::manage_memory(): No more references, but the pointer isn't NULL");
    }
    //errr, is this const_cast thing correct?
    Atom *victim = const_cast<Atom*>(this);
    delete victim->value.mem;
    victim->value.mem = NULL;
  }
}


void Atom::addRef() const {
  if (type == WEAKREF) {
    value.mem->weak_refcount++;
  }
  else if (uses_memory(type)) {
    value.mem->refcount++;
#if MEMWATCH
    if (value.mem->watched) {
      std::wcerr << value.mem << L"++ " << value.mem->refcount << L"\n";
    }
#endif
  }
}

void Atom::subRef() const {
  if (type == WEAKREF) {
    value.mem->weak_refcount--;
    manage_memory();
  }
  else if (uses_memory(type)) {
    value.mem->refcount--;
#if MEMWATCH
    if (value.mem->watched) {
      std::wcerr << value.mem << L"-- " << value.mem->refcount << L"\n";
    }
#endif
    manage_memory();
  }
}

void Atom::clear() {
  subRef();
  value.mem = NULL;
  type = NONE;
}







bool Atom::isType(AtomType check_type) const {
  return type == check_type;
}

AtomType Atom::getType() const {
  return type;
}


String Atom::toString(Context &context, bool internal_string) const {
  StringStream out;
  switch (type) {
    case NUMBER:
      out << getNumber();
      break;
    case SYMBOL:
      out << getSymbol().toString();
      break;
    case BOOLEAN:
      if (getBoolean()) {
        out << "true";
      }
      else {
        out << "false";
      }
      break;
    case STRING:
      {
        //XXX This does not use Syntax strings as it ought to!
        String s = getString().substr(); //Use a copy

        int double_quote = 0, single_quote = 0;
        for (int i = 0; s[i] != L'\0'; i++) {
          if (s[i] == L'"') {
            double_quote++;
          }
          if (s[i] == L'\'') {
            single_quote++;
          }
        }
        wchar_t quote_type = L'"';
        if (!internal_string) {
          if (double_quote > single_quote) {
            //Have more " than '
            quote_type = L'\'';
          }
          if (s.length() == 1 && single_quote == 0) {
            //Is a single character, try to conform to C style
            quote_type = L'\'';
          }
        }
        //Escape escapes
        for (int i = s.size(); i >= 0; i--) {
          switch (s[i]) {
            case L'\n':
              s.replace(i, 1, Syntax::escape+L"n");
              break;
            case L'\t':
              s.replace(i, 1, Syntax::escape+L"t");
              break;
            case L'\\':
              s.replace(i, 1, Syntax::escape+Syntax::escape); //XXX: Limits Syntax::escape to 1 char
              break;
          }
          if (s[i] == quote_type) {
            s.replace(i, 1, Syntax::escape+quote_type);
          }
        }
        out << quote_type << s << quote_type;
      }
      break;
    case NODE:
      out << getNode().toString(context);
      break;
    case TYPE_NAME:
      out << type_name(getTypeName());
      break;
    case SCOPE:
      out << getScope().toString(context);
      break;
    case WEAKREF:
      {
        Atom a = getWeakref();
        out << L"WeakRef ";
        if (a.isContainer()) {
          out << L"to <" << type_name(a.getType()) << L">";
        }
        else {
          out << a.toStringRepr(context);
        }
      }
      break;
    case BUILTIN:
      out << L"builtin " << builtin_to_string(value.builtin, true);
      break;
    case NONE:
      out << L"none";
      break;
    case OBJECT:
      out << getObject().toString(context);
      break;
    case ABSTRACT:
      out << getAbstract().toString(context);
      break;
    default:
      out << "Type without toString";
      break;
  }
  return out.str();
}

String Atom::toStringRepr(Context &context) const {
  String ret;
  switch (getType()) {
    //These guys are obvious enough
    case STRING:
    case WEAKREF:
    case NONE:
    case BUILTIN: //TODO: Return builtin's name... somehow...
      break;

    case SCOPE:
      return String(L"<scope>");
      break;
    //these guys need a hint
    case NUMBER:
    case SYMBOL:
    default:
      ret += type_name(getType()) + String(L" ");
      break;
      
  }
  ret += toString(context);
  return ret;
}

Atom Atom::convert(Context &context, AtomType dest_type) const {
  if (dest_type == type) {
    return *this; //NOTE: does not copy memory objects
  }
  if (dest_type == STRING) {
    return Atom::create(new String(toString(context)));
  }

  //Do try to not forget that converting to STRING is already handled.
  switch (type) {
    //Our source type is...
    case NONE:
      if (dest_type == BOOLEAN) {
        return Atom::create(false);
      }
      break;
    case NUMBER:
      if (dest_type == BOOLEAN) {
        if (getNumber() == 0.0 || isnan(getNumber())) {
          return Atom::create(false);
        }
        return Atom::create(true);
      }
      break;
    case SYMBOL:
      if (dest_type == BOOLEAN) {
        return Atom::create(true);
      }
      break;
    case BOOLEAN:
      if (dest_type == NUMBER) {
        return Atom::create(getBoolean() ? 1.0 : 0.0);
      }
      break;
    case TYPE_NAME:
      break;
    case BUILTIN:
      break;

    case NODE:
      break;
    case STRING:
      {
        if (dest_type == BOOLEAN) {
          return Atom::create<Boolean>(getString().size());
        }
        if (dest_type == NODE) {
          Parser p;
          Atom r = p.parseAtom(getString());
          if (!r.isType(dest_type)) {
            throw UnconvertibleType();
          }
          return r;
        }
      }
      break;
    case SCOPE:
      break;

    default:
      throw UnconvertibleType(); //unknown type
      //XXX AbstractType?
  }
  throw UnconvertibleType();
}








Number Atom::getNumber() const {
  needType(NUMBER);
  return value.number;
}

Symbol Atom::getSymbol() const {
  needType(SYMBOL);
  return value.symbol;
}

Boolean Atom::getBoolean() const {
  needType(BOOLEAN);
  return value.boolean;
}

AtomType Atom::getTypeName() const {
  needType(TYPE_NAME);
  return value.type_name;
}

HectateBuiltin Atom::getBuiltin() const {
  needType(BUILTIN);
  return value.builtin;
}


Node &Atom::getNode() const {
  needType(NODE);
  checkRef();
  return *value.mem->value.node;
}

String &Atom::getString() const {
  needType(STRING);
  checkRef();
  return *value.mem->value.string;
}

Scope &Atom::getScope() const {
  needType(SCOPE);
  checkRef();
  return *value.mem->value.scope;
}

Object &Atom::getObject() const {
  needType(OBJECT);
  checkRef();
  return *value.mem->value.object;
}

AbstractType &Atom::getAbstract() const {
  needType(ABSTRACT);
  checkRef();
  return *value.mem->value.abstract;
};

void Atom::checkRef() const {
  if (uses_memory(type)) {
    if (value.mem == NULL) {
      throw HectateError(L"NULL Atom.mem pointer");
    }
    if (value.mem->refcount <= 0) {
      throw HectateError(L"Atom.mem has a non-negative refcount");
    }
  }
}

void Atom::needType(AtomType check_type) const {
  if (type != check_type) {
    throw HectateError(String(L"Type must be a ") + type_name(check_type));
  }
}


void Atom::set(Number val) {
  type = NUMBER;
  value.number = val;
}

void Atom::set(Symbol val) {
  type = SYMBOL;
  value.symbol = val;
}

void Atom::set(Boolean val) {
  type = BOOLEAN;
  value.boolean = val;
}

void Atom::set(AtomType val) {
  type = TYPE_NAME;
  value.type_name = val;
}

void Atom::set(HectateBuiltin val) {
  type = BUILTIN;
  value.builtin = val;
}

void Atom::set(Node *val) {
  new_mem(NODE);
  assert(val);
  value.mem->value.node = val;
}

void Atom::set(String *val) {
  new_mem(STRING);
  assert(val);
  value.mem->value.string = val;
}

void Atom::set(Scope *val) {
  new_mem(SCOPE);
  assert(val);
  value.mem->value.scope = val;
  val->setSelf(*this);
}

void Atom::set(Object *val) {
  new_mem(OBJECT);
  assert(val);
  value.mem->value.object = val;
}

void Atom::set(Memory *val) {
  if (val == NULL) {
    throw HectateError(L"Atom::set: Tried to create a WEAKREF to a NULL pointer");
  }
  clear();
  type = WEAKREF;
  value.mem = val;
  addRef();
}

void Atom::set(AbstractType* val) {
  new_mem(ABSTRACT);
  assert(val);
  value.mem->value.abstract = val;
}




void Atom::new_mem(AtomType new_type) {
  if (!uses_memory(new_type)) {
    throw HectateError(L"Tried to create memory object for a type that does not use it.");
  }
  clear();
  value.mem = new Memory;
  value.mem->refcount = 1;
  type = new_type;
  value.mem->type = new_type;
}

Atom Atom::makeWeakref() const {
  //return a weakref to THIS
  if (!uses_memory(type)) {
    throw HectateError(L"Atom::makeWeakref(): Tried to create a weakref for a type that does not use memory");
  }
  return Atom::create(value.mem);
}

Atom Atom::getWeakref() const {
  needType(WEAKREF);
  if (value.mem->refcount > 0) {
    Atom ret;
    ret.type = value.mem->type;
    ret.value.mem = value.mem;
    ret.addRef(); //To make up for "Atom ret" getting deleted;
    return ret;
  }
  else {
    return ATOM_NONE;
  }
  /*
  //dereference a weakref
  //This gets whatever the weakref was originally. It gets a new reference also.
  needType(WEAKREF);
  Atom ret;
  Mem *real = value.mem->value.mem;
  if (real == NULL) {
    return ATOM_NONE;
  }
  assert(real->weakref == value.mem);
  ret.type = real->type;
  ret.value.mem = real;
  ret.addRef(); //To make up for "Atom ret" getting deleted
  return ret;
  */
}

Atom Atom::unweaken() const {
  if (isType(WEAKREF)) {
    return getWeakref();
  }
  return *this;
}

void Atom::changeRefType(RefType dest_type) {
  if (dest_type == WeakRef) {
    if (type == WEAKREF) {
      throw HectateAbort(L"Atom::changRefType: weakref is already a weakref");
    }
    *this = makeWeakref();
  }
  else if (dest_type == StrongRef) {
    if (type != WEAKREF) {
      throw HectateAbort(L"Atom::changeRefType: strongref is already a strongref");
    }
    else if (!uses_memory(type)) {
      throw HectateAbort(L"Atom::changeRefType: Tried to convert a non-memory using type to a weakref");
    }
    //convert a weakref atom into its value
    *this = unweaken();
  }
}

bool Atom::isContainer() const {
  return is_container(type);
}

Container &Atom::getContainer() const {
  if (isContainer()) {
    if (isType(SCOPE)) {
      return getScope();
    }
    if (isType(NODE)) {
      return getNode();
    }
    if (isType(OBJECT)) {
      return getObject();
    }
    if (isType(ABSTRACT)) {
      return getAbstract();
    }
    throw HectateAbort(L"getContainer(): This is a container?");
  }
  throw HectateAbort(L"getContainer(): Not a container");
}








short Atom::getRefCount() const {
  if (uses_memory(type)) {
    return value.mem->refcount;
  }
  return -1;
}

void Atom::watch(bool do_mem_watch) const {
#if MEMWATCH
  std::wcout << value.mem << L" refcount = " << value.mem->refcount << L"\n";
  if (uses_memory(type)) {
    value.mem->watched = do_mem_watch;
  }
#endif
}

void Atom::externalMemory(bool is_external) {
  if (uses_memory(type)) {
    value.mem->external = is_external;
  }
}

