#ifndef NODE_H
#define NODE_H

#include <vector>

#include "Hectate/Types.h"
#include "Hectate/Container.h"
#include "Hectate/Iteration.h"

typedef std::vector<Atom> items_type;

class Node : public Container, public Iterable {
  public:
    Node();
    ~Node();
    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;
    const Atom &get(unsigned int index);
    Atom &getMod(unsigned int index);
    virtual MapAction map(Container::MapFunction function);
    void set(unsigned int index, Atom &a);
    unsigned int length();
    void pop();
    void push(Atom a);
    void push(Node *other);
    String toString(Context &context);
    String toStringSimple(Context &context);
    void setBrackets(wchar_t open, wchar_t close);
    items_type::iterator begin();
    items_type::iterator end();
    items_type::iterator erase(items_type::iterator it);
    items_type::iterator insert(items_type::iterator it, Atom &value);

    Atom makeIterator(Atom &src);
    std::size_t size() const;

  private:
    wchar_t open_bracket, close_bracket;
    items_type items;
};


#endif /* NODE_H */

