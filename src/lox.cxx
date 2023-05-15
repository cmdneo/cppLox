#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

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

	auto statements = parser.parse();

	if (lox_had_error)
		return;

	interpreter.interpret(std::move(statements));
}

// bool is_expression_only(string_view line)
// {
// 	Scanner scanner(line);
// 	auto tokens = scanner.scan_tokens();
// }

void run_prompt()
{
	for (string line;;) {
		cout << "> ";
		if (!std::getline(std::cin, line))
			break;

		// TODO
		// If the user enters an expression then try to make that an
		// expression statement and execute that, then print it's result

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
