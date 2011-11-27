#ifndef CONFIG_H
#define CONFIG_H

#include "Hectate/Types.h"
#include "Hectate/Atom.h"

#include <math.h>

/*
 * Exclusion rules:
 *      No LTR
 *      No combining
 *      No conflicting
 * */

#define HECTATE_SYNTAX_UNICODE 1

namespace Syntax {
#if HECTATE_SYNTAX_UNICODE
  //Comments
  const String comment = L"#﹟＃"; //number sign; hash
  const String comment_inline = L"*⁎∗✱﹡＊"; //asterisk

  //Nodes
  const String bracket =
    /* ascii brackets ('bracket' is used generaly) */
    L"[]()<>{}"
    /* unicode brackets (so many...) */
      /* parens */
      L"⁽⁾₍₎❨❩❪❫﴾﴿︵︶﹙﹚（）"
      /* square */
      L"⁅⁆【】〔〕〖〗〘〙〚〛［］⦋⦌"
      /* curly */
      L"︷︸﹛﹜｛｝⁅⁆⦃⦄"
      /* angled */
      L"〈〉❬❭❰❱⟨⟩⟪⟫⧼⧽" //NOTE: chevrons are quotation markers
      /* weird */
      L"⟬⟭〔〕"
      /* vertical */
      L"⎴⎵⏞⏟⏠⏡"
      /* misc */
      L"⟅⟆" //not really brackets, but the party wouldn't be the same without them
    ;
  const String bracket_bad = L"⎶⎡⎢⎣⎤⎥⎦⎧⎨⎩⎪⎫⎬⎭⎰⎱༻༽"; //brackets that are beyond us
  const String call = L".។።᙮᠃᠉⳹⳾。︒﹒．｡"; //full stop
  const String join = L"::ː˸፥፦᭝⍠⦂⫶︓﹕："; //colon

#if 1
  //human language string deliminators
  const String string =
    /* ascii */
    L"\'\'\"\""
    /* language */
    L"»»“„﹁﹂„“„”『』””‹›“”’‘﹃﹄’’»«‘’‚‘‚’«»「」›‹"
    /* misc */
    L"⍘⍘⍞⍞❛❛❜❜＂＂"
    ;
#else /* Old string delims */
  //matching string deliminators
  const String string = 
    /* ascii */
    L"\"\"''"
    /* fancy (matching) */
    L"“”‘’«»" //missing a few
    /* misc */
    L"⍘⍘⍞⍞❛❛❜❜＂＂"
    ;
#endif
  const String escape = L"\\"; //reverse solidus. TODO XXX Note: This is limited to 1 char
  const String format = L"`｀`⚰⚱"; //grave accents

  //Symbols
  const String path_sep = L"/⁄∕／"; //solidus
  const String reserved = L"~!@$_|\\,;?"; //For later language expansion (I suppose variants ought to be reserved also.)

#else /* !HECTATE_SYNTAX_UNICODE */
  //be boring
  const String comment = L"#";
  const String comment_inline = L"*";
  const String bracket = L"[]()<>{}";
  const String bracket_bad = L""; //Well, any other bracket would be a bad bracket, but acknowledge only ASCII
  const String call = L".";
  const String join = L":";
  const String string = L"\"\"\'\'";
  const String escape = L"\\";
  const String format = L"`";
  const String path_sep = L"/";
  const String reserved = L"~!@$_|\\,;?";
#endif
  const String invalid_symbol_chars = comment + bracket + call + join + string + format + reserved;
  const String default_string_joiner = L"string-join"; //(default for format)



  //Constants
  struct {
    const Atom value;
    const wchar_t *text;
  } const constants[] = {
    {Atom(INFINITY), L"inf"},
    {Atom(INFINITY), L"+inf"},
    {Atom(-INFINITY), L"-inf"},
    #if HECTATE_SYNTAX_UNICODE
    {Atom(INFINITY), L"∞"},
    {Atom(INFINITY), L"+∞"},
    {Atom(-INFINITY), L"-∞"},
    #endif

    {Atom(NAN), L"nan"},
    {Atom(NAN), L"+nan"},
    {Atom(NAN), L"-nan"},

    {Atom(true), L"true"},
    {Atom(false), L"false"},
    #if HECTATE_SYNTAX_UNICODE
    //TODO: Check out this area for other interesting unicode?
    {Atom(true), L"⊨"},
    {Atom(false), L"⊩"},
    #endif

    {Atom(), L"none"},

    {Atom(), NULL} //sentinal
  };
}

namespace Limit {
  //parsing
  const int parse_depth = 128;
  const unsigned int input_length = 1024*10;

  //evaluation
  const unsigned int eval_depth = 1024;
  const unsigned int include_length = 32;
  
}

//should absolutely be bigger than 'unsigned char'.
//Some debug code will need to be updated if changed to an unsigned type
typedef short RefCountType; 


#endif /* CONFIG_H */

