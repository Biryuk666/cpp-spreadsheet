#pragma once

#include "common.h"
#include "formula.h"

#include <memory>
#include <optional>
#include <unordered_set>
#include <variant>
#include <vector>

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;
    bool HasCache() const;
    
    void InvalidateCache();

private:
    class Impl {
    public:
        Impl() = default;
        virtual ~Impl() = default;

        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;

        virtual bool HasCache() const;   
        virtual void InvalidateCache() {}
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string&& text);
        Value GetValue() const override;
        std::string GetText() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(const std::string& formula, SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        virtual std::vector<Position> GetReferencedCells() const override;
        
        bool HasCache() const override;        
        void InvalidateCache() override;

    private:
        std::unique_ptr<FormulaInterface> formula_ptr_;
        const SheetInterface& sheet_ref_;  
        mutable std::optional<FormulaInterface::Value> cache_;
    };

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    std::unordered_set<Cell*> referenced_cells_, dependent_cells_;
    
    bool HasCircularDependencies(const std::vector<Position>& referenced_cells_pos);
    void UpdateDependencies(std::vector<Position>& referenced_cells_pos);
};