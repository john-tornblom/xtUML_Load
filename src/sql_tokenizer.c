/*
   SQL tokenizer

   Copyright (C) 2017 John Törnblom

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>  // fopen(), fmemopen(), feof(), fgetc(), fclose()
#include <stdlib.h> // malloc(), realloc()
#include <assert.h> // assert()
#include <string.h> // strlen()

#include "sql_tokenizer.h"

#define ASSERT(cond, msg) assert(cond && msg)


/**
 * Keep track on the current state.
 */
typedef struct sql_tokenizer {
  sql_token_t  type;
  char*        value;
  unsigned int value_size;
  unsigned int value_pos;
  int          la;
  FILE*        fp;
} sql_tokenizer_t;


/**
 * Check if a character is a punctuation.
 */
static inline bool is_punctuation(char ch) {
  return (ch == ',' || ch == '(' || ch == ')' || ch == ';');
}


/**
 * Check if a character is a space.
 */
static inline bool is_space(char ch) {
  return (ch == ' ' || (ch >= '\t' && ch <= '\r'));
}


/**
 * Check if a character is a digit.
 */
static inline bool is_digit(char ch) {
  return (ch >= '0' && ch <= '9');
}


/**
 * Check if a character is an alphabetic letter.
 */
static inline bool is_alpha(char ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}


/**
 * Scan a character from input.
 */
static bool scan(sql_tokenizer_t *t) {
  if(t->la < 0) {
    return false;
  }
  
  if(t->value_pos >= t->value_size - 1) {
    t->value_size *= 2;
    t->value = realloc(t->value, sizeof(char) * t->value_size);
    ASSERT(t->value, "out of memory");
  }

  t->value[t->value_pos++] = t->la;
  t->value[t->value_pos] = '\0';
  
  if(feof(t->fp)) {
    t->la = -1;
  } else {
    t->la = fgetc(t->fp);
  }

  return true;
}


/**
 * Skip a character from input.
 */
static void skip(sql_tokenizer_t *t) {
  if(feof(t->fp)) {
    t->la = -1;
  } else {
    t->la = fgetc(t->fp);
  }
}


/**
 * Skip blank characters from input.
 */
static void skip_blanks(sql_tokenizer_t *t) {
  while (is_space(t->la)) {
    skip(t);
  }
}


/**
 * Accept a string from input.
 */
static bool accept_string(sql_tokenizer_t *t) {
  if (t->la != '\'') {
    return false;
  }
  skip(t);
  
  while (t->la != '\'') {
    if(!scan(t)) {
      return false;
    }
    
    if(t->la == '\'') {
      skip(t);
      if (t->la == '\'') {
	scan(t);
      } else {
	break;
      }
    }
  }

  if(t->la == '\'') {
    skip(t);
  }
  
  t->type = TOK_STRING;
  return true;
}


/**
 * Accept a unique_id from input.
 */
static bool accept_uuid(sql_tokenizer_t *t) {
  if (t->la != '"') {
    return false;
  }
  skip(t);
  
  while (t->la != '"') {
    ASSERT(scan(t), "syntax error");
  }
  skip(t);

  ASSERT(t->value_pos == 36, "syntax error");
  
  t->type = TOK_UUID;
  return true;
}


/**
 * Accept a punctuation from input.
 */
static bool accept_punctuation(sql_tokenizer_t *t) {
  if(!is_punctuation(t->la)) {
    return false;
  }
  
  scan(t);
  t->type = TOK_PUNCTUATION;
  return true;
}


/**
 * Accept an integer or real from input.
 */
static bool accept_number(sql_tokenizer_t *t) {
  bool dot = false;
  
  if (!is_digit(t->la)) {
    return false;
  }
  
  while (is_digit(t->la) || t->la == '.') {
    ASSERT(!(dot & (t->la == '.')), "syntax error");
    dot |= (t->la == '.');
    scan(t);
  }

  if (dot) {
    t->type = TOK_REAL;
  } else {
    t->type = TOK_INTEGER;
  }
  return true;
}


/**
 * Accept a comment or negative number from input.
 */
static bool accept_comment_or_negative_number(sql_tokenizer_t *t) {
  if (t->la != '-') {
    return false;
  }

  scan(t);
  if(accept_number(t)) {
    return true;
  }

  // drop the scanned '-' character
  t->value_pos--;
  
  ASSERT(t->la == '-', "syntax error");
  skip(t);
  
  while (t->la != '\n' && t->la != '\r' && t->la >= 0) {
    scan(t);
  }
  
  t->type = TOK_COMMENT;
  return true;
}


/**
 * Accept an identifier from input.
 */
static bool accept_identifier(sql_tokenizer_t *t) {
  if (!is_alpha(t->la) && t->la != '_') {
    return false;
  }
  
  while (is_alpha(t->la) || is_digit(t->la) || t->la == '_') {
    scan(t);
  }
  
  t->type = TOK_IDENTIFIER;  
  return true;
}


/**
 * Accept end of file from input.
 */
static bool accept_eof(sql_tokenizer_t *t) {
  if(t->la >= 0) {
    return false;
  }
  
  t->type = TOK_EOF;
  return true;
}


/**
 * Progress input to the next token.
 */
static bool next(sql_tokenizer_t *t) {
  t->type = TOK_INVALID;
  t->value_pos = 0;
  t->value[0] = '\0';

  skip_blanks(t);

  if(accept_eof(t)) {
    return false;
  }
  
  if(accept_string(t)) {
    return true;
  }
  
  if(accept_uuid(t)) {
    return true;
  }
  
  if(accept_punctuation(t)) {
    return true;
  }
  
  if(accept_number(t)) {
    return true;
  }
  
  if(accept_comment_or_negative_number(t)) {
    return true;
  }
  
  if(accept_identifier(t)) {
    return true;
  }
  
  ASSERT(false, "syntax error");
  
  return false;
}


/**
 * Tokenize text and emit tokens to a callback function.
 */
void sql_tokenize_text(sql_token_cb cb, void *ctx, const char* text) {
  sql_tokenizer_t t;
  bool running;
  
  t.fp         = fmemopen((void*)text, strlen(text), "r");
  t.value      = malloc(64);
  t.value_size = 64;
  t.la         = ' ';

  ASSERT(t.fp, "out of memory");
  ASSERT(t.value, "out of memory");

  do {
    running = next(&t);
    running &= cb(ctx, t.type, t.value);
  } while (running);
  
  fclose(t.fp);
  free(t.value);
}


/**
 * Tokenize text in a file and emit tokens to a callback function.
 */
void sql_tokenize_file(sql_token_cb cb, void* ctx, const char* filename) {
  sql_tokenizer_t t;
  bool running;
  
  t.fp         = fopen(filename, "r");
  t.value      = malloc(64);
  t.value_size = 64;
  t.la         = ' ';

  ASSERT(t.fp, "unable to open file");
  ASSERT(t.value, "out of memory");

  do {
    running = next(&t);
    running &= cb(ctx, t.type, t.value);
  } while (running);

  fclose(t.fp);
  free(t.value);
}
