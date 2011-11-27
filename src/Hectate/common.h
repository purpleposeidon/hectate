#ifndef COMMON_H
#define COMMON_H

#include <cwchar>
#include <exception>
#include <mutex>

#include "Hectate/Forwards.h"
#define CATCH_ERRORS 0

String bold(String what);
String bold(const wchar_t *what);
String dim(String what);
String dim(const wchar_t *what);

//Error base type. TODO: Don't use it!
class HectateError : public std::exception {
public:
  HectateError();
  HectateError(const wchar_t *src_msg);
  HectateError(String src_msg);
  ~HectateError() throw ();
  virtual String &get_msg();

  const char* what();
private:
  String msg;
  char *what_str;
};

//This is used to interrupt execution of Builtin's.
class HectatePropagate : public HectateError {
public:
  HectatePropagate() {
    msg = L"*hectate propagation*";
  }
  ~HectatePropagate() throw () {}
  virtual String &get_msg() { return msg; }
private:
  String msg;
};

//This is used for unrecoverable errors
//TODO: This should probably just inherit std::exception
typedef HectateError HectateAbort; //TODO: properness
//class HectateAbort : public HectateError {
//};

/*class LockHolder {
public:
  LockHolder(std::mutex &mutex) : mutex(mutex) {
    mutex.lock();
  }
  ~LockHolder() {
    mutex.unlock();
  }
  LockHolder &operator=(LockHolder &) const = delete;
  LockHolder(LockHolder &) = delete;
private:
  std::mutex &mutex;
};*/

#endif /* COMMON_H */

