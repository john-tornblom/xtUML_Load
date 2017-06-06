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

#ifndef SQL_TOKENIZER_H
#define SQL_TOKENIZER_H

#include <stdbool.h>

typedef enum sql_token {
  TOK_INVALID,
  TOK_PUNCTUATION,
  TOK_STRING,
  TOK_UUID,
  TOK_REAL,
  TOK_INTEGER,
  TOK_IDENTIFIER,
  TOK_COMMENT,
  TOK_EOF,
} sql_token_t;

typedef bool (*sql_token_cb)(void *ctx, sql_token_t token, const char* value);

void sql_tokenize_text(sql_token_cb cb, void* ctx, const char* data);
void sql_tokenize_file(sql_token_cb cb, void* ctx, const char* filename);

#endif

