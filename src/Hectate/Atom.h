#ifndef ATOM_H
#define ATOM_H
#include <string>
#include <vector>

#include "Hectate/common.h"
#include "Hectate/Types.h"
#include "Hectate/Symbol.h"


#define MEMWATCH 1

class UnconvertibleType : public HectateError {
//type a can't be transformed into type b
public:
  String &get_msg();
};

struct Memory;

class Atom {
public:
  Atom();
  Atom(Number n); //for initializing some constants
  Atom(Boolean b);
  Atom(const Atom &orig);
  ~Atom();
  const Atom &operator=(const Atom &right);
  //comparison operations use only type and memory location.
  bool operator==(const Atom &right) const;
  bool operator<(const Atom &right) const;
  void clear(); //sets to None, and removes any references

  template <typename T>
  static Atom create(T value) {
    //Use eg create<Number> if the compiler complains about ambiguity.
    Atom ret;
    ret.set(value);
    return ret;
  }

  bool isType(AtomType check_type) const;
  AtomType getType() const;
  void needType(AtomType check_type) const;
  String toString(Context &context, bool internal_string = false) const;
  String toStringRepr(Context &context) const;
  Atom convert(Context &context, AtomType dest_type) const; //may throw an UnconvertibleType

  //value getters
  Number getNumber() const;
  Symbol getSymbol() const;
  Boolean getBoolean() const;
  AtomType getTypeName() const;
  HectateBuiltin getBuiltin() const;

  //object getters
  Node &getNode() const;
  String &getString() const;
  Scope &getScope() const;
  Object &getObject() const;
  AbstractType &getAbstract() const;

  //value setters
  //XXX TODO: Should we make these private? See, if you do a set(), you'll also have to do an addRef()
  void set(Number val);
  void set(Symbol val);
  void set(Boolean val);
  void set(AtomType val);
  void set(HectateBuiltin val);
  
  //object setters; the atom mem object will/should have 1 reference after this.
  void set(Node *val);
  void set(String *val);
  void set(Scope *val);
  void set(Object *val);
  void set(Memory *val);
  void set(AbstractType *val);

  //weak ref stuff
  Atom makeWeakref() const; //Creates a weak reference
  Atom getWeakref() const; //gets the thing pointed to by the reference. This creates a normal reference!
  Atom unweaken() const; //if a weakref, de-reference it

  //container functions
  bool isContainer() const;
  Container &getContainer() const;
  void changeRefType(RefType dest_type);
  
  short getRefCount() const;
  short getWeakRefCount() const;
  void watch(bool do_mem_watch) const; //A debug function
  void externalMemory(bool is_external); //prevent deallocation of memory
  
private:
  AtomType type;
  union {
    Number number;
    Symbol symbol;
    Boolean boolean;
    AtomType type_name;
    HectateBuiltin builtin;
    Memory *mem;
  } value;

  void new_mem(AtomType new_type);
  
  //NOTE: Keeping track of how many refs each atom makes may be a useful thing! It could help for debugging...
  void addRef() const;
  void subRef() const;
  void checkRef() const;
  void manage_memory() const throw ();
};

const Atom ATOM_NONE; //relies on the constructor to make this a none type.

#endif /* ATOM_H */

