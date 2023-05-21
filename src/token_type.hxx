#ifndef TOKEN_TYPE_HXX_INCLUDED
#define TOKEN_TYPE_HXX_INCLUDED

#include <cassert>
#include <string>

enum class TokenType {
	// Single character tokens
	LEFT_PAREN,
	RIGHT_PAREN,
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	DOT,
	MINUS,
	PLUS,
	SLASH,
	STAR,
	SEMICOLON,
	COLON,
	QUESTION,

	// One or two character tokens
	BANG,
	BANG_EQUAL,
	EQUAL,
	EQUAL_EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,

	// Literals
	IDENTIFIER,
	STRING,
	NUMBER,

	// Keywords
	VAR,
	FUN,
	CLASS,
	SUPER,
	THIS,
	IF,
	ELSE,
	WHILE,
	FOR,
	OR,
	AND,
	ASSERT,
	PRINT,
	RETURN,
	BREAK,
	CONTINUE,
	NIL,
	TRUE,
	FALSE,

	END_OF_FILE,
};

static std::string to_string(TokenType tt)
{
	using enum TokenType;

	switch (tt) {
	case LEFT_PAREN:
		return "LEFT_PAREN";
	case RIGHT_PAREN:
		return "RIGHT_PAREN";
	case LEFT_BRACE:
		return "LEFT_BRACE";
	case RIGHT_BRACE:
		return "RIGHT_BRACE";
	case COMMA:
		return "COMMA";
	case DOT:
		return "DOT";
	case MINUS:
		return "MINUS";
	case PLUS:
		return "PLUS";
	case SLASH:
		return "SLASH";
	case STAR:
		return "STAR";
	case SEMICOLON:
		return "SEMICOLON";
	case COLON:
		return "COLON";
	case QUESTION:
		return "QUESTION";
	case BANG:
		return "BANG";
	case BANG_EQUAL:
		return "BANG_EQUAL";
	case EQUAL:
		return "EQUAL";
	case EQUAL_EQUAL:
		return "EQUAL_EQUAL";
	case GREATER:
		return "GREATER";
	case GREATER_EQUAL:
		return "GREATER_EQUAL";
	case LESS:
		return "LESS";
	case LESS_EQUAL:
		return "LESS_EQUAL";
	case IDENTIFIER:
		return "IDENTIFIER";
	case STRING:
		return "STRING";
	case NUMBER:
		return "NUMBER";
	case VAR:
		return "VAR";
	case FUN:
		return "FUN";
	case CLASS:
		return "CLASS";
	case SUPER:
		return "SUPER";
	case THIS:
		return "THIS";
	case IF:
		return "IF";
	case ELSE:
		return "ELSE";
	case WHILE:
		return "WHILE";
	case FOR:
		return "FOR";
	case OR:
		return "OR";
	case AND:
		return "AND";
	case ASSERT:
		return "ASSERT";
	case PRINT:
		return "PRINT";
	case RETURN:
		return "RETURN";
	case BREAK:
		return "BREAK";
	case CONTINUE:
		return "CONTINUE";
	case NIL:
		return "NIL";
	case TRUE:
		return "TRUE";
	case FALSE:
		return "FALSE";
	case END_OF_FILE:
		return "END_OF_FILE";
	}

	assert(!"Unreachable code");
}

#endif
