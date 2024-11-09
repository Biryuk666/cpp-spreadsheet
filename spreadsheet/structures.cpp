#include "common.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdint.h>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return (row == rhs.row && col == rhs.col);
}

bool Position::operator<(const Position rhs) const {
    return (row < rhs.row || col < rhs.col);
}

bool Position::IsValid() const {
    return (row >= 0 && row < MAX_ROWS) && (col >=0 && col < MAX_COLS);
}

std::string Position::ToString() const {
    if (!IsValid()) {
        return {};
    }
    std::string result;
    result.reserve(MAX_POSITION_LENGTH);

    int col_copy = col;
    while (col_copy >= 0) {
        result.insert(result.begin(), 'A' + col_copy % LETTERS);
        col_copy = col_copy / LETTERS - 1;
    }

    result += std::to_string(row + 1);

    return result;
}

Position Position::FromString(std::string_view str) {
    uint8_t max_str_size = std::to_string(MAX_ROWS).size() + MAX_POS_LETTER_COUNT;

    if (str.size() > max_str_size || str.empty()) {
        return Position::NONE;
    }

    std::string str_col, str_row;    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            if (i == MAX_POS_LETTER_COUNT || !str_row.empty()) {
                return Position::NONE;
            }
            str_col.push_back(str[i]);
        } else if (str[i] >= '0' && str[i] <= '9') {
            str_row.push_back(str[i]);
        } else {
            return Position::NONE;
        }
    }

    if (str_col.empty() || str_row.empty()) {
        return Position::NONE;
    }

    int int_col = 0;
    for (const char ch : str_col) {
        int_col *= LETTERS;
        int_col += ch - 'A' + 1;
    }
    
    int int_row = std::stoi(str_row);

    return {int_row - 1, int_col -1};
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}