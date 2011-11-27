#ifndef FORWARDS_H
#define FORWARDS_H




//TODO: enum AtomType : char
//(not used right now because KDevelop can't parse it :P)
/*
 * "Values" must fit inside sizeof(void*). They are always copied.
 * "Callables" may return something other than themselves if evaluated.
 * "Containers" need memory management and give a way to get at their contents
 */
enum AtomType {
  //values
  NONE = 0,
  BOOLEAN = '?',
  NUMBER = '#',
  TYPE_NAME = 'c',
  SYMBOL = '%', //[Callable]
  BUILTIN = 'b', //[Callable]

  //memory using types
  WEAKREF = 'W', //[Callable]
  STRING = '"', //[Callable]
  NODE = 'N', //[Callable] [Container]
  SCOPE = 'S', //[Callable] [Container]
  OBJECT = 'O', //[Callable] [Container]
  ABSTRACT = 'A', //[Callabe] [Container]
  PATH = '/', //[Callable]

  //If you've added a type, there's many functions to update in:
  //Types.cpp, ContextEval.cpp, Atom.cpp
  //(There's probably some I've forgotten, too.)
  //Consider first implementing the type using OBJECT or ABSTRACT.
};

enum RefType {StrongRef, WeakRef}; //TODO: enum class would be nice; I want the namespace


//Forward declarations of classes used in Atoms
class Symbol;
class Path; //TODO: Array of Symbol
class Node;
class Scope;
class Object;
class AbstractType;

//Forward declarations of classes used elsewhere
class Atom;
class Context;
class Container;
class Iterable;
class Iterator;

//The easy types
#include <string>
#include <sstream>

typedef bool Boolean;
typedef double Number;
typedef std::wstring String;
typedef std::wstringstream StringStream;
typedef void (*HectateBuiltin)(Context &context); //prefix these functions with h_ so that their symbols can be recognized


#endif /* FORWARDS_H */

