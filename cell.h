#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <unordered_set>
#include <optional>

class Sheet;

using Cache = std::optional<CellInterface::Value>;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(const std::string& text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    Sheet& sheet_;
    mutable Cache cache_;
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Cell*> used_cells_;
    std::unordered_set<Cell*> cells_where_this_is_used_;

    bool HaveCircularDependencies(const std::unique_ptr<Impl>& new_impl) const;
    void CorrectReferences();
    void ClearCache();
};
