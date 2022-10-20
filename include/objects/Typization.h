#pragma once
#include <variant>
#include <set>
#include <map>
#include <deque>
#include "Regex.h"
#include "FiniteAutomat.h"

// ��������� ������� ������
namespace Typization {
	// ������������ ����� ��������
	enum class ObjectType {
		NFA,      // ������������������� ��
		DFA,      // ����������������� ��
		Regex,    // ���������� ���������
		Int,      // ����� �����
		FileName, // ��� ����� ��� ������
		Boolean   // true/false
	};

	// ��������� �������� ��� �������� � ��������������
	template <ObjectType T, class V>
	struct ObjectHolder {
		V value;
		ObjectType type() const { return  T };
		// �����������
		optional<bool> minimal; // � ��� �����
	};

	// ������������� ������
	using GeneralObject = variant<
		ObjectHolder<ObjectType::NFA, FiniteAutomat>,
		ObjectHolder<ObjectType::DFA, FiniteAutomat>,
		ObjectHolder<ObjectType::Regex, Regex>,
		ObjectHolder<ObjectType::Int, int>,
		ObjectHolder<ObjectType::FileName, string>,
		ObjectHolder<ObjectType::Boolean, bool>
	>;

	// �������, ������� �� ����� � ���������
	// �������� - ���� �������, �� �� ������ boolean
	struct Function {
		// ��� �������
		string name;
		// ���� �������� ����������
		vector<ObjectType> input;
		// ��� ��������� ���������
		ObjectType output;
	};
};