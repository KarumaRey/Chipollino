#include "Regex.h"
#include "FiniteAutomaton.h"
#include "Language.h"
#include <set>

Lexem::Lexem(Type type, char symbol, int number)
	: type(type), symbol(symbol), number(number) {}

vector<Lexem> Regex::parse_string(string str) {
	vector<Lexem> lexems;
	lexems = {};

	auto is_symbol = [](char c) {
		return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
	};

	for (const char& c : str) {
		Lexem lexem;
		switch (c) {
		case '(':
			lexem.type = Lexem::parL;
			break;
		case ')':
			lexem.type = Lexem::parR;
			break;
		case '|':
			lexem.type = Lexem::alt;
			break;
		case '*':
			lexem.type = Lexem::star;
			break;
		default:
			if (is_symbol(c)) {
				lexem.type = Lexem::symb;
				lexem.symbol = c;
			} else {
				lexem.type = Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			}
			break;
		}

		if (lexems.size() &&
			(
				// Lexem left
				lexems.back().type == Lexem::symb ||
				lexems.back().type == Lexem::star ||
				lexems.back().type == Lexem::parR) &&
			(
				// Lexem right
				lexem.type == Lexem::symb || lexem.type == Lexem::parL)) {

			// We place . between
			lexems.push_back({Lexem::conc});
		}

		if (lexems.size() &&
			((lexems.back().type == Lexem::parL &&
			  (lexem.type == Lexem::parR || lexem.type == Lexem::alt)) ||
			 (lexems.back().type == Lexem::alt && lexem.type == Lexem::parR) ||
			 (lexems.back().type == Lexem::alt && lexem.type == Lexem::alt))) {
			//  We place eps between
			lexems.push_back({Lexem::eps});
		}

		lexems.push_back(lexem);
	}

	if (lexems.size() && lexems[0].type == Lexem::alt) {
		lexems.insert(lexems.begin(), {Lexem::eps});
	}

	if (lexems.back().type == Lexem::alt) {
		lexems.push_back({Lexem::eps});
	}

	int balance = 0;
	for (size_t i = 0; i < lexems.size(); i++) {
		if (lexems[i].type == Lexem::parL) {
			balance++;
		}
		if (lexems[i].type == Lexem::parR) {
			balance--;
		}
	}

	if (balance != 0) {
		lexems = {};
		lexems.push_back({Lexem::error});
		return lexems;
	}

	return lexems;
}

Regex* Regex::scan_conc(const vector<Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::conc && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);

			if (l == nullptr || r == nullptr || r->type == Regex::eps ||
				l->type == Regex::eps) { // Проверка на адекватность)
				return p;
			}

			p = new Regex;
			l->term_p = p;
			r->term_p = p;
			p->term_l = l;
			p->term_r = r;
			p->value = lexems[i];
			p->type = Regex::conc;

			set<alphabet_symbol> s = l->alphabet;
			s.insert(r->alphabet.begin(), r->alphabet.end());
			p->alphabet = s;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_star(const vector<Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::star && balance == 0) {
			Regex* l = expr(lexems, index_start, i);

			if (l == nullptr || l->type == Regex::eps) {
				return p;
			}

			p = new Regex;
			l->term_p = p;
			p->term_l = l;

			p->term_r = nullptr;
			p->value = lexems[i];
			p->type = Regex::star;

			set<alphabet_symbol> s = l->alphabet;
			p->alphabet = s;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_alt(const vector<Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::alt && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);
			// cout << l->type << " " << r->type << "\n";
			if (((l == nullptr) || (r == nullptr)) ||
				(l->type == Regex::eps &&
				 r->type == Regex::eps)) { // Проверка на адекватность)
				return nullptr;
			}

			p = new Regex;
			l->term_p = p;
			r->term_p = p;
			p->term_l = l;
			p->term_r = r;

			p->value = lexems[i];
			p->type = Regex::alt;

			set<alphabet_symbol> s = l->alphabet;
			s.insert(r->alphabet.begin(), r->alphabet.end());

			p->alphabet = s;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_symb(const vector<Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	if (lexems.size() <= (index_start) ||
		lexems[index_start].type != Lexem::symb) {
		return nullptr;
	}
	p = new Regex;
	p->value = lexems[index_start];
	p->type = Regex::symb;

	vector<alphabet_symbol> v = {lexems[index_start].symbol};
	set<alphabet_symbol> s(v.begin(), v.end());

	p->alphabet = s;
	p->term_l = nullptr;
	p->term_r = nullptr;
	return p;
}

Regex* Regex::scan_eps(const vector<Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;
	// cout << lexems[index_start].type << "\n";
	if (lexems.size() <= (index_start) ||
		lexems[index_start].type != Lexem::eps) {
		return nullptr;
	}
	p = new Regex;
	p->value = lexems[index_start];
	p->type = Regex::eps;

	set<alphabet_symbol> s;
	// l = new Language(s);

	p->alphabet = s;
	p->term_l = nullptr;
	p->term_r = nullptr;
	return p;
}

Regex* Regex::scan_par(const vector<Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;

	if (lexems.size() <= (index_end - 1) ||
		(lexems[index_start].type != Lexem::parL ||
		 lexems[index_end - 1].type != Lexem::parR)) {
		return nullptr;
	}
	p = expr(lexems, index_start + 1, index_end - 1);
	return p;
}
Regex* Regex::expr(const vector<Lexem>& lexems, int index_start,
				   int index_end) {
	Regex* p;
	p = scan_alt(lexems, index_start, index_end);
	if (!p) {
		p = scan_conc(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_star(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_symb(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_eps(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_par(lexems, index_start, index_end);
	}
	return p;
}
Regex::Regex() {
	type = Regex::eps;
	term_l = nullptr;
	term_r = nullptr;
}

bool Regex::from_string(string str) {
	vector<Lexem> l = parse_string(str);
	Regex* root = expr(l, 0, l.size());

	if (root == nullptr || root->type == eps) {
		return false;
	}

	//*this = *(root->copy());
	value = root->value;
	type = root->type;
	alphabet = root->alphabet;
	language = shared_ptr<Language>(new Language(alphabet));
	if (root->term_l != nullptr) {
		term_l = root->term_l->copy();
		term_l->term_p = this;
	}
	if (root->term_r != nullptr) {
		term_r = root->term_r->copy();
		term_r->term_p = this;
	}
	delete root;
	return true;
}
void Regex::regex_union(Regex* a, Regex* b) {
	type = Type::conc;
	term_l = a->copy();
	term_r = b->copy();
}
void Regex::regex_alt(Regex* a, Regex* b) {
	type = Type::alt;
	term_l = a->copy();
	term_r = b->copy();
}
void Regex::regex_star(Regex* a) {
	type = Type::star;
	term_l = a->copy();
}
void Regex::regex_eps() {
	type = Type::eps;
}
Regex* Regex::copy() const {
	Regex* c = new Regex();
	c->type = type;
	c->value = value;
	c->language = language;
	if (type != Regex::eps && type != Regex::symb) {
		c->term_l = term_l->copy();
		c->term_l->term_p = c;
		if (type != Regex::star) {
			c->term_r = term_r->copy();
			c->term_r->term_p = c;
		}
	}
	return c;
}

Regex::Regex(const Regex& reg)
	: BaseObject(reg.language), type(reg.type), value(reg.value),
	  term_p(reg.term_p), alphabet(reg.alphabet),
	  term_l(reg.term_l == nullptr ? nullptr : reg.term_l->copy()),
	  term_r(reg.term_r == nullptr ? nullptr : reg.term_r->copy()) {}

void Regex::clear() {
	if (term_l != nullptr) {
		// term_l->clear();
		delete term_l;
	}
	if (term_r != nullptr) {
		// term_r->clear();
		delete term_r;
	}
	// delete language;
}

Regex::~Regex() {
	clear();
}

void Regex::pre_order_travers() {
	if (value.symbol) {
		cout << value.symbol << " ";
	} else {
		cout << type << " ";
	}
	if (term_l) {
		term_l->pre_order_travers();
	}
	if (term_r) {
		term_r->pre_order_travers();
	}
}

string Regex::to_txt() const {
	string str1 = "", str2 = "";
	if (term_l) {
		str1 = term_l->to_txt();
	}
	if (term_r) {
		str2 = term_r->to_txt();
	}
	string symb;
	if (value.symbol) symb = value.symbol;
	if (type == Type::eps) symb = "";
	if (type == Type::alt) {
		symb = '|';
		if (term_p != nullptr && term_p->type == Type::conc) {
			str1 = "(" + str1;
			str2 = str2 + ")"; // ставим скобки при альтернативах внутри
							   // конкатенации a(a|b)a
		}
	}
	if (type == Type::star) {
		symb = '*';
		if (term_l->type != Type::symb)
			str1 = "(" + str1 +
				   ")"; // ставим скобки при итерации, если символов > 1
	}

	return str1 + symb + str2;
}
// возвращает пару <вектор сотсояний, max_index>
pair<vector<State>, int> Regex::get_tompson(int max_index) {
	string str;			  //идентификатор состояния
	vector<State> s = {}; //вектор состояний нового автомата
	map<alphabet_symbol, set<int>> m, p, map_l, map_r; // словари автоматов
	set<int> trans; // новые транзишены
	int offset; // сдвиг для старых индексов состояний в новом автомате
	pair<vector<State>, int> al; // для левого автомата относительно операции
	pair<vector<State>, int> ar; // для правого автомата относительно операции
	Language* alp;				 // Новый язык для автомата
	switch (type) {
	case Regex::alt: // |

		al = term_l->get_tompson(max_index);
		ar = term_r->get_tompson(al.second);
		max_index = ar.second;

		str = "q" + to_string(max_index + 1);
		m['\0'] = {1, int(al.first.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.first.size(); i++) {
			State test = al.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l['\0'] = {int(al.first.size() + ar.first.size()) + 1};
			}
			s.push_back(State(al.first[i].index + 1, {}, al.first[i].identifier,
							  false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 0; i < ar.first.size(); i++) {
			State test = ar.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first;
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + offset);
				}
				map_r[elem] = trans;
			}
			if (test.is_terminal) {
				map_r['\0'] = {offset + int(ar.first.size())};
			}

			s.push_back(State(ar.first[i].index + offset, {},
							  ar.first[i].identifier, false, map_r));
			map_r = {};
		}

		str = "q" + to_string(max_index + 2);
		s.push_back(State(int(al.first.size() + ar.first.size()) + 1, {}, str,
						  true, p));

		return {s, max_index + 2};
	case Regex::conc: // .
		al = term_l->get_tompson(max_index);
		ar = term_r->get_tompson(al.second);
		max_index = ar.second;

		for (size_t i = 0; i < al.first.size(); i++) {
			State test = al.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				State test_r = ar.first[0];
				for (auto el : test_r.transitions) {
					alphabet_symbol elem = el.first; // al->alphabet[i];
					for (int transition_to : test_r.transitions[elem]) {
						// trans.push_back(test.transitions[elem][j] + 1);
						map_l[elem].insert(transition_to + al.first.size() - 1);
					}
					// map_l[elem] = trans;
				}
			}
			// cout << al->states[i].identifier << " " << al->states[i].index
			// <<"\n";
			s.push_back(State(al.first[i].index, {}, al.first[i].identifier,
							  false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 1; i < ar.first.size(); i++) {
			State test = ar.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				// alfa.push_back(elem);
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + offset - 1);
				}
				map_r[elem] = trans;
			}

			s.push_back(State(ar.first[i].index + offset - 1, {},
							  ar.first[i].identifier, test.is_terminal, map_r));
			map_r = {};
		}

		return {s, max_index};
	case Regex::star: // *
		al = term_l->get_tompson(max_index);
		max_index = al.second;

		str = "q" + to_string(max_index + 1);
		m['\0'] = {1, int(al.first.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.first.size(); i++) {
			State test;
			test = al.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l['\0'] = {1, int(al.first.size()) + 1};
			}
			s.push_back(State(al.first[i].index + 1, {}, al.first[i].identifier,
							  false, map_l));
			map_l = {};
		}
		offset = s.size();

		str = "q" + to_string(max_index + 2);
		s.push_back(State(int(al.first.size()) + 1, {}, str, true, p));

		return {s, max_index + 2};
	case Regex::eps:
		str = "q" + to_string(max_index + 1);

		m['\0'] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		return {s, max_index + 2};
	default:

		str = "q" + to_string(max_index + 1);
		m[value.symbol] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		return {s, max_index + 2};
	}
	return {};
}

FiniteAutomaton Regex::to_tompson() {
	return FiniteAutomaton(0, get_tompson(-1).first, language);
}

int Regex::L() {
	int l;
	int r;
	switch (type) {
	case Regex::alt:
		l = term_l->L();
		r = term_r->L();
		return l + r;
	case Regex::conc:
		l = term_l->L();
		r = term_r->L();
		if (l != 0 && r != 0) {
			return 1;
		}
		return 0;
	case Regex::star:
		return 1;
	case Regex::eps:
		return 1;
	default:
		return 0;
	}
}
vector<Lexem>* Regex::first_state() {
	vector<Lexem>* l;
	vector<Lexem>* r;
	switch (type) {
	case Regex::alt:
		l = term_l->first_state();
		r = term_r->first_state();
		l->insert(l->end(), r->begin(), r->end());
		delete r;
		return l;
	case Regex::star:
		l = term_l->first_state();
		return l;
	case Regex::conc:
		l = term_l->first_state();
		if (term_l->L() != 0) {
			r = term_r->first_state();
			l->insert(l->end(), r->begin(), r->end());
			delete r;
		}
		//
		return l;
	case Regex::eps:
		l = new vector<Lexem>;
		return l;
	default:
		l = new vector<Lexem>;
		l->push_back(value);
		return l;
	}
}

vector<Lexem>* Regex::end_state() {
	vector<Lexem>* l;
	vector<Lexem>* r;
	switch (type) {
	case Regex::alt:
		l = term_l->end_state();
		r = term_r->end_state();
		l->insert(l->end(), r->begin(), r->end());
		delete r;
		return l;
	case Regex::star:
		l = term_l->end_state();
		return l;
	case Regex::conc:
		l = term_r->end_state();
		if (term_r->L() != 0) {

			r = term_l->end_state();
			l->insert(l->end(), r->begin(), r->end());
			delete r;
		}
		return l;
	case Regex::eps:
		l = new vector<Lexem>;
		return l;
	default:
		l = new vector<Lexem>;
		l->push_back(value);
		return l;
	}
}

map<int, vector<int>> Regex::pairs() {
	map<int, vector<int>> l;
	map<int, vector<int>> r;
	map<int, vector<int>> p;
	vector<Lexem>* rs;
	vector<Lexem>* ps;
	switch (type) {
	case Regex::alt:
		l = term_l->pairs();
		r = term_r->pairs();
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		return l;
	case Regex::star:
		l = term_l->pairs();
		rs = term_l->end_state();
		ps = term_l->first_state();
		for (size_t i = 0; i < rs->size(); i++) {
			for (size_t j = 0; j < ps->size(); j++) {
				r[(*rs)[i].number].push_back((*ps)[j].number);
			}
		}
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		delete rs;
		delete ps;
		return l;
	case Regex::conc:
		l = term_l->pairs();
		r = term_r->pairs();
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		r = {};
		rs = term_l->end_state();
		ps = term_r->first_state();

		for (size_t i = 0; i < rs->size(); i++) {
			for (size_t j = 0; j < ps->size(); j++) {
				r[(*rs)[i].number].push_back((*ps)[j].number);
			}
		}
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		delete rs;
		delete ps;
		return l;
	default:
		break;
	}
	return {};
}

vector<Regex*> Regex::pre_order_travers_vect() {
	vector<Regex*> r;
	vector<Regex*> ret;
	if (value.symbol) {
		r = {};
		r.push_back(this);
		return r;
	}
	r = {};
	if (term_l) {
		ret = term_l->pre_order_travers_vect();
		r.insert(r.end(), ret.begin(), ret.end());
	}
	if (term_r) {
		ret = term_r->pre_order_travers_vect();
		r.insert(r.end(), ret.begin(), ret.end());
	}
	return r;
}
bool Regex::is_term(int number, const vector<Lexem>& list) {
	for (size_t i = 0; i < list.size(); i++) {
		if (list[i].number == number) {
			return true;
		}
	}
	return false;
}
FiniteAutomaton Regex::to_glushkov() {

	vector<Regex*> list = this->pre_order_travers_vect();
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.number = i;
	}
	vector<Lexem>* first = this->first_state(); // Множество начальных состояний
	vector<Lexem>* end = this->end_state(); // Множество конечных состояний
	map<int, vector<int>> p =
		this->pairs(); // Множество возможных пар состояний
	vector<State> st; // Список состояний в автомате
	map<alphabet_symbol, set<int>> tr; // мап для переходов в каждом состоянии

	for (size_t i = 0; i < first->size(); i++) {
		tr[(*first)[i].symbol].insert((*first)[i].number + 1);
	}

	st.push_back(State(0, {}, "S", false, tr));

	for (size_t i = 0; i < list.size(); i++) {
		Lexem elem = list[i]->value;
		tr = {};

		for (size_t j = 0; j < p[elem.number].size(); j++) {
			tr[list[p[elem.number][j]]->value.symbol].insert(p[elem.number][j] +
															 1);
		}
		string s = elem.symbol + to_string(i + 1);
		st.push_back(State(i + 1, {}, s, is_term(elem.number, (*end)), tr));
	}
	delete first;
	delete end;
	return FiniteAutomaton(0, st, language);
}

FiniteAutomaton Regex::to_ilieyu() {
	FiniteAutomaton glushkov = this->to_glushkov();
	vector<State> states = glushkov.states;
	vector<int> follow;
	for (size_t i = 0; i < states.size(); i++) {
		State st1 = states[i];
		map<alphabet_symbol, set<int>> map1 = st1.transitions;
		for (size_t j = i + 1; j < states.size(); j++) {
			State st2 = states[j];
			map<alphabet_symbol, set<int>> map2 = st2.transitions;
			bool flag = true;
			if (i == j || map2.size() != map1.size()) {
				continue;
			}

			for (auto& it1 : map1) {
				set<int> v1 = it1.second;
				set<int> v2 = map2[it1.first];
				if (v1 != v2 /*equal(v1.begin(), v1.end(), v2.begin())*/) {
					flag = false;
					break;
				}
			}
			if (flag) {
				follow.push_back(j);
				states[i].label.insert(j);
			}
		}
	}
	vector<State> new_states;
	for (size_t i = 0; i < states.size(); i++) {
		if (find(follow.begin(), follow.end(), states[i].index) ==
			follow.end()) {
			new_states.push_back(states[i]);
		}
	}

	for (size_t i = 0; i < new_states.size(); i++) {
		State v1 = new_states[i];
		map<alphabet_symbol, set<int>> old_map = v1.transitions;
		map<alphabet_symbol, set<int>> new_map;
		for (auto& it1 : old_map) {
			set<int> v1 = it1.second;
			for (int transition_to : v1) {
				for (size_t k = 0; k < new_states.size(); k++) {
					if (new_states[k].label.find(transition_to) !=
							new_states[k].label.end() ||
						transition_to == new_states[k].index) {
						new_map[it1.first].insert(k);
					}
				}
			}
		}
		new_states[i].transitions = new_map;
	}

	for (size_t i = 0; i < new_states.size(); i++) {
		new_states[i].index = i;
	}

	return FiniteAutomaton(0, new_states, glushkov.language);
}
bool Regex::is_eps_possible() {
	switch (type) {
	case Type::eps:
		return true;
	case Type::symb:
		return false;
	case Type::alt:
		return term_l->is_eps_possible() || term_r->is_eps_possible();
	case Type::conc:
		return term_l->is_eps_possible() && term_r->is_eps_possible();
	case Type::star:
		return true;
	default:
		return false;
	}
}

void Regex::get_prefix(int len, std::set<std::string>* prefs) const {
	std::set<std::string>*prefs1, *prefs2;
	switch (type) {
	case Type::eps:
		if (len == 0) prefs->insert("");
		return;
	case Type::symb:
		if (len == 1) {
			std::string res = "";
			res += value.symbol;
			prefs->insert(res);
		}
		return;
	case Type::alt:
		prefs1 = new std::set<std::string>();
		prefs2 = new std::set<std::string>();
		term_l->get_prefix(len, prefs1);
		term_r->get_prefix(len, prefs2);
		for (auto i = prefs1->begin(); i != prefs1->end(); i++) {
			prefs->insert(*i);
		}
		for (auto i = prefs2->begin(); i != prefs2->end(); i++) {
			prefs->insert(*i);
		}
		delete prefs1;
		delete prefs2;
		return;
	case Type::conc:
		prefs1 = new std::set<std::string>();
		prefs2 = new std::set<std::string>();
		for (int k = 0; k <= len; k++) {
			term_l->get_prefix(k, prefs1);
			term_r->get_prefix(len - k, prefs2);
			for (auto i = prefs1->begin(); i != prefs1->end(); i++) {
				for (auto j = prefs2->begin(); j != prefs2->end(); j++) {
					prefs->insert(*i + *j);
				}
			}
			prefs1->clear();
			prefs2->clear();
		}
		delete prefs1;
		delete prefs2;
		return;
	case Type::star:
		if (len == 0) {
			prefs->insert("");
			return;
		}
		prefs1 = new std::set<std::string>();
		prefs2 = new std::set<std::string>();
		for (int k = 1; k <= len; k++) {
			term_l->get_prefix(k, prefs1);
			get_prefix(len - k, prefs2);
			for (auto i = prefs1->begin(); i != prefs1->end(); i++) {
				for (auto j = prefs2->begin(); j != prefs2->end(); j++) {
					prefs->insert(*i + *j);
				}
			}
			prefs1->clear();
			prefs2->clear();
		}
		delete prefs1;
		delete prefs2;
		return;
	}
}

string Regex::to_str() const {
	string str1 = "", str2 = "";
	if (term_l) {
		str1 = term_l->to_str();
	}
	if (term_r) {
		str2 = term_r->to_str();
	}
	string symb;
	if (value.symbol) symb = value.symbol;
	if (type == Type::eps) symb = "";
	if (type == Type::alt) {
		symb = '|';
		if (term_p != nullptr && term_p->type == Type::conc) {
			str1 = "(" + str1;
			str2 = str2 + ")"; // ставим скобки при альтернативах внутри
							   // конкатенации a(a|b)a
		}
	}
	if (type == Type::star) {
		symb = '*';
		if (term_l->type != Type::symb)
			str1 = "(" + str1 +
				   ")"; // ставим скобки при итерации, если символов > 1
	}

	return str1 + symb + str2;
}

bool Regex::derevative_with_respect_to_sym(Regex* respected_sym,
										   const Regex* reg_e,
										   Regex& result) const {
	if (respected_sym->type != Type::eps && respected_sym->type != Type::symb) {
		std::cout << "Invalid input: unexpected regex instead of symbol\n";
		return false;
	}
	if (respected_sym->type == Type::eps) {
		result = *reg_e;
		return true;
	}
	Regex subresult, subresult1;
	bool answer = true, answer1, answer2;
	switch (reg_e->type) {
	case Type::eps:
		if (respected_sym->type != Type::eps) return false;
		result.type = Type::eps;
		return answer;
	case Type::symb:
		if (respected_sym->value.symbol != reg_e->value.symbol) {
			return false;
		}
		result.type = Type::eps;
		return answer;
	case Type::alt:
		answer1 = derevative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 subresult);
		answer2 = derevative_with_respect_to_sym(respected_sym, reg_e->term_r,
												 subresult1);
		if (answer1 && answer2) {
			result.type = Type::alt;
			result.term_l = subresult.copy();
			result.term_r = subresult1.copy();
		}
		if (!answer1 && !answer2) {
			return false;
		}
		if (answer1) {
			result = subresult;
		}
		if (answer2) {
			result = subresult1;
		}
		return answer;
	case Type::conc:
		subresult.type = Type::conc;
		if (subresult.term_l == nullptr) subresult.term_l = new Regex();
		answer1 = derevative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 *subresult.term_l);
		subresult.term_r = reg_e->term_r->copy();
		if (reg_e->term_l->is_eps_possible()) {
			answer2 = derevative_with_respect_to_sym(respected_sym,
													 reg_e->term_r, subresult1);
			if (answer1 && answer2) {
				result.type = Type::alt;
				result.term_l = subresult.copy();
				result.term_r = subresult1.copy();
			}
			if (answer1 && !answer2) {
				result.type = subresult.type;
				if (subresult.term_l != nullptr)
					result.term_l = subresult.term_l->copy();
				if (subresult.term_r != nullptr)
					result.term_r = subresult.term_r->copy();
			}
			if (answer2 && !answer1) {
				result.type = subresult1.type;
				if (subresult1.term_l != nullptr)
					result.term_l = subresult1.term_l->copy();
				if (subresult1.term_r != nullptr)
					result.term_r = subresult1.term_r->copy();
			}
			answer = answer1 | answer2;
		} else {
			answer = answer1;
			result.type = subresult.type;
			if (subresult.term_l != nullptr)
				result.term_l = subresult.term_l->copy();
			if (subresult.term_r != nullptr)
				result.term_r = subresult.term_r->copy();
		}
		return answer;
	case Type::star:
		result.type = Type::conc;
		if (result.term_l == nullptr) result.term_l = new Regex();
		bool answer = derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, *result.term_l);
		result.term_r = reg_e->copy();
		return answer;
	}
}

bool Regex::partial_derevative_with_respect_to_sym(
	Regex* respected_sym, const Regex* reg_e, vector<Regex>& result) const {
	Regex cur_result;
	if (respected_sym->type != Type::eps && respected_sym->type != Type::symb) {
		std::cout << "Invalid input: unexpected regex instead of symbol\n";
		return false;
	}
	if (respected_sym->type == Type::eps) {
		cur_result.type = reg_e->type;
		if (reg_e->term_l != nullptr) cur_result.term_l = reg_e->term_l->copy();
		if (reg_e->term_l) cur_result.term_r = reg_e->term_l->copy();
		result.push_back(cur_result);
		return true;
	}
	Regex cur_subresult, cur_subresult1;
	vector<Regex> subresult, subresult1;
	bool answer = true, answer1, answer2;
	switch (reg_e->type) {
	case Type::eps:
		return false;
	case Type::symb:
		if (respected_sym->value.symbol != reg_e->value.symbol) {
			return false;
		}
		cur_result.type = Type::eps;
		result.push_back(cur_result);
		return answer;
	case Type::alt:
		answer1 = partial_derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		answer2 = partial_derevative_with_respect_to_sym(
			respected_sym, reg_e->term_r, subresult1);
		for (int i = 0; i < subresult.size(); i++) {
			result.push_back(subresult[i]);
		}
		for (int i = 0; i < subresult1.size(); i++) {
			result.push_back(subresult1[i]);
		}
		answer = answer1 | answer2;
		return answer;
	case Type::conc:
		cur_subresult.type = Type::conc;
		answer1 = partial_derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		cur_subresult.term_r = reg_e->term_r->copy();
		for (int i = 0; i < subresult.size(); i++) {
			cur_subresult.term_l = subresult[i].copy();
			result.push_back(cur_subresult);
			delete cur_subresult.term_l;
			cur_subresult.term_l = nullptr;
		}
		if (reg_e->term_l->is_eps_possible()) {
			answer2 = partial_derevative_with_respect_to_sym(
				respected_sym, reg_e->term_r, subresult1);
			for (int i = 0; i < subresult1.size(); i++) {
				result.push_back(subresult1[i]);
			}
			answer = answer1 | answer2;
		} else {
			answer = answer1;
		}
		return answer;
	case Type::star:
		cur_result.type = Type::conc;
		bool answer = partial_derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		cur_result.term_r = reg_e->copy();
		for (int i = 0; i < subresult.size(); i++) {
			cur_result.term_l = subresult[i].copy();
			result.push_back(cur_result);
			delete cur_result.term_l;
			cur_result.term_l = nullptr;
		}
		cur_result.clear();
		return answer;
	}
}

bool Regex::derevative_with_respect_to_str(std::string str, const Regex* reg_e,
										   Regex& result) const {
	bool success = true;
	Regex cur = *reg_e;
	Regex next;
	for (int i = 0; i < str.size(); i++) {
		auto sym = new Regex();
		sym->type = Type::symb;
		sym->value.symbol = str[i];
		success &= derevative_with_respect_to_sym(sym, &cur, next);
		delete sym;
		cur = next;
		if (!success) {
			break;
		}
	}
	result = next;
	cout << " answer is " << result.to_str();
	return success;
}

// Производная по символу
std::optional<Regex> Regex::symbol_derevative(
	const Regex& respected_sym) const {
	auto rs = respected_sym.copy();
	Regex result;
	std::optional<Regex> ans;
	if (derevative_with_respect_to_sym(rs, this, result))
		ans = result;
	else
		ans = nullopt;
	delete rs;
	return ans;
}
// Частичная производная по символу
void Regex::partial_symbol_derevative(const Regex& respected_sym,
									  vector<Regex>& result) const {
	auto rs = respected_sym.copy();
	partial_derevative_with_respect_to_sym(rs, this, result);
	delete rs;
	return;
}
// Производная по префиксу
std::optional<Regex> Regex::prefix_derevative(std::string respected_str) const {
	Regex result;
	std::optional<Regex> ans;
	if (derevative_with_respect_to_str(respected_str, this, result))
		ans = result;
	else
		ans = nullopt;
	return ans;
}
// Длина накачки
int Regex::pump_length() const {
	std::map<std::string, bool> checked_prefixes;
	for (int i = 0;; i++) {
		std::set<std::string> prefs;
		get_prefix(i, &prefs);
		for (auto it = prefs.begin(); it != prefs.end(); it++) {
			bool was = false;
			for (int j = 0; j < it->size(); j++) {
				if (checked_prefixes[it->substr(0, j)]) {
					was = true;
					break;
				}
			}
			if (was) continue;
			for (int j = 0; j < it->size(); j++) {
				for (int k = j + 1; k < it->size(); k++) {
					Regex pumping;
					std::string pumped_prefix;
					pumped_prefix += it->substr(0, j);
					pumped_prefix += "(" + it->substr(j, k - j) + ")*";
					pumped_prefix += it->substr(j, it->size() - k);
					pumping.type = Type::conc;
					pumping.term_l = new Regex;
					pumping.term_l->from_string(pumped_prefix);
					pumping.term_r = new Regex;
					derevative_with_respect_to_str(*it, this, *pumping.term_r);
					if (true) { // TODO: check if pumping language belongs reg_e
								// language
						checked_prefixes[*it] = true;
						return i;
					}
				}
			}
		}
	}
	return -1;
}

bool Regex::equal(Regex* r1, Regex* r2) {
	if (r1 == nullptr && r2 == nullptr) return true;
	if (r1 == nullptr || r2 == nullptr) return true;
	int r1_value, r2_value;
	if (r1->value.symbol)
		r1_value = (int)r1->value.symbol;
	else
		r1_value = r1->type;
	if (r2->value.symbol)
		r2_value = (int)r2->value.symbol;
	else
		r2_value = r2->type;

	if (r1_value != r2_value) return false;

	return equal(r1->term_l, r2->term_l) && equal(r1->term_r, r2->term_r) ||
		   equal(r1->term_r, r2->term_l) && equal(r1->term_l, r2->term_r);
}

bool Regex::equivalent(Regex r1, Regex r2) {
	return FiniteAutomaton::equivalent(r1.to_ilieyu(), r2.to_ilieyu());
}
// TODO нужно сделать методы Regex константными
bool Regex::subset(const Regex& r) const {
	/*FiniteAutomaton dfa1 = to_ilieyu().determinize();
	FiniteAutomaton dfa2 = r.to_ilieyu().determinize();
	Language l;
	FiniteAutomaton dfa_instersection(intersection(dfa1, dfa2, &l));
	return equivalent(dfa_instersection, dfa2);*/
	return false;
}
