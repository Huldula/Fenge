#pragma once

#define OPCODE_LIST \
	T(NOP,	0x0) \
	T(REG,	0x1) \
	T(LWI,	0x2) \
	T(UPI,	0x3) \
	T(LI,	0x4) \
	T(JMP,	0x5) \
	T(JMPC,	0x6) \
	T(LD,	0x7) \
	T(ST,	0x8) \
	T(CALL,	0x9) \
	T(RET,	0xA) \
	T(TBD0,	0xB) \
	T(TBD1,	0xC) \
	T(TBD2,	0xD) \
	T(TBD3,	0xE) \
	T(HLT,	0xF) \

//#define FUNCTION_LIST \
//	T(ILL,	0x00) \
//	T(EQ,	0x10) \
//	T(NEQ,	0x20) \
//	T(GR,	0x30) \
//	T(LS,	0x40) \
//	T(GREQ,	0x50) \
//	T(LSEQ,	0x60) \
//	T(EVEN,	0x70) \
//	T(ODD,	0x80) \
//	T(OR,	0x90) \
//	T(AND,	0xA0) \
//	T(NOT,	0xB0) \
//	T(XOR,	0xC0) \
//	T(TBD00,	0xD0) \
//	T(TBD01,	0xE0) \
//	T(TBD02,	0xF0) \
//	\
//	T(MOV,	0x01) \
//	T(DSPL,	0x11) \
//	T(ADD,	0x02) \
//	T(SUB,	0x03) \
//	T(SHL,	0x04) \
//	T(SHR,	0x05) \
//	T(ROTL,	0x06) \
//	T(ROTR,	0x07) \



// 0xF functions do not exist - just for development
#define FUNCTION_LIST \
	T(ILL,	0x00) \
	T(EQ,	0x01) \
	T(NEQ,	0x02) \
	T(GR,	0x03) \
	T(LS,	0x04) \
	T(GREQ,	0x05) \
	T(LSEQ,	0x06) \
	T(EVEN,	0x07) \
	T(ODD,	0x08) \
	T(OR,	0x09) \
	T(AND,	0x0A) \
	T(NOT,	0x0B) \
	T(XOR,	0x0C) \
	T(TBD0,	0x0D) \
	T(TBD1,	0x0E) \
	T(TBD2,	0x0F) \
	\
	T(MOV,	0x01) \
	T(DSPL,	0x11) \
	T(ADD,	0x20) \
	T(SUB,	0x30) \
	T(SHL,	0x40) \
	T(SHR,	0x50) \
	T(ROTL,	0x60) \
	T(ROTR,	0x70) \
	\
	T(MUL, 0x21) \
	T(DIV, 0xF2) \
	T(MOD, 0xF3) \