#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "error.hxx"
#include "ast_printer.hxx"
#include "scanner.hxx"
#include "parser.hxx"
#include "interpreter.hxx"

using std::cout;
using std::string;
using std::string_view;

// Preserve the interpreter state, throughout the session
static Interpreter interpreter;

// Interpreter entry: Runs the lox-script!
void run_lox_interpreter(string_view source)
{
	Scanner scanner(source);
	Parser parser(scanner.scan_tokens());

	auto expr = parser.parse();

	if (lox_had_error)
		return;

	interpreter.interpret(*expr);
}

void run_prompt()
{
	for (string line;;) {
		cout << "> ";
		if (!std::getline(std::cin, line))
			break;

		run_lox_interpreter(line);
		lox_had_error = false;
		lox_had_runtime_error = false;
		cout << '\n';
	}
}

void run_file(string path)
{
	std::ifstream infile(path);
	if (!infile)
		return;

	string source(std::istreambuf_iterator<char>(infile), {});
	run_lox_interpreter(source);

	if (lox_had_error || lox_had_runtime_error)
		std::exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		run_prompt();
	} else if (argc == 2) {
		run_file(argv[1]);
	} else {
		cout << "Usage: " << argv[0] << " [filename]\n";
		std::exit(EXIT_FAILURE);
	}

	return 0;
}
