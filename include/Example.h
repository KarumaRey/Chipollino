#include <iostream>
#include "Regex.h"
#include "FiniteAutomat.h"

/* 
��� ����������� �����, ��� �� ������ ������ ������� 
������������� ������� �, ��������������, �� �������. 
����� ��������� ������ main
*/
class Example {
public:
	// ������ ���������� regex �� ������
	static void regex_parsing();
	static void fa_equal_check();
	static void fa_bisimilar_check();
	static void fa_merge_bisimilar();
};