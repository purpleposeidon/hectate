#include "builtins_info.h"

#include <iostream>
#include <cstring>
#include <dlfcn.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string>

#include "Hectate/common.h"



void local_function() {
  //hi.
}

std::string prettyify(const char *src) {
  if (src == NULL) {
    return std::string("<?>");
  }
  char *ret;
  //demangle
  int demangle_status = 0;
  ret = abi::__cxa_demangle(src, 0, 0, &demangle_status);
  if (demangle_status || ret == NULL) {
    return std::string(src);
  }

  //do some doctoring to make it look nice
  const char match[] = "(Context&)";
  static auto match_len = strlen(match);
  auto l = strlen(ret);
  if (ret[0] == L'h' && ret[1] == L'_' && strcmp(&ret[l-match_len], match) == 0) {
    //the usual end does nothing for us, get rid of it
    ret[l-match_len] = '\0';
    //(we'll keep the h_ prefix tho)
  }
  std::string final(ret);
  free(ret);
  return final;
}

String builtin_to_string(const HectateBuiltin &symbol, bool demangle) {
  StringStream out;
  if (symbol) {
    //load structure
    Dl_info info;
    info.dli_saddr = 0;
    int dladdr_error = dladdr((void*)symbol, &info);
    if (dladdr_error == 0) {
      throw HectateAbort(L"dladdr() failed");
    }
    
    if (demangle && info.dli_sname) {
      out << prettyify(info.dli_sname).c_str();
    }
    else if (info.dli_sname) {
      out << info.dli_sname;
    }
    else {
      out << std::hex << "0x" << (long long)symbol << std::dec;
    }
    //strange offset?
    if (symbol != info.dli_saddr && info.dli_saddr) {
      out << std::hex;
      out << "+" << (long long)symbol - (long long)info.dli_saddr;
      out << std::dec;
      //(maybe we should really throw an error here...? Oh well.)
    }

    //add the filename if it's an imported symbol
    Dl_info local_info;
    int local_dladdr_error = dladdr((void*)local_function, &local_info);
    if (local_dladdr_error == 0) {
      throw HectateAbort(L"dladdr() failed [2]");
    }
    if (strcmp(info.dli_fname, local_info.dli_fname)) {
      //it is an imported function
      out << " from " << info.dli_fname;
    }
  }
  else {
    out << L"NULL";
  }
  return out.str();
}

void check_miss(int &missed, StringStream &stream) {
  if (missed == 0) {
    return;
  }
  stream << L"(" << missed << L" function" << (missed == 1 ? "" : "s") <<  " had no symbol)" << std::endl;
  missed = 0;
}

String builtin_backtrace(bool demangle) {
  const int MAX = 100;
  void *buffer[MAX];
  int size = backtrace((void**)&buffer, MAX);
  char **symbols = backtrace_symbols((void**)&buffer, size);

  StringStream stream;
  int missed = 0;
  for (int i = 0; i < size; i++) {
    if (demangle) {
      stream << symbols[i] << std::endl;
      continue;
    }
    
    std::string s = symbols[i];
    size_t left = s.rfind("(") + 1;
    if (left == std::string::npos) {
      missed++;
      continue;
    }
    size_t end = s.find("+", left);
    if (end == std::string::npos) {
      end = s.rfind(")");
      if (end == std::string::npos) {
        missed++;
        continue;
      }
    }
    
    
    check_miss(missed, stream);
    char *symbol = strdup(s.substr(left, end-left).c_str());
    std::string realname = prettyify(symbol);
    free(symbol);
    if (realname.length() == 0) {
      missed++;
      continue;
    }
    
    stream << realname.c_str() << std::endl;
    
  }
  check_miss(missed, stream);
  free(symbols);
  return stream.str();
}

