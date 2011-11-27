#include "Hectate/common.h"

#include <cstdlib>
#include <cwchar>
#include <iostream>


const String BOLD(L"\x1b[1m"), NORMAL(L"\x1b[0m"), DIM(L"\x1b[38;5;237m");
//DIM(L"\x1b[34m"); //L"\x1b[30m\x1b[1m");

String bold(String what) {
  return BOLD + what + NORMAL;
}

String bold(const wchar_t *what) {
  return BOLD + String(what) + NORMAL;
}

String dim(String what) {
  return DIM + what + NORMAL;
}

String dim(const wchar_t *what) {
  return DIM + String(what) + NORMAL;
}


void breakhere() {} //because gdb sucks.

HectateError::HectateError() : what_str(NULL) {
  breakhere();
  msg = String(L"Unspecified error");
  if (!CATCH_ERRORS) {
    //std::wcerr << L"[HectateError created] " << msg << std::endl;
  }
}

const String error_prefix = L"[HectateError created, try 'breakhere'] ";

HectateError::HectateError(const wchar_t *src_msg) : what_str(NULL) {
  breakhere();
  msg = String(src_msg);
  if (!CATCH_ERRORS) {
    std::wcerr << error_prefix << msg << std::endl;
  }
}

HectateError::HectateError(String src_msg) : what_str(NULL) {
  breakhere();
  msg = src_msg.substr();
  if (!CATCH_ERRORS) {
    std::wcerr << error_prefix << msg << std::endl;
  }
}

String &HectateError::get_msg() {
  return msg;
}

HectateError::~HectateError() throw () {
  if (what_str) {
    free(what_str);
  }
}

const char *HectateError::what() {
  if (what_str == NULL) {
    String my_msg = get_msg();
    const wchar_t *src = my_msg.c_str();
    mbstate_t size_ps, conv_ps;
    size_t size = wcsrtombs(NULL, &src, 0, &size_ps); //get the size needed for conversion
    size++;
    what_str = (char*)malloc(sizeof(wchar_t)*(size));
    wcsrtombs(what_str, &src, size, &conv_ps);
  }
  return what_str;
}

