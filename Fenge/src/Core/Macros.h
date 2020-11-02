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

#define IS_WHITESPACE_CHAR(c) c == ' ' || c == '\t' || c == '\r' || c == '\n'
#define IS_NEW_LINE_CHAR(c) c == '\n'
#define IS_DECIMAL_CHAR(c) c >= '0' && c <= '9'
#define IS_DECIMAL_DOT_CHAR(c) IS_DECIMAL_CHAR(c) || c == '.'
#define IS_OCTAL_CHAR(c) c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F'
#define IS_LETTER_CHAR(c) c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'
#define IS_LETTER_DECIMAL_CHAR(c) IS_LETTER_CHAR(c) || IS_DECIMAL_CHAR(c)

#define INSERT_TO_BEGIN(into, other) into.insert(into.begin(), other.begin(), other.end())
#define INSERT_TO_END(into, other) into.insert(into.end(), other.begin(), other.end())
