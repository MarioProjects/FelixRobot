%{
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define YY_NO_INPUT
#include <viterbi/parsers/dict-parser/dict-flex.h>
%}


/* Options */
%option noyywrap nounput case-insensitive
%option prefix="dict_"
/* %option debug */

%x INCOMMENT

WSCHAR              [ \t\f\v\r]
OCTDIGIT            [0-7]
HEXDIGIT            [0-9a-fA-F]
DIGIT               [0-9]

                /* Strings */
ESCAPE              (\\[ntvbrfa\n\\?'"])
STRING              \"([^\\\n"]|{ESCAPE})*\"
CHARLIT             \'([^\\\n']|{ESCAPE})*\'

                /* Intergers */
OCTAL               (0{OCTDIGIT}+)
HEXADECIMAL         (0[xX]{HEXDIGIT}+)
INTEGER             [+-]?([1-9]{DIGIT}*)|0

                /* Floats */
EXPONENT            [Ee][+-]?{DIGIT}+
FRACTIONAL          ({DIGIT}+\.)|({DIGIT}*\.{DIGIT}+)
FLOAT               [+-]?{FRACTIONAL}{EXPONENT}?|{DIGIT}+{EXPONENT}

                /* Others */
WHITESPACE          {WSCHAR}+|({WSCHAR}*\\)+{WSCHAR}*
NEWLINE             [\n]

IDENTCHARS          [^ \t\f\v\r\n]
IDENT               {IDENTCHARS}+
%%


%{
  BEGIN(INITIAL);
  errno=0;
%}

"SENT-START"		{ return SENTSTART; }
"["			{ return BRACKET_IN; }
"]"			{ return BRACKET_OUT; }
"SENT-END"		{ return SENTEND; }

"%".*                   { dict_begin_token(yytext); /*return(COMMENT);*/ }

{NEWLINE}               {
                          dict_begin_token(yytext);
                          return ENDL;
                        }

{WHITESPACE}            { dict_begin_token(yytext); /*return(SPACE);*/ }


{OCTAL}		{
                  dict_begin_token(yytext);
                  long n = strtol(yytext+1,NULL,8);
                  if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
                    fprintf(stderr, "integer is out of range");
                  }
	          dict_lval.ival = n;
                  return INT;
		}
{HEXADECIMAL}   {
                  dict_begin_token(yytext);
                  long n = strtol(yytext+2,NULL,16);
                  if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
                    fprintf(stderr, "integer is out of range");
                  }
	          dict_lval.ival = n;
	          return INT;
		}


{IDENT}         {
                  dict_begin_token(yytext);
                  dict_lval.sval = dict_copy_string(yytext, false);
                  return IDENTIFIER;
                }


%%

