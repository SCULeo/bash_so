/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IF = 258,
    THEN = 259,
    ELSE = 260,
    ELIF = 261,
    FI = 262,
    CASE = 263,
    ESAC = 264,
    FOR = 265,
    SELECT = 266,
    WHILE = 267,
    UNTIL = 268,
    DO = 269,
    DONE = 270,
    FUNCTION = 271,
    COPROC = 272,
    COND_START = 273,
    COND_END = 274,
    COND_ERROR = 275,
    IN = 276,
    BANG = 277,
    TIME = 278,
    TIMEOPT = 279,
    TIMEIGN = 280,
    WORD = 281,
    ASSIGNMENT_WORD = 282,
    REDIR_WORD = 283,
    NUMBER = 284,
    ARITH_CMD = 285,
    ARITH_FOR_EXPRS = 286,
    COND_CMD = 287,
    AND_AND = 288,
    EQUAL_EQUAL = 289,
    OR_OR = 290,
    GREATER_GREATER = 291,
    LESS_LESS = 292,
    LESS_AND = 293,
    LESS_LESS_LESS = 294,
    GREATER_AND = 295,
    SEMI_SEMI = 296,
    SEMI_AND = 297,
    SEMI_SEMI_AND = 298,
    LESS_LESS_MINUS = 299,
    AND_GREATER = 300,
    AND_GREATER_GREATER = 301,
    LESS_GREATER = 302,
    GREATER_BAR = 303,
    BAR_AND = 304,
    yacc_EOF = 305
  };
#endif
/* Tokens.  */
#define IF 258
#define THEN 259
#define ELSE 260
#define ELIF 261
#define FI 262
#define CASE 263
#define ESAC 264
#define FOR 265
#define SELECT 266
#define WHILE 267
#define UNTIL 268
#define DO 269
#define DONE 270
#define FUNCTION 271
#define COPROC 272
#define COND_START 273
#define COND_END 274
#define COND_ERROR 275
#define IN 276
#define BANG 277
#define TIME 278
#define TIMEOPT 279
#define TIMEIGN 280
#define WORD 281
#define ASSIGNMENT_WORD 282
#define REDIR_WORD 283
#define NUMBER 284
#define ARITH_CMD 285
#define ARITH_FOR_EXPRS 286
#define COND_CMD 287
#define AND_AND 288
#define EQUAL_EQUAL 289
#define OR_OR 290
#define GREATER_GREATER 291
#define LESS_LESS 292
#define LESS_AND 293
#define LESS_LESS_LESS 294
#define GREATER_AND 295
#define SEMI_SEMI 296
#define SEMI_AND 297
#define SEMI_SEMI_AND 298
#define LESS_LESS_MINUS 299
#define AND_GREATER 300
#define AND_GREATER_GREATER 301
#define LESS_GREATER 302
#define GREATER_BAR 303
#define BAR_AND 304
#define yacc_EOF 305

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 329 "parse.y"

  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;

#line 167 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
