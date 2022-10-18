#include "Interpreter.h"

bool Interpreter::Lexer::eof() {
	return input.pos >= input.str.size();
}

char Interpreter::Lexer::current_symbol() {
	if (input.pos >= 0 && !eof()) {
		return input.str[input.pos];
	}
	return 0;
}

void Interpreter::Lexer::next_symbol() {
	input.pos++;
}

void Interpreter::Lexer::skip_spaces() {
	while (current_symbol() == ' ') {
		next_symbol();
	}
}

bool Interpreter::Lexer::scan_word(string word) {
	skip_spaces();
	int pos_prev = input.pos;
	for (int i = 0; i < word.size(); i++) {
		if (current_symbol() != word[i]) {
			input.pos = pos_prev;
			return false;
		}
		next_symbol();
	}
	return true;
}

string Interpreter::Lexer::scan_until_space() {
	string acc = "";
	skip_spaces();
	while (
		current_symbol() != ' ' &&
		current_symbol() != '\n' &&
		!eof()) {

		acc += current_symbol();
		next_symbol();
	}
	return acc;
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_equalSign() {
	if (scan_word("=")) {
		return Lexem(Lexem::equalSign);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_doubleExclamation() {
	if (scan_word("!!")) {
		return Lexem(Lexem::doubleExclamation);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_function() {
	for (const auto& function : functions) {
		if (scan_word(function)) {
			return Lexem(Lexem::function, function);
		}
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_id() {
	// TODO: ������� �������� �� ������������ �����, ����� ��
	// ���������� � �����, �� ���� �������� � ������� �������
	string id_name = scan_until_space();
	cout << "ID " << id_name << "\n";
	return Lexem(Lexem::id, id_name);
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_regex() {
	// TODO: ��������� regex ����-������
	input.save();
	string word = scan_until_space();
	auto regex = Regex(word);
	if (regex.is_error()) {
		input.restore();
		return Lexem(Lexem::error);
	}
	return Lexem(Lexem::regex, word);
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_number() {
	int pos_prev = input.pos;
	auto is_digit = [](char c) {
		return c >= '0' && c <= '9';
	};
	string acc = "";
	while (!eof() && is_digit(current_symbol())) {
		acc += current_symbol();
	}
	if (acc == "") {
		input.pos = pos_prev;
		return Lexem(Lexem::error);
	}
	return Lexem(stoi(acc));
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_predicate() {
	for (const auto& predicate : predicates) {
		if (scan_word(predicate)) {
			return Lexem(Lexem::predicate, predicate);
		}
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_test() {
	if (scan_word("Test")) {
		return Lexem(Lexem::test);
	}
	return Lexem(Lexem::error);
}

Interpreter::Lexer::Lexem Interpreter::Lexer::scan_lexem() {
	if (Lexem lex = scan_number(); lex.type) { return lex; }
	if (Lexem lex = scan_equalSign(); lex.type) { return lex; }
	if (Lexem lex = scan_doubleExclamation(); lex.type) { return lex; }
	if (Lexem lex = scan_function(); lex.type) { return lex; }
	if (Lexem lex = scan_predicate(); lex.type) { return lex; }
	if (Lexem lex = scan_test(); lex.type) { return lex; }
	if (Lexem lex = scan_regex(); lex.type) { return lex; }
	if (Lexem lex = scan_id(); lex.type) { return lex; }
	return Lexem(Lexem::error);
}

Interpreter::Lexer::Lexem::Lexem(Type type, string value) : type(type), value(value) {}

Interpreter::Lexer::Lexem::Lexem(int num) : num(num), type(number) {}

Interpreter::Lexer::Lexer() {}

vector<vector<Interpreter::Lexer::Lexem>> Interpreter::Lexer::load_file(string path) {
	ifstream input_file(path);
	// ���� ����� ���������� ������ �� ������
	vector<vector<Lexem>> lexem_lines = {};
	string str = "";
	while (getline(input_file, str)) {
		lexem_lines.push_back(parse_string(str));
	}
	return lexem_lines;
}

vector<Interpreter::Lexer::Lexem> Interpreter::Lexer::parse_string(string str) {
	input.str = str;
	input.pos = 0;
	vector<Lexem> lexems;
	while (!eof()) {
 		auto lexem = scan_lexem();
		lexems.push_back(lexem);
	}
	return lexems;
}
