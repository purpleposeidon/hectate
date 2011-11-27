
#ifndef OBJECT_H
#define OBJECT_H

#include "Hectate/Container.h"
#include "Hectate/Scope.h"


class Object : public Container {
public:
  void eval(Context &context); //[object-instance arguments*] --> [%CALL arguments*]
  String toString(Context &context); //Uses %TO-STRING, %NAME, or "object"
  virtual MapAction map(std::function<MapAction(Atom &)> function); //map function onto contents

private:
  //Hectate functions for manipulationg objects
  friend void h_object_new(Context &context); //[object/new base_class args]
  friend void h_object_get(Context &context); //[object/get object key]
  friend void h_object_set(Context &context); //[object/set object key value]

  template <typename F>
  void map_scopes(F func);
  
  Scope contents;
};

#endif /* OBJECT_H */

