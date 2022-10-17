#pragma once
#include <fstream>
#include <string>
#include <set>
#include <deque>
#include "Regex.h"

class Interpreter {
public:
	
	Interpreter() {
		Lexer s;
		auto lexems = s.parse_string("MN1 = Glushkov ((ab)*|a)* !!");
	}
private:
	
	class Lexer {
	public:
		struct Lexem {
			enum Type {
				error,
				equalSign,
				doubleExclamation,
				function,
				id,
				regex,
				number,
				predicate,
				test
			};

			Type type = error;
			// ���� type = id | function | predicate
			string value = "";
			// ���� type = number
			int num = 0;

			Lexem(Type type = error, string value = "");
			Lexem(int num);
		};

		Lexer();
		void load_file(string path);
		// ���������� �������, �������� �� ��������
		vector<vector<Lexem>> get_lexems();
		// ���� ������ �� ������� (��� �������� ������)
		vector<Lexem> parse_string(string);
	private:

		// ����� ������ ������ � �����, ������ � ������
		struct {
		public:
			string str = "";
			int pos = 0;

			void save() {
				saves.push_back(pos);
			}
			void restore() {
				pos = saves.back();
				saves.pop_back();
			}
		private:
			deque<int> saves;

		} input;

		bool eof();
		char current_symbol();
		void next_symbol();
		void skip_spaces();
		bool scan_word(string);
		string scan_until_space();

		Lexem scan_equalSign();
		Lexem scan_doubleExclamation();
		Lexem scan_function();
		Lexem scan_id();
		Lexem scan_regex();
		Lexem scan_number();
		Lexem scan_predicate();
		Lexem scan_test();
		Lexem scan_lexem();

		// TODO: ������� ����� �������� ������� � ������� � �������� ������
		// ���-�� ���� ������� map � ���������� ������ ����� set
		set<string> functions = {
			"Thompson",
			"IlieYu",
			"Antimirov",
			"Arden",
			"Glushkov",
			"Determinize",
			"RemEps",
			"Linearize",
			"Minimize",
			"Reverse",
			"Annote",
			"DeLinearize",
			"Complement",
			"MergeBisim",
			"PumpLength",
			"ClassLength",
			"KSubSet",
			"Normalize",
			"States",
			"ClassCard",
			"Ambiguity",
			"Width",
			"MyhillNerode",
			"Simplify",
		};

		set<string> predicates = {
			"Bisimilar",
			"Minimal",
			"Subset",
			"Equiv",
			"Minimal",
			"Equal",
			"SemDet",
		};
	};
};