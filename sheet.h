#pragma once

#include "common.h"
#include "cell.h"
#include <unordered_map>

struct PositionHasher {
    size_t operator() (Position obj) const {
        return obj.row + obj.col * 31;
    }
};

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    Cell* GetRawCell(Position pos);
    const Cell* GetRawCell(Position pos) const;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    using InnerCellStorage = std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher>;
    InnerCellStorage sheet_;

    bool IsExistedCell(Position pos) const;
};