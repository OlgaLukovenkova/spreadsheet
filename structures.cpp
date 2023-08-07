#include "common.h"

#include <cctype>
#include <sstream>
#include <regex>
#include <optional>

#include <iostream>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

namespace detail {
	std::string NumberToString(int number) {
		std::string str;
		str.reserve(MAX_POSITION_LENGTH);
		while (number >= 0) {
			int mod = number % LETTERS;
			str.insert(str.begin(), 'A' + mod);
			number = number / LETTERS - 1;
		}
		return str;
	}

	int LettersToNumber(std::string_view str) {
		int base = 1;
		int col = 0;
		for (auto it = str.rbegin(); it != str.rend(); ++it) {
			col += (*it - 'A' + 1) * base;
			base *= LETTERS;
		}
		return col;
	}
}

const Position Position::NONE = { -1, -1 };

bool Position::operator==(const Position rhs) const {
	return rhs.row == row && rhs.col == col;
}

bool Position::operator<(const Position rhs) const {
	return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
	return row >= 0 && row < MAX_ROWS && col >= 0 && col < MAX_COLS;
}

std::string Position::ToString() const {
	using namespace std::literals;
	if (!IsValid()) {
		return ""s;
	}
	return detail::NumberToString(col) + std::to_string(row + 1);
}

Position Position::FromString(std::string_view str) {
	using namespace std::literals;
	std::regex regex_of_address(R"(([A-Z]{1,3})(\d{1,5}))");
	std::smatch match_obj;
	std::string test_str{ str };

	if (!std::regex_match(test_str, match_obj, regex_of_address)) {
		return Position::NONE;
	}

	Position pos{ std::stoi(match_obj[2].str()) - 1, detail::LettersToNumber(match_obj[1].str()) - 1 };
	
	if (!pos.IsValid()) {
		return Position::NONE;
	}
	
	return pos;
}

bool Size::operator==(Size rhs) const {
	return cols == rhs.cols && rows == rhs.rows;
}

