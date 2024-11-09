#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>


struct Hasher {
    std::size_t operator()(const Position& pos) const;

private:
    static const size_t INDEX = 17;
};

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher> sheet_;
    std::map<int, int> rows_to_cols_num_, cols_to_rows_num_;

    void PrintContext(std::ostream& output, std::string context) const;
};