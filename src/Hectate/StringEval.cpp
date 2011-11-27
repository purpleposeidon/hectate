#include "StringEval.h"

void TODO_eval_node_string(Context &context, const Atom &src, Node &args) {
  /*
    [str
      #* Contents-related functions *#
      count 'string to count'
      has 'string to see if is in str'
      starts 'bool str begins with this'
      ends 'bool str ends with this'
      cut 'string to remove from the front'
      cut-front 'string to remove from the front'
      cut-end 'string to remove from the end'
      strip 'string to remove from either end'
      strip-any ['list' 'of' 'strings' 'to' 'remove' 'one' 'of'] #Must check all of them for removability; get rid of the longest
      strip-all ['list' 'of' 'strings' 'to' 'remove'] #'strip-any' until the string doesn't change
      #And also *-front and *-back variants

      #* Selection-related functions *#
      N #Gets the Nth char
      N N #Does python-style slicing (But not the 3-indices version that nobody uses)

    ]
  */

  //get rid of TODO and remove the corresponding guy from ContextEval.cpp
}
