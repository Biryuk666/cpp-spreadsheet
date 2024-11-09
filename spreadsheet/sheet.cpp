#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <utility>

using namespace std::literals;

std::size_t Hasher::operator()(const Position& pos) const {
        return pos.row * INDEX + pos.col * std::pow(INDEX, 2);
    }

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("The set position is out of range"s);
    } else {
        auto it = sheet_.find(pos);
        if (it == sheet_.end()) {
            sheet_[pos] = std::make_unique<Cell>(*this);
            ++rows_to_cols_num_[pos.row];
            ++cols_to_rows_num_[pos.col];
        }
        sheet_.at(pos)->Set(std::move(text));
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("The get position is out of range"s);
    }
    auto it = sheet_.find(pos);
    if (it == sheet_.end()) {
        return nullptr;
    }
    return it->second.get();
}

CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>((static_cast<const Sheet&>(*this)).GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("The clear position is out of range"s);
    }
    auto it = sheet_.find(pos);
    if (it != sheet_.end()) {
        it->second->Clear();
        if (!it->second->IsReferenced()) {
            sheet_.erase(pos);
            if (--rows_to_cols_num_.at(pos.row) == 0) {
                rows_to_cols_num_.erase(pos.row);
            }
            if (--cols_to_rows_num_.at(pos.row) == 0) {
                cols_to_rows_num_.erase(pos.col);
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    if (sheet_.empty()) return {0,0};
    return {std::prev(rows_to_cols_num_.end())->first + 1, std::prev(cols_to_rows_num_.end())->first + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    PrintContext(output, "Value"s);
}

void Sheet::PrintTexts(std::ostream& output) const {
    PrintContext(output, "Text"s);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

std::ostream& operator<<(std::ostream& output, CellInterface::Value value) {
    std::visit([&output](const auto& obj) {output << obj;}, value);
    return output;
}

void Sheet::PrintContext(std::ostream& output, std::string context) const {
    const Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; row++) {
        for (int col = 0; col < size.cols; col++) {
            if (col > 0) output << '\t';
            auto it = sheet_.find({row, col});
            if(it != sheet_.end()) {
                if (context == "Value"s) {
                    output << it->second->GetValue();
                } else if (context == "Text"s) {
                    output << it->second->GetText();
                }
            }
        }
        output << '\n';
    }
}