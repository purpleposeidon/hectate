#include "Container.h"

#include <sstream>

#include "Hectate/Atom.h"

void Container::freeze(bool do_freeze) {
  if (level == undefined) {
    throw HectateAbort(L"Container::freeze(): Can't freeze because level is undefined (setRoot?)");
  }
  auto lock = getLock();
  if (frozen == do_freeze) {
    if (do_freeze) {
      throw HectateError(L"Container::freeze(true): Container is already frozen");
    }
    else {
      throw HectateError(L"Container::freeze(false): Container is already thawed");
    }
  }
  frozen = do_freeze;
  map([=](Atom &child) {
    if (child.isContainer()) {
      if (repair_reference(child)) {
        child.getContainer().freeze(do_freeze);
      }
    }
    return MapContinue;
  });
}

bool Container::isFrozen() {
  return frozen;
}

String Container::containerInfo() {
  StringStream out;
  out << L"Level: " << level << L" Thefts: " << thefts;
  return out.str();
}

Container::pLG Container::getLock() {
  LG *guard = new LG(mutex);
  return pLG(guard);
}



void Container::makeRoot() {
  mark(root);
}

RefType Container::claim(Atom &child) {
  if (child.isContainer()) {
    return child.getContainer().container_moving(this);
  }
  else {
    return StrongRef;
  }
}

Atom Container::properClaim(Atom &child) {
  switch (claim(child)) {
    case WeakRef:
      return child.makeWeakref();
      break;
    case StrongRef:
      return child;
      break;
  }
  throw HectateAbort(L"Container::properClaim() switch failure");
}


void Container::dropped(Atom &child) {
  if (child.isContainer()) {
    child.getContainer().container_dropped(this);
  }
}



RefType Container::container_moving(Container *claimer, bool have_lock) {
  //The calls to mark() have have_lock as true because we will acquire the lock
  auto body = [&]() -> RefType {
    //move me
    if (isFrozen()) {
      return WeakRef;
    }
    if (level == undefined) {
      if (claimer->level != undefined) {
        mark(claimer->level, true);
      }
      return StrongRef;
    }
    if (can_contain(claimer->level, level)) {
      mark(claimer->level, true);
      return StrongRef;
    }
    else {
      //my level is higher than yours, so you don't get to reference me.
      return WeakRef;
    }
  };
  if (have_lock) {
    return body();
  }
  else {
    auto lock = getLock();
    return body();
  }
}


void Container::container_dropped(Container *parent, bool have_lock) {
  //XXX arg not used? Why is that there?
  //drop me
  auto body = [&]() {
    /*
    //XXX TODO: Try enabling this?
    if (level != undefined && !can_contain(parent->level, level)) {
      //I was stolen by my parent! But this no longer matters, as the parent doesn't reference us anymore
      thefts--;
    }
    */
    if (thefts) {
      //we were stolen; we might have created a reference to an old parent
      check_thefts();
    }
  };
  if (have_lock) {
    body();
  }
  else {
    auto lock = getLock();
    body();
  }
}


bool Container::repair_reference(Atom &child) {
  //makes sure my reference to child is in a proper state.
  Container &c = child.getContainer();
  if (c.level == undefined) {
    if (level != undefined) {
      //reclaim the child
      c.mark(level);
    }
    //else: I am also undefined; do nothing
    return true;
  }
  if (!can_contain(level, c.level)) {
    //stolen from us!
    child.changeRefType(WeakRef);
    return false; //the child is no longer a container
  }
  return true;
}

void Container::mark_body(LevelType parent_level, bool have_lock) {
  LevelType new_level = parent_level + 1;
  if (parent_level == undefined) {
    level = undefined;
  }
  else if (level == new_level) {
    return; //no action needed
  }
  else {
    if (level != undefined) {
      thefts++;
    }
    level = new_level;
  }
  //XXX: What if there's a cycle?
  map([=](Atom &child) {
    if (!child.isContainer()) {
      return MapContinue;
    }
    Container &c = child.getContainer();
    if (repair_reference(child)) {
      //still a container
      c.mark(level);
      if (c.level == level && level != undefined) {
        throw HectateAbort(L"Container::mark_body(): Child's level is the same as ours! (And level is defined...)");
      }
    }
    //else: repair_reference changed it to a weakref
    return MapContinue;
  });
}


void Container::mark(LevelType parent_level, bool have_lock) {
  if (have_lock) {
    mark_body(parent_level, have_lock);
  }
  else {
    auto lock = getLock();
    mark_body(parent_level, true);
  }
}

void Container::check_thefts() {
  //XXX: Again, what if there's a cycle?
  //cycles probably won't be a problem because of repair_reference().
  map([=](Atom &child) {
    if (child.isContainer()) {
      if (repair_reference(child)) {
        Container &c = child.getContainer();
        c.check_thefts();
      }
      //else: repair_reference changed it to a weakref
    }
    return MapContinue;
  });
}


