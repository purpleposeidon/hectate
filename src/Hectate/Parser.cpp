#include "Hectate/Parser.h"

#include <sstream>
#include <wctype.h>
#include <memory>

#include "Hectate/config.h"
#include "Hectate/words.h"

#include "Hectate/Atom.h"
#include "Hectate/Node.h"

#include <iostream>
#include <assert.h>

#include "Hectate/gdb_sucks.h"

//utilifunctions
bool pair_open(const String source, wchar_t c) {
  int l = source.find(c);
  if (l == -1) {
    return false;
  }
  return l % 2 == 0;
}

bool pair_close(const String source, wchar_t c) {
  int l = source.find(c);
  if (l == -1) {
    return false;
  }
  return l % 2 == 1;
}

bool pair_match(const String source, wchar_t open, wchar_t close) {
  if (pair_open(source, open) && pair_close(source, close)) {
    String combined_powers;
    combined_powers += open;
    combined_powers += close;
    return source.find(combined_powers) != String::npos;
    //return (source.find(open) + 1) == source.find(close);
  }
  return false;
}

wchar_t pair_mate(const String source, wchar_t s) {
  if (pair_open(source, s)) {
    return source[source.find(s)+1];
  }
  if (pair_close(source, s)) {
    return source[source.find(s)-1];
  }
  return L'\0';
}

String quote_enders(wchar_t c) {
  String ret;
  for (auto i = Syntax::string.begin(); i != Syntax::string.end(); i += 2) {
    if (*i == c) {
      ret += i[1];
    }
  }
  return ret;
}

bool whitespace_match(wchar_t c) {
  return iswspace(c) || Syntax::comment.find(c) != std::string::npos;
}

class TrailingWeirdnessValueSave {
public:
  TrailingWeirdnessValueSave(Parser &p, bool toset) : parser(p) {
    orig = parser.ignore_trailing_weirdness;
    parser.ignore_trailing_weirdness = toset;
  }
  ~TrailingWeirdnessValueSave() {
    parser.ignore_trailing_weirdness = orig;
  }
private:
  bool orig;
  Parser &parser;
};




//ParseError functions
ParseError::ParseError(String &src, ParseErrorEnum error_type, int index, int line, int column)
: error_type(error_type), index(index), line(line), column(column) {
  //do some scootching over
  if (src[index] == L'\0' && index > 0) {
    index--;
  }
  if (src[index] == L'\n' && index > 0) {
    index--;
  }
  int i = index;
  int line_start = i, line_end = i;
  int head_limit = 60, tail_limit = 60;

  while (line_start > 0 && src[line_start] != L'\n' && head_limit--) {
    line_start--;
  }
  while (src[line_end] != L'\0' && src[line_end] != L'\n' && tail_limit--) {
    line_end++;
  }
  /*
   * [foo b}
   *       ^
   * Syntax Error, line 23 col 7: Unmatched bracket
   */
  src_line.assign(src, line_start, line_end - line_start);
  src_line_arrow = i - line_start;
  if (head_limit == -1) {
    src_line = L"…" + src_line;
    src_line_arrow++;
  }
  if (tail_limit == -1) {
    src_line += L"…";
  }
}
ParseError::~ParseError() throw () {}

const wchar_t *ParseError::error_name() {
  switch (error_type) {
    case NO_ERROR: return L"No Error";
    case EARLY_EOF: return L"Unexpected end of input";
    case NO_SEPERATION: return L"No seperation after atom. (Need whitespace, joiner, or closing bracket)";
    case UNCLOSED_STRING: return L"Unterminated string";
    case UNCLOSED_BRACKET: return L"Unclosed bracket";
    case UNCLOSED_COMMENT: return L"Unclosed comment";
    case NOT_OPEN_STRING: return L"Character can not begin a string"; //TODO
    case NOT_OPEN_BRACKET: return L"Character is not an opening bracket"; //TODO
    case NODE_TOO_DEEP: return L"Max depth of nodes exceeded"; //TODO
    case NODE_TOO_LONG: return L"Too many items in node";
    case SYMBOL_TOO_LONG: return L"Symbol is too long";
    case RESERVED_CHAR: return L"Reserved character";
    case WEIRD_BRACKET: return L"Unable to handle this kind of bracket";
    case UNMATCHED_BRACKET: return L"Closing bracket does not match opening bracket";
    case NO_FOLLOWING_FORMAT_STRING: return L"A string must follow `string-formatter:";
    case BAD_NUMBER: return L"Invalid characters following Number";
    case NO_CLUSTER_JOIN: return L"Expected a cluster-joiner between cluster components";
    case MIXED_PATH: return L"Path has mixed seperator characters";
    case PARSE_FILE_FAIL: return L"Not a Node or Atom-list";

    //These errors shouldn't show up?
    case EXPECTED_NODE_CONTENT: return L"XXX: Expected node content";
    case EXPECTED_OPEN_BRACKET: return L"XXX: Expected open bracket";
    case EXPECTED_CLOSE_BRACKET: return L"XXX: Expected close bracket";
    case EXPECTED_DOT_NODE: return L"XXX: Expected .call";
    case INCOMPLETE_PARSE: return L"XXX: Parse did not consume entire input"; //TODO

    //limits errors
    case TOO_DEEP: return L"Nodes nested too deeply";
    case TOO_LONG: return L"Input is longer than the maximum permitted";
  }
  return L"Unknown error";
}

bool ParseError::couldContinue() {
  switch (error_type) {
    case EARLY_EOF:
    case UNCLOSED_STRING:
    case UNCLOSED_BRACKET:
    case UNCLOSED_COMMENT:
      return true;
    default:
      return false;
  }
}

bool ParseError::noError() {
  return error_type == NO_ERROR;
}

String &ParseError::stringError() {
  if (err_msg.length()) {
    return err_msg;
  }
  
  StringStream out;
  out << "line " << line << " col " << column;
  out << ": " << error_name() << std::endl;
  out << src_line << std::endl;
  out << String(src_line_arrow, L' ') << bold(L"⌃") << std::endl;

  err_msg = out.str();
  return err_msg;
}

String &ParseError::get_msg() {
  return stringError();
}











//Parser utility functions
wchar_t Parser::get() {
  wchar_t r = src[i];
  if (r == L'\0') {
    return r;
  }
  i++;
  if (r == L'\n') {
    line++;
    column = 0;
  }
  else {
    column++;
  }
  return r;
}

wchar_t Parser::peek(unsigned int ahead) {
  if (i + ahead > src.length()) {
    return L'\0';
  }
  return src[i + ahead];
}

void Parser::advance(int distance) {
  if (distance < 0) {
    throw HectateError(L"Parser::advance() got negative distance");
  }
  while (distance) {
    get();
    distance--;
  }
}


bool Parser::match(String to_match) {
  return to_match.find(peek()) != std::string::npos;
}

bool Parser::match(String to_match, wchar_t c) {
  return to_match.find(c) != std::string::npos;
}

int Parser::can_consume(const wchar_t *to_match) {
  int j;
  for (j = 0; to_match[j]; j++) {
    if (towlower(peek(j)) != towlower(to_match[j])) {
      return 0;
    }
  }
  return j;
}

bool Parser::try_consume(const wchar_t *to_match) {
  int j = can_consume(to_match);
  if (j > 0) {
    i += j;
    return true;
  }
  return false;
}


StringStream *Parser::stringStream() {
  return new StringStream(&src[i]);
}

ParseError Parser::make_error(ParseErrorEnum err_enum) {
  ParseError result(src, err_enum, i, line, column);
  result.stringError();
  return result;
}

NextParse Parser::next() {
  //half a tokenizer; tells what type of input is next
  if (whitespace_match(peek())) {
    return NEXT_WHITESPACE;
  }
  if (match(Syntax::bracket)) {
    if (pair_open(Syntax::bracket, peek())) {
      return NEXT_NODE_OPEN;
    }
    else {
      return NEXT_NODE_CLOSE;
    }
  }
  if (match(Syntax::call)) {
    return NEXT_DOT_NODE;
  }
  if (match(Syntax::join)) {
    return NEXT_JOIN;
  }
  if (peek() == L'\0') {
    return NEXT_EOF;
  }
  if (match(Syntax::reserved)) {
    throw make_error(RESERVED_CHAR);
  }
  if (match(Syntax::bracket_bad)) {
    throw make_error(WEIRD_BRACKET);
  }
  return NEXT_ATOM;
}






void Parser::init(String &src_string) {
  i = 0;
  column = 0;
  line = 0;
  error = NO_ERROR;
  depth = 0;
  src = src_string;
  ignore_trailing_weirdness = false;
  if (src.length() > Limit::input_length) {
    throw make_error(TOO_LONG);
  }
  skipWhitespace();
}

void Parser::done() {
  skipWhitespace();
  if (next() != NEXT_EOF) {
    throw make_error(INCOMPLETE_PARSE);
  }
}

Node* Parser::parse(String src_string) {
  init(src_string);
  std::unique_ptr<Node> result;
  
  NextParse n = next();
  if (n == NEXT_ATOM || n == NEXT_JOIN) {
    //+ 2 3
    result = std::unique_ptr<Node>(atom_list());
  }
  else if (n == NEXT_EOF) {
    result = std::unique_ptr<Node>(new Node);
  }
  else {
    //if (n == NEXT_NODE_OPEN || n == NEXT_DOT_CALL || n == NEXT_NODE_CLOSE || n == NEXT_) {
    //[+ 2 3]
    result = std::unique_ptr<Node>(node());
    skipWhitespace();
    if (next() != NEXT_EOF) {
      //has tailing items, so it's actually like [quote +] 23 28
      std::unique_ptr<Node> new_result(new Node);
      Node *append = atom_list();
      new_result->push(Atom::create(result.release()));
      new_result->push(append);
      result = std::unique_ptr<Node>(new_result.release());
      delete append;
    }
  }

  done();
  return result.release();
}

Atom Parser::parseAtom(String src) {
  init(src);
  Atom ret = atom_cluster();
  done();
  return ret;
}

Node* Parser::parseFile(String src_string) {
  init(src_string);
  Node *file_head = new Node;
  file_head->push(Atom::create(Symbol::create(L"flow")));
  file_head->push(Atom::create(Symbol::create(L"do")));
  file_head->setBrackets(L'/', L'\0');

  std::unique_ptr<Node> file(new Node);
  file->push(Atom::create(file_head));
  file->setBrackets(L'{', L'}');

  while (1) {
    skipWhitespace();
    switch (next()) {
      case NEXT_NODE_OPEN:
      case NEXT_DOT_NODE:
      case NEXT_JOIN:
        file->push(Atom::create(node()));
        break;
      case NEXT_ATOM:
        file->push(atom());
        break;
      case NEXT_EOF:
        done();
        return file.release();
      default:
        //XXX: This isn't very good.
        throw make_error(PARSE_FILE_FAIL);
        
    }
  }
}



Node *Parser::atom_list() {
  //TODO: unique_ptr
  std::unique_ptr<Node> result(new Node);
  bool cont = true;
  while (cont) {
    switch (next()) {
      case NEXT_ATOM:
      case NEXT_NODE_OPEN:
      case NEXT_DOT_NODE: //.
      case NEXT_JOIN: //:
        //result->push(Atom::create(atom_cluster()));
        {
          Atom to_add = atom_cluster();
          result->push(to_add);
        }
        break;
      case NEXT_WHITESPACE:
        skipWhitespace();
        break;
      case NEXT_NODE_CLOSE:
      case NEXT_EOF:
        cont = false;
        break;
      default:
        throw HectateError(L"Parser::atom_list doesn't know what to do with next()");
    }
  }
  return result.release();
}

void Parser::atom_cluster_handler(Node *node) {
  // <first?> (':'* Atom)* ':'*
  while (1) {
    auto joiner = next();
    if (joiner == NEXT_WHITESPACE || joiner == NEXT_EOF || joiner == NEXT_NODE_CLOSE) {
      //end of the cluster
      break;
    }
    
    if (joiner == NEXT_JOIN) {
      get(); //remove the ':'
    }
    else if (joiner == NEXT_DOT_NODE) {
      //do nothing: keep the .
    }
    else {
      //foo:bar"invalid"
      throw make_error(NO_CLUSTER_JOIN);
    }
    switch (next()) {
      case NEXT_ATOM:
      case NEXT_NODE_OPEN:
      case NEXT_DOT_NODE:
      {
        Atom a = atom();
        node->push(a);
        break;
      }
      case NEXT_JOIN:
        //[print foo::]
        break;
      case NEXT_WHITESPACE:
      case NEXT_EOF:
      case NEXT_NODE_CLOSE:
        //[print foo:bar  ]
        break;
    }
  }
  node->setBrackets(L':', L'\0');
}

Atom Parser::atom_cluster() {
  if (next() == NEXT_JOIN) {
    TrailingWeirdnessValueSave save_tw(*this, false);

    std::unique_ptr<Node> node(new Node);
    atom_cluster_handler(node.get());
    return Atom::create(node.release());
  }
  Atom first;
  {
    TrailingWeirdnessValueSave save_tw(*this, false);
    first = atom();
  }
  NextParse n = next();
  if (n == NEXT_JOIN || n == NEXT_DOT_NODE) {
    //actually an Atom(Node) rather than Atom(whatever)
    std::unique_ptr<Node> node(new Node);
    node->push(first);
    atom_cluster_handler(node.get());
    return Atom::create(node.release());
  }
  else if (n == NEXT_WHITESPACE || n == NEXT_EOF || n == NEXT_NODE_CLOSE || ignore_trailing_weirdness) {
    return first;
  }
  else {
    throw make_error(NO_SEPERATION);
  }
}

Node *Parser::node() {
  if (depth++ > Limit::parse_depth) {
    throw make_error(TOO_DEEP);
  }
  switch (next()) {
    case NEXT_NODE_OPEN:
      {
        wchar_t open_bracket = get();
        if (!pair_open(Syntax::bracket, open_bracket)) {
          throw make_error(EXPECTED_OPEN_BRACKET);
        }

        Node *contents = atom_list();

        wchar_t close_bracket = get();
        if (!pair_close(Syntax::bracket, close_bracket)) {
          delete contents;
          throw make_error(UNCLOSED_BRACKET);
        }
        if (!pair_match(Syntax::bracket, open_bracket, close_bracket)) {
          i--; //Hack: Move our indicator back a space
          delete contents;
          throw make_error(UNMATCHED_BRACKET);
        }
        contents->setBrackets(open_bracket, close_bracket);
        return contents;
      }
      break;
    case NEXT_DOT_NODE:
      {
        if (!match(Syntax::call)) {
          throw make_error(EXPECTED_DOT_NODE);
        }
        wchar_t dot = get();
        NextParse n = next();
        std::unique_ptr<Node> contents(new Node);
        if (n == NEXT_WHITESPACE || n == NEXT_EOF || n == NEXT_NODE_CLOSE) {
          //leave empty
        }
        else {
          Atom a = atom_cluster();
          if (a.isType(NODE)) {
            contents->push(&a.getNode());
          }
          else {
            contents->push(a);
          }
        }
        contents->setBrackets(dot, L'\0');
        return contents.release();
      }
      break;
    default:
      throw make_error(EXPECTED_NODE_CONTENT);
      break;
  }
}

Atom Parser::atom() {
  //Try Node
  if (match(Syntax::bracket) || match(Syntax::call)) {
    Node *r = node();
    if (r == NULL) {
      throw HectateError(L"Parser::node() returned NULL");
    }
    return Atom::create(r);
  }

  
  //Try constant atoms
  for (int i = 0; Syntax::constants[i].text; i++) {
    if (try_consume(Syntax::constants[i].text)) {
      return Syntax::constants[i].value;
    }
  }

  //Try Number
  if (iswdigit(peek()) || ((peek() == L'-' || peek() == L'+') && iswdigit(peek(1)))) {
    StringStream *buff = stringStream();
    Number num;
    *buff >> num;
    int end_index = buff->tellg();
    delete buff;
    wchar_t end = peek(end_index);
    if (end_index >= 0 && (match(Syntax::invalid_symbol_chars, end) || iswspace(end) || end == L'\0')) {
      advance(end_index);
      return Atom::create(num);
    }
    else {
      //Not actually a number, it's a token that starts with numbers for some awful reason
      throw make_error(BAD_NUMBER);
    }
  }

  //Try String
  if (match(Syntax::string)) {
    Node holder;
    read_string(&holder, L'\0');
    return holder.get(0);
  }
  //Try format string
  if (match(Syntax::format)) {
    //code duplication!
    std::unique_ptr<Node> result(new Node);
    wchar_t format = get();
    if (match(Syntax::string, peek())) {
      result->push(Atom::create(Word::string_join));
    }
    else {
      result->push(atom());
      //`foo:"string" looks better to me than `foo"string"
      while (match(Syntax::join)) {
        get();
      }
    }
    read_string(&(*result), format);
    result->setBrackets(L'`', L'\0');
    return Atom::create(result.release());
  }

  //before defaulting to a symbol, make sure the char's okay
  if (match(Syntax::reserved)) {
    throw make_error(RESERVED_CHAR);
  }
  
  //Default Symbol, or scope path
  String whole_symbol;
  wchar_t path_sep = L'\0';
  while (1) {
    /*if (!path_sep && match(Syntax::path_sep)) {
      path_sep = get();
      whole_symbol += Syntax::path_sep[0];
    }*/
    if (match(Syntax::path_sep)) {
      if (path_sep) {
        if (peek() != path_sep) {
          throw make_error(MIXED_PATH);
        }
        get();
      }
      else {
        path_sep = get();
      }
      whole_symbol += Syntax::path_sep[0];
    }
    else if (match(Syntax::invalid_symbol_chars) || peek() == L'\0' || iswspace(peek())) {
      break;
    }
    else {
      whole_symbol += get();
    }
  }

  if (path_sep) {
    //need to split it up into a Node
    Node *result = new Node;
    try {
      if (match(Syntax::path_sep, whole_symbol[0])) {
        result->push(Atom::create(Word::ScopeRoot));
      }
      StringStream symbol_stream(whole_symbol);
      String sym;
      while (getline(symbol_stream, sym, Syntax::path_sep[0])) {
        if (sym.size() == 0) {
          continue;
        }
        result->push(Atom::create(Symbol::create(sym)));
      }
      result->setBrackets(path_sep, L'\0');
    }
    catch (...) {
      delete result;
      throw;
    }
    return Atom::create(result);
  }
  else {
    return Atom::create(Symbol::create(whole_symbol));
  }
}


void Parser::read_string(Node* node, wchar_t format) {
  wchar_t string_open = get();
  if (!match(Syntax::string, string_open)) {
    throw make_error(NO_FOLLOWING_FORMAT_STRING);
  }
  String closers = quote_enders(string_open);

  String final;
  while (1) {
    wchar_t c = get();
    if (match(closers, c)) {
      break;
    }
    if (c == L'\0') {
      throw make_error(UNCLOSED_STRING);
    }
    if (match(Syntax::escape, c)) {
      c = get();
      switch (c) {
        case L'n':
          c = L'\n';
          break;
        case L't':
          c = L'\t';
          break;
        case L'e':
          c = L'\x1b';
          break;
      }
    }
    else if (format && c == format) {
      if (final.length()) {
        node->push(Atom::create(new String(final)));
        final.erase();
      }
      //TODO: (For consideration) Send users to hell for embedding strings in strings. Especally if the deliminator is the exact same.
      TrailingWeirdnessValueSave save_tw(*this, true);
      node->push(atom());
      continue;
    }
    final += c; //efficieny!
  }
  if (final.length() || format == L'\0') {
    node->push(Atom::create(new String(final)));
  }
}


bool Parser::skipWhitespace() {
  //handles comments and whitespace
  const int orig_i = i;
  while (1) {
    wchar_t c = peek();
    if (c == L'\0') {
      break;
    }
    if (iswspace(c)) {
      get();
      continue;
    }
    if (match(Syntax::comment, c)) {
      //Start of a comment
      wchar_t comment = get();
      wchar_t inline_ = get();
      if (match(Syntax::comment_inline, inline_)) {
        //#*comment*#
        while (1) {
          auto c = peek();
          if (c == L'\0') {
            throw make_error(UNCLOSED_COMMENT);
          }
          if (c == inline_ && peek(1) == comment) {
            get();
            get();
            break;
          }
          get();
        }
      }
      else {
        //#comment
        while (peek() != L'\n' && peek() != L'\0') {
          get();
        }
      }
      continue;
    }
    break;
  }
  return i > orig_i;
}

