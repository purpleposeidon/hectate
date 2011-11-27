#ifndef PARSER_H
#define PARSER_H
#include <wchar.h>
#include <sstream>

#include "Hectate/common.h"

//TODO: Forward-declare
enum ParseErrorEnum {
  //TODO: Get rid of ParseErrorEnum's that aren't used
  NO_ERROR,
  EARLY_EOF,
  NO_SEPERATION,
  UNCLOSED_STRING,
  UNCLOSED_BRACKET,
  UNCLOSED_COMMENT,
  NOT_OPEN_STRING,
  NOT_OPEN_BRACKET,
  NODE_TOO_DEEP,
  NODE_TOO_LONG,
  RESERVED_CHAR,
  WEIRD_BRACKET,
  SYMBOL_TOO_LONG,
  UNMATCHED_BRACKET,
  NO_FOLLOWING_FORMAT_STRING,
  BAD_NUMBER,
  NO_CLUSTER_JOIN,
  MIXED_PATH,
  PARSE_FILE_FAIL,

  EXPECTED_NODE_CONTENT,
  EXPECTED_OPEN_BRACKET,
  EXPECTED_CLOSE_BRACKET,
  EXPECTED_DOT_NODE,
  INCOMPLETE_PARSE,

  TOO_DEEP,
  TOO_LONG,
};

enum NextParse {
  NEXT_NODE_OPEN, NEXT_NODE_CLOSE, NEXT_DOT_NODE/*.*/, NEXT_JOIN /*:*/, NEXT_ATOM, NEXT_WHITESPACE, NEXT_EOF,
};


//TODO: Forward-declare? If it's possible
class ParseError : public HectateError {
public:
  ParseError(String &src, ParseErrorEnum error_type, int index, int line, int column);
  ~ParseError() throw ();
  const wchar_t *error_name();
  bool couldContinue();
  bool noError();
  String &stringError();
  String &get_msg();
  ParseErrorEnum error_type;
private:
  String err_msg;
  String src_line;
  int src_line_arrow;
  int index, line, column;
};

class TrailingWeirdnessValueSave;

class Parser {
  /*
   * See the file SYNTAX for a pseudo-BNF of the grammar
   */
public:
  Node *parse(String src_string);
  Atom parseAtom(String src);
  Node *parseFile(String src_string);
private:
  //vars
  int i, line, column;
  String src;
  Node *final_parse;
  ParseErrorEnum error;
  void init(String &src_string);
  void done();
  int depth;
  bool ignore_trailing_weirdness; //for things like `"foo `.bar"
  friend class TrailingWeirdnessValueSave;

  //Parser functions
  Node *atom_list();
  Atom atom_cluster();
  void atom_cluster_handler(Node *node);
  Atom atom();
  Node *node();

  void read_string(Node *node, wchar_t format);
  String *string();
  Node *format_string();
  Number number();
  Symbol symbol();
  bool skipWhitespace(); //also skips comments.

  NextParse next();


  //Utility functions
  wchar_t get();
  wchar_t peek(unsigned int ahead = 0);
  void advance(int distance);
  bool match(String to_match);
  bool match(String to_match, wchar_t c);
  int can_consume(const wchar_t *to_match);
  bool try_consume(const wchar_t *to_match);
  StringStream *stringStream();
  ParseError make_error(ParseErrorEnum err_enum);
};

#endif /* PARSER_H */

