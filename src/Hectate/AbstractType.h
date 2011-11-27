
#ifndef ABSTRACTTYPE_H
#define ABSTRACTTYPE_H

#include "Hectate/Context.h"
#include "Hectate/Container.h"

class AbstractType : public Container {
public:
  virtual ~AbstractType() {}
  virtual String toString(Context &context) = 0;
  virtual void eval(Context &context) = 0;

  //virtual function inherited from Container
  virtual MapAction map(std::function<Container::MapAction(Atom &)>) { return MapContinue; } //Default to not acting like a container
};


#endif /* ABSTRACTTYPE_H */

