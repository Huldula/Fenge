#pragma once

#include "fgpch.h"

#define BYTE unsigned char
#define CBYTE const unsigned char
#define CSHORT const unsigned short
#define ADDR unsigned int
#define CADDR const unsigned int

#define WHITESPACE_CHAR_LIST(T) T(' ') \
	T('\t') \
	T('\r') \
	T('\n')

#define IS_WHITESPACE_CHAR(c0) c0 == ' ' || c0 == '\t' || c0 == '\r' || c0 == '\n'
#define IS_NEW_LINE_CHAR(c0) c0 == '\n'
#define IS_DECIMAL_CHAR(c0) c0 >= '0' && c0 <= '9'
#define IS_DECIMAL_DOT_CHAR(c0) IS_DECIMAL_CHAR(c0) || c0 == '.'
#define IS_LETTER_CHAR(c0) c0 >= 'a' && c0 <= 'z' || c0 >= 'A' && c0 <= 'Z'
#define IS_LETTER_DECIMAL_CHAR(c0) IS_LETTER_CHAR(c0) || IS_DECIMAL_CHAR(c0)

#define INSERT_TO_BEGIN(into, other) into.insert(into.begin(), other.begin(), other.end())
#define INSERT_TO_END(into, other) into.insert(into.end(), other.begin(), other.end())
