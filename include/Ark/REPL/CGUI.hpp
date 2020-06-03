#ifndef CGUI_HPP
#define CGUI_HPP

#include <replxx.hxx>

using Replxx = replxx::Replxx;

namespace Ark
{
	const std::vector<std::string> KeywordsDict {
		//variables
		"let", "mut", "set", "true", "false", "nil",
		// control flow
		"while", "if",
		// error
		"throw", "try",
		// list
		"list", "fill", "append",
		// function
		"fun", "return",
		// IO
		"print", "readFile", "input", "writeFile", 
		// REPL commands
		"help", "history", "quit"
	};

	const std::vector<std::pair<std::string, Replxx::Color>> ColorsRegexDict {
		// single chars
		{"\\\"", Replxx::Color::BRIGHTBLUE},
		{"\\-", Replxx::Color::BRIGHTBLUE},
		{"\\+", Replxx::Color::BRIGHTBLUE},
		{"\\=", Replxx::Color::BRIGHTBLUE},
		{"\\/", Replxx::Color::BRIGHTBLUE},
		{"\\*", Replxx::Color::BRIGHTBLUE},
		{"\\[", Replxx::Color::BRIGHTMAGENTA},
		{"\\]", Replxx::Color::BRIGHTMAGENTA},
		{"\\{", Replxx::Color::BRIGHTMAGENTA},
		{"\\}", Replxx::Color::BRIGHTMAGENTA},

		// color keywords
		// error
		{"throw", Replxx::Color::RED},
		{"try", Replxx::Color::GREEN},
		// list
		{"list", Replxx::Color::BLUE},
		{"append", Replxx::Color::BLUE},
		{"fill", Replxx::Color::BLUE},
		// function
		{"fun", Replxx::Color::CYAN},
		{"return", Replxx::Color::MAGENTA},
		// loop and statement
		{"while", Replxx::Color::MAGENTA},
		{"if", Replxx::Color::MAGENTA},
		// variables
		{"let", Replxx::Color::CYAN},
		{"mut", Replxx::Color::CYAN},
		{"set", Replxx::Color::BLUE},
		{"true", Replxx::Color::BRIGHTRED},
		{"false", Replxx::Color::BRIGHTRED},
		{"nil", Replxx::Color::BRIGHTRED},
		// IO
		{"print", Replxx::Color::CYAN},
		{"readFile", Replxx::Color::CYAN},
		{"input", Replxx::Color::BRIGHTGREEN},
		{"writeFile", Replxx::Color::BRIGHTGREEN},

		// commands
		{"help", Replxx::Color::BRIGHTMAGENTA},
		{"history", Replxx::Color::BRIGHTMAGENTA},
		{"quit", Replxx::Color::BRIGHTMAGENTA},
		{"q", Replxx::Color::BRIGHTMAGENTA},

		// numbers
		{"[\\-|+]{0,1}[0-9]+", Replxx::Color::YELLOW},
		{"[\\-|+]{0,1}[0-9]*\\.[0-9]+", Replxx::Color::YELLOW},

		// strings
		{"\".*?\"", Replxx::Color::BRIGHTGREEN}
	};
}

#endif