
#ifndef CONTAINER_H
#define CONTAINER_H

#include <functional>
#include <mutex>
#include <memory>

#include "Hectate/Forwards.h"
#include "Hectate/common.h"

typedef unsigned int LevelType;

class ContainerLockGuard;

class Container {
public:
  Container() : level(undefined), thefts(0), frozen(false) {}
  Container &operator=(const Container&) = delete;
  Container(Container &) = delete;
  
  enum MapAction {MapContinue, MapBreak}; //controls map
  typedef std::function<MapAction(Atom &)> MapFunction;
  virtual MapAction map(MapFunction) = 0; //run function on all items in container. NOTE: May run before initialization is complete
  bool isFrozen(); //return if this container can be modified
  void freeze(bool do_freeze); //freeze or unfreeze

  String containerInfo();
  void makeRoot();
  
  typedef std::lock_guard<std::mutex> LG;
  typedef std::unique_ptr<LG> pLG;
  pLG getLock();

protected:
  RefType claim(Atom &child); //Call before insertion to get type of reference to be used
  Atom properClaim(Atom &child); //converts child to form suitable for storage
  void dropped(Atom &child); //call after the container's removal

private:
  const static LevelType undefined = 0; //Items that haven't been set under a root have this level
  const static LevelType root = 1; //At the top of the container tree
  //used only by mark() and repair_reference();
  LevelType level; //container tree depth level
  unsigned int thefts; //incremented when level is improved, decremented when something below gives up reference
  std::mutex mutex; //try getLock()!
  bool frozen; //if true, unwritable.
  
  static bool can_contain(LevelType parent, LevelType child) {
    return parent < child;
  }

  RefType container_moving(Container *claimer, bool have_lock = false); //Call before insertion to get type of reference to be used
  void container_dropped(Container *parent, bool have_lock = false); //call after the container's removal
  
  void mark(LevelType parent_level, bool have_lock = false);
  void mark_body(LevelType parent_level, bool have_lock);
  void check_thefts();
  bool repair_reference(Atom &child); //return true if child is still a container
};


class ContainerLockGuard {
  
};

#endif /* CONTAINER_H */

