#pragma once

#include "fgpch.h"

#define WHITESPACE_CHAR_LIST(T) T(' ') \
	T('\t') \
	T('\r') \
	T('\n')

#define IS_WHITESPACE_CHAR(c0) c0 == ' ' || c0 == '\t' || c0 == '\r' || c0 == '\n'
#define IS_LETTER_CHAR(c0) c0 >= 'a' && c0 <= 'z' || c0 >= 'A' && c0 <= 'Z'
#define IS_NEW_LINE_CHAR(c0) c0 == '\n'
#define IS_DECIMAL_CHAR(c0) c0 >= '0' && c0 <= '9'
#define IS_DECIMAL_DOT_CHAR(c0) IS_DECIMAL_CHAR(c0) || c0 == '.'
