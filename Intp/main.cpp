#include <iostream>
#include <fstream>
#include <sstream>

#include "Frontend.h"

int main(void) {
	std::cout << "Hello world!\n";
	std::ifstream file;
	file.open("./test.txt", std::ios::in);
	if (!file) return -1;
	std::string content;
	while (1) {
		char c; file.get(c);
		if (file.eof()) break;
		content += c;
	}
	file.close();
	GScript::Parser parser;
	parser.Parse(content);
}