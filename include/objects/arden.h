//
// Created by xtoter on 03.10.22.
//
#include "Regex.h"
#include <FiniteAutomaton.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <string>

#ifndef CHIPOLLINO_ARDEN_H

#define CHIPOLLINO_ARDEN_H

struct expression_arden {
	int condition;
	Regex* temp_regex;
};
Regex nfa_to_regex(FiniteAutomaton in);
#endif // CHIPOLLINO_ARDEN_H