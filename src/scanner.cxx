#include <cctype>
#include <vector>
#include <string>

#include "scanner.hxx"
#include "token_type.hxx"

using enum TokenType;

static const std::map<std::string_view, TokenType> KEYWORD_MAP = {
	{"var", VAR},     {"fun", FUN},       {"class", CLASS}, {"super", SUPER},
	{"this", THIS},   {"if", IF},         {"else", ELSE},   {"while", WHILE},
	{"for", FOR},     {"or", OR},         {"and", AND},     {"assert", ASSERT},
	{"print", PRINT}, {"return", RETURN}, {"nil", NIL},     {"true", TRUE},
	{"false", FALSE},
};

void Scanner::do_identifier()
{
	while (std::isalnum(peek()) || peek() == '_')
		advance();

	auto text = source.substr(start, current - start);
	auto type = IDENTIFIER;
	if (auto res = KEYWORD_MAP.find(text); res != KEYWORD_MAP.end())
		type = res->second;

	add_token(type);
}

void Scanner::do_number()
{
	while (std::isdigit(peek()))
		advance();

	if (peek() == '.' && std::isdigit(peek_next()))
		advance(); // Eat the '.;

	while (std::isdigit(peek()))
		advance();

	auto num_text = source.substr(start, current - start);
	double result = 0.0;

	std::from_chars(num_text.cbegin(), num_text.cend(), result);
	add_token(NUMBER, result);
}

void Scanner::do_string()
{
	while (peek() != '"' && !is_at_end())
		advance();

	if (is_at_end())
		print_error(line, "Unterminated string litetral.");

	advance(); // Eat the closing "

	// Remove the quotes
	auto text = source.substr(start + 1, current - start - 2);
	add_token(STRING, std::string(text));
}

void Scanner::scan_token()
{

	char c = advance();
	switch (c) {
	case '(':
		add_token(LEFT_PAREN);
		break;
	case ')':
		add_token(RIGHT_PAREN);
		break;
	case '{':
		add_token(LEFT_BRACE);
		break;
	case '}':
		add_token(RIGHT_BRACE);
		break;
	case ',':
		add_token(COMMA);
		break;
	case '.':
		add_token(DOT);
		break;
	case '-':
		add_token(MINUS);
		break;
	case '+':
		add_token(PLUS);
		break;
	case '*':
		add_token(STAR);
		break;
	case ';':
		add_token(SEMICOLON);
		break;
	case ':':
		add_token(COLON);
		break;
	case '?':
		add_token(QUESTION);
		break;
	case '!':
		add_token(match('=') ? BANG_EQUAL : BANG);
		break;
	case '=':
		add_token(match('=') ? EQUAL_EQUAL : EQUAL);
		break;
	case '<':
		add_token(match('=') ? LESS_EQUAL : LESS);
		break;
	case '>':
		add_token(match('=') ? GREATER_EQUAL : GREATER);
		break;
	case '/':
		if (match('/')) {
			while (peek() != '\n' && !is_at_end())
				advance();
			add_token(COMMENT);
		} else {
			add_token(SLASH);
		}
		break;

	// Ignore whitespace
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		break;

	case '"':
		do_string();
		break;

	default:
		if (std::isdigit(c))
			do_number();
		else if (std::isalpha(c) || c == '_')
			do_identifier();
		else
			print_error(line, std::string("Unexpected character '") + c + "'.");

		break;
	}
}

std::vector<Token> Scanner::scan_tokens()
{

	while (!is_at_end()) {
		start = current;
		scan_token();
	}

	tokens.emplace_back(END_OF_FILE, "", Primitive(), line);
	return tokens;
	// return std::move(tokens);
}