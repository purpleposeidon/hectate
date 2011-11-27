#include "loader.h"
#include "common.h"

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>


using namespace std;

#define FANCY_PROMPT 1
//TODO: Have FANCY_PROMPT not output a →

String edit(String &input) {
  string file_name = "/tmp/hectate-loader";
  wofstream o(file_name.c_str());
  o << input.c_str();
  o.close();
  string cmd = string("vi ") + file_name;
  system(cmd.c_str());
  wifstream i(file_name.c_str());

  String ret;
  while (i.good()) {
    String line;
    getline(i, line);
    ret += line;
    ret += L'\n';
  }
  return ret;
}

Node *read_node() {
  static String last;
  while (wcin.good()) {
    //read a node. If we get invalid input, try again.
    #if FANCY_PROMPT
      wcout << bold(L"✶")
            << L"⊃─┤ ";
    #else
      wcout << L"#<--" << endl;
    #endif
    wcout.flush();
    String input;
    bool multiline = false;
    while (1) {
      while (wcin.good()) {
        //Read line
        wchar_t c = wcin.get();
        if (c == L'\0') {
          break;
        }
        if (wcin.eof() || c == -1) {
          wcout << bold(L"EOF") << endl;
          return NULL;
        }
        input += c;
        if (c == L'\n') {
          break;
        }
      }
      if (input[0] == L'~') {
        //XXX this is messy
        //might go away?
        input = edit(last);
        last = input;
      }

      //Parse
      Parser parser;
      const wchar_t *parse_status_indicator = L" ";
      try {
        Node *r = parser.parse(input);
        if (multiline) {
          #if FANCY_PROMPT
            wcout << L"   └"
                  << dim(L"⬢ ")
                  << endl;
          #endif
        }
        last = input;
        return r;
      }
      catch (ParseError &e) {
        if (!e.couldContinue()) {
          wcout << bold(L"Parse failed: ") << e.get_msg();
          last = input;
          break;
        }
        if (e.error_type == UNCLOSED_BRACKET) {
          parse_status_indicator = L"]";
        }
        if (e.error_type == UNCLOSED_STRING) {
          parse_status_indicator = L"\"";
        }
        if (e.error_type == UNCLOSED_COMMENT) {
          parse_status_indicator = L"#";
        }
      }
      
      //next iteration adds another line and re-parses
      #if FANCY_PROMPT
        wcout << L"  " << dim(parse_status_indicator) << L"│ ";
      #else
        //I don't want indents if using a simple prompt
        //wcout << L"  ";
      #endif
      wcout.flush();
      multiline = true;
    }
  }
  return NULL;
}
