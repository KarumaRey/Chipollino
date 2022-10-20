#pragma once
#include <fstream>
#include <string>
#include <variant>
#include <set>
#include <map>
#include <deque>
#include "Regex.h"
#include "Typization.h"
#include "FiniteAutomat.h"

using namespace Typization;

class Interpreter {
public:

	Interpreter() {
		Lexer s;
		auto lexems = s.load_file("test.txt");
	}
private:
	// ���������� ������� � ������ ����������
	GeneralObject apply_function(Function, vector<ObjectType>); // TODO

	// ��� �������� ������� �� �� id
	map<string, GeneralObject> objects;

	// �������� ����������
	// [�������������] = ([�������].)*[�������]? [������]+ (!!)?
	struct Decalaration {
		// �������������, � ������� ��������� ������
		string  id;
		// ���������� �������
		vector<Function> function_sequence;
		// ��������� ���������� ������� (1 ��� �����)
		vector<GeneralObject> parameters;
	};

	// ����������� ����� test
	struct Test {
		// ���������: 
		// ��� ��� ���������� ���������;
		variant<Regex, FiniteAutomat> sample;
		// ���������� ��������� ��� �����������(������ � ��������� �����) � 
		// �������� ���;
		Regex test_set;
		// ����������� ����� � ��� �������� � ����.
		int iterations;
	};

	// �������� [��������] [������]+
	struct Predicate {
		// ������� (��������)
		Function predicate;
		// ���������
		vector<GeneralObject> parameters;
	};

	// ����� ��� ���������
	using GeneralOperation = variant<Decalaration, Test, Predicate>;

	// ���������� �������� �� ������ ������
	Decalaration scan_declaration(vector<Lexem>); // TODO
	Test scan_test(vector<Lexem>); // TODO
	Predicate scan_predicate(vector<Lexem>); // TODO
	GeneralOperation scan_operation(vector<Lexem>); // TODO

	// ���������� ������������������ ������� �� �� ���������
	optional<vector<Function>> build_function_sequence(vector<string> function_names); // TODO

	// ��������� ���� �������; TODO: ���������������� ��� � ������������ Interpreter()
	set<Function> functions; // TODO: ���������� operator< ��� Function
	// ��� ������������ ������� ������ ����� ���������� ������� � �����������
	// ����������, ������������ ��� ���� ����� ��� �������������
	map<string, vector<Function>> names_to_functions;
	// ���������� ���� names_to_functions �� ���� functions
	void generate_function_mapping(); // TODO

	class Lexer {
	public:
		struct Lexem {
			enum Type { // TODO �������� ��� ������ (��� filename)
				error,
				equalSign,
				doubleExclamation,
				function,
				id,
				dot,
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
		vector<vector<Lexem>> load_file(string path);
		// ���������� �������, �������� �� ��������
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
		Lexem scan_dot();
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