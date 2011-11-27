
#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <queue>
#include <set>

#include "Hectate/common.h"
#include "Hectate/Atom.h"
#include "Hectate/Node.h"
#include "Hectate/FiberManager.h"
#include "Hectate/ArgManager.h"

typedef std::vector<Atom> NodeVector;

enum EvalAction {
  NO_ACTION = 0, /* no action set; the initial value. Is an error. */
  RESULT, /* result from evaluating a node; the default Action */
  RETURN, /* ends {do} */
  BREAK, /* ends (while), (repeat), ... */
  REDO, /* this function will be run again (?) */
  ERROR, /* from [error-throw] or a builtin */
  ABORT, /* uncatchable error. This is fatal in all cases; do not handle it. */
  eval_action_count, //Keep this last
  //Changing this? Change the two functions below & flow.cpp Action::
};

const wchar_t *eval_action_name(EvalAction a);
const wchar_t *eval_action_description(EvalAction a);



class Context {
  public:
    Context(FiberManager *fm);
    ~Context();
    const Context &operator=(const Context &) = delete;
    Context(const Context &) = delete;

    void pushEval(Atom &atom); //push an atom onto the eval stack
    Atom &getEval(); //gets top of stack
    void setRoot(Atom &atom); //sets the root scope

    
    template <class EvalType>
    void eval(EvalType what, bool throw_action = true); //Can be: const Atom &, (const?) Iterator *
    void propagate() throw (HectatePropagate); //If we don't have RESULT through HectatePropagate

    ArgManager &arg(const wchar_t *function_name); //gets yarr argh manager
    ArgManager *argAt(unsigned int before);
    //const Atom &getArg(unsigned int index); //[arg0 arg1 arg2]
    //const Atom getArgWant(unsigned int index, AtomType want_type); //if arg[index]'s type != want, eval
    //unsigned int argCount(); //How many args the top of the stack has. [] has 0, [a] has 1
    bool hasSymbol(Symbol sym); //Returns if sym is exists and is accessable
    Atom &getSymbol(Symbol sym); //finds symbol in the scope, and in the root_scope_atom.
    void setSymbol(Symbol to_set, const Atom &value); //sets the symbol in our scope
    void updateSymbol(Symbol to_set, const Atom &value); //Update a symbol that exists in a higher scope
    void unsetSymbol(Symbol to_unset); //remove the symbol from our scope
    

    bool hasAction(EvalAction action); //return true if eval() gave that action. TODO: A vardiac hasAction would be nice. But, it would need a terminating value?  (Now with '0x, we can use a template)
    Atom &result(); //gets result Atom
    EvalAction action(); //gets action
    Atom deref(const Atom &orig); //gets what is pointed at by the symbol and weakref
    bool want(AtomType t);
    bool want(AtomType t, Atom &to_convert);
    void need(AtomType t);
    void need(AtomType t, Atom &to_convert);
    void finish(EvalAction action = RESULT, Atom atom = ATOM_NONE, bool force = false);
    void finish(Atom atom) /* finish(EvalAction = RESULT, Atom) */ { finish(RESULT, atom); }
    void clear(); //finish(NO_ACTION, ATOM_NONE);
    void error(String err); //finish(ERROR, err)
    void error(const wchar_t *err); //finish(ERROR, err)

    bool isComplete(); //returns if the evaluation is complete (stack is empty)
    void wait(); //Yields to the FiberManager
    void run(Atom &atom); //

    void pushScope();
    void popScope();


    template <typename F>
    void map_eval_stack(F function) {
      //for (auto it : eval_stack) { //XXX not until GCC 4.6
      for (auto it = call_stack.rbegin(); it != call_stack.rend(); it++) {
        if (function(it)) {
          break;
        }
      }
    }

    template <typename F>
    void map_scope_stack(F function) {
      for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); it++) {
        Scope &arg = it->getScope();
        if (function(arg)) {
          break;
        }
      }
      Scope &arg = root_scope_atom.getScope();
      function(arg);
    }

    typedef std::set<Atom> IncludesList;
    inline IncludesList &getIncludes() { return includes; }
    
    struct FinishSaver {
      FinishSaver(Context &context) : context(context) {
        result = context.result();
        action = context.action();
      }
      ~FinishSaver() {
        context.finish(action, result, true);
      }
      Context &context;
      Atom result;
      EvalAction action;
    };
    FinishSaver saveFinish();
  private:
    Atom root_scope_atom; //scopes
    std::vector<Atom> scope_stack;
    IncludesList includes; //scopes that are include'd
    std::vector<std::shared_ptr<ArgManager> > call_stack;

    void push_call(Atom &iter);
    void pop_call();
    
    Atom block_result;
    EvalAction block_action;
    
    unsigned int evals; //TODO: Keep track of statistics
    
    FiberManager *fiber_manager;
    //TODO: Need a Scope list for objects

    
    Atom &scope_symbol(Symbol sym); //looksup sym in our scope
    void eval_body(Atom &atom);
    void eval_body_iter(Atom &iter_src);

    
    friend void eval_iter_node(Context &context, Atom &arg0);
    friend void h_set_arg(Context &context);
    friend struct ContextIterCall;
};

//TODO: Track memory so that we can monitor usage, and destroy it if necessary. *All* memory allocation should go through this.
//context.registerMemory(Mem) or something? Use that “new(ptr) Type” Type syntax?

#endif /* CONTEXT_H */

