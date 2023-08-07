#include "sheet.h"
#include <iostream>

using namespace std::literals;

Sheet::~Sheet() {}

bool Sheet::IsExistedCell(Position pos) const {
    return sheet_.count(pos) > 0;
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Wrong position"s);
    }

    // если €чейка существовала и текст не мен€етс€
    if (IsExistedCell(pos) && text == sheet_.at(pos)->GetText()) {
        return;
    }

    // если €чейка не существует, создадим пустую, чтобы затем заполнить
    if (!IsExistedCell(pos)) {
        sheet_[pos] = std::make_unique<Cell>(*this);
    }

    try {
        //заполн€ем
        sheet_[pos]->Set(text);
    }
    catch (const CircularDependencyException& e) {
        throw e;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetRawCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetRawCell(pos);
}

Cell* Sheet::GetRawCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Wrong position"s);
    }
    if (!IsExistedCell(pos)) {
        return nullptr;
    }

    return sheet_.at(pos).get();
}

const Cell* Sheet::GetRawCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetRawCell(pos);
}

void Sheet::ClearCell(Position pos) {
    SetCell(pos, ""s);
    if (!sheet_[pos]->IsReferenced()) {
        sheet_.erase(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    Size size{ 0, 0 };
    for (const auto& [pos, cell_ptr] : sheet_) {
        if (!cell_ptr->GetText().empty()) {
            if (pos.row + 1 > size.rows) {
                size.rows = pos.row + 1;
            }
            if (pos.col + 1 > size.cols) {
                size.cols = pos.col + 1;
            }
        }
    }
    return size;
}

namespace {
    std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
        std::visit(
            [&](const auto& x) {
                output << x;
            },
            value);
        return output;
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        bool is_first = true;
        for (int col = 0; col < size.cols; ++col) {
            CellInterface::Value value = IsExistedCell({ row, col }) ? GetCell({row, col})->GetValue() : ""s;
            output << (is_first ? ""s : "\t") << value;
            is_first = false;
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        bool is_first = true;
        for (int col = 0; col < size.cols; ++col) {
            std::string value = IsExistedCell({ row, col }) ? GetCell({ row, col })->GetText() : ""s;
            output << (is_first ? ""s : "\t") << value;
            is_first = false;
        }
        output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}