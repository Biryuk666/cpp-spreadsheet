#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <utility>

using namespace std::literals;

// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}
Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> temp_impl;
    if (text.empty()) {
        temp_impl = std::make_unique<EmptyImpl>();
    } else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        temp_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
        const Impl& formula = *temp_impl;
        auto referensed_cells_pos = formula.GetReferencedCells();
        if (!referensed_cells_pos.empty()) {
            if (HasCircularDependencies(referensed_cells_pos)) {
                throw CircularDependencyException("Circular dependency detected");
            }
        }
        InvalidateCache();
        UpdateDependencies(referensed_cells_pos);
    } else {
        temp_impl = std::make_unique<TextImpl>(std::move(text));
    }
    impl_ = std::move(temp_impl);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

bool Cell::IsReferenced() const {
    return !dependent_cells_.empty();
}

bool Cell::HasCache() const {
    return impl_->HasCache();
}

bool Cell::HasCircularDependencies(const std::vector<Position>& referenced_cells_pos) {
    std::unordered_set<const Cell*> referenced_cells;
    for (const auto& pos : referenced_cells_pos) {
        referenced_cells.insert(dynamic_cast<Cell*>(sheet_.GetCell(pos)));
    }
    std::unordered_set<const Cell*> processed_cells;
    std::vector<const Cell*> stack;
    stack.push_back(this);
    
    while (!stack.empty()) {            
        const Cell* cell_ptr = stack.back();            
        stack.pop_back();            
        processed_cells.insert(cell_ptr);

        if (referenced_cells.find(cell_ptr) == referenced_cells.end()) {                    
            for (const Cell* referensed : cell_ptr->referenced_cells_) {
                if (processed_cells.find(referensed) == processed_cells.end()) {
                    stack.push_back(referensed);
                }
            }            
        } else {
            return true;
        }
    }

    return false;
}

void Cell::InvalidateCache() {
    if (HasCache()) {
        impl_->InvalidateCache();
        for (auto& cell : dependent_cells_) {
            cell->InvalidateCache();
        }
    }
}

void Cell::UpdateDependencies(std::vector<Position>& referenced_cells_pos) {
    for (auto& cell : referenced_cells_) {
        cell->dependent_cells_.erase(this);
    }

    referenced_cells_.clear();

    for (const auto& pos : referenced_cells_pos) {
        auto cell_ptr = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        if (!cell_ptr) {
            sheet_.SetCell(pos, "");
            cell_ptr = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        }
        cell_ptr->dependent_cells_.insert(this);
        referenced_cells_.insert(cell_ptr);
    }
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

bool Cell::Impl::HasCache() const {
    return true;
}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return ""s;
}

std::string Cell::EmptyImpl::GetText() const {
    return ""s;
}

Cell::TextImpl::TextImpl(std::string&& text) : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_[0] == ESCAPE_SIGN) {
        return text_.substr(1);
    }
    return text_;
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(const std::string& formula, SheetInterface& sheet) : formula_ptr_(std::move(ParseFormula(formula.substr(1)))), sheet_ref_(sheet) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_) {
        cache_ = formula_ptr_->Evaluate(sheet_ref_);
    }
    if (std::holds_alternative<double>(cache_.value())) {
        return std::get<double>(cache_.value());
    } else {
        return std::get<FormulaError>(cache_.value());
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_ptr_->GetExpression();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();

}

bool Cell::FormulaImpl::HasCache() const {
    return cache_.has_value();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_ptr_->GetReferencedCells();
}

void Cell::FormulaImpl::InvalidateCache() {
    cache_.reset();
}