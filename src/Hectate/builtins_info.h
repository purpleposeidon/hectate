
#ifndef BUILTINS_INFO_H
#define BUILTINS_INFO_H

#include "Hectate/Forwards.h"

String builtin_to_string(const HectateBuiltin &symbol, bool demangle);
String builtin_backtrace(bool demangle);

#endif /* BUILTINS_INFO_H */

