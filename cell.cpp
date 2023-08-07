#include "cell.h"
#include <queue>

using namespace std::literals;

class Cell::Impl {
public:
	virtual CellInterface::Value GetValue(const SheetInterface& sheet) const = 0;
	virtual std::string GetText() const = 0;
	virtual std::vector<Position> GetReferencedCells() const = 0;
	virtual ~Impl() {}

protected:
	Impl() {};
};

class Cell::EmptyImpl : public Cell::Impl {
public:
	EmptyImpl();
	CellInterface::Value GetValue(const SheetInterface& sheet) const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;
};

class Cell::TextImpl : public Cell::Impl {
public:
	explicit TextImpl(const std::string& str);
	CellInterface::Value GetValue(const SheetInterface& sheet) const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

private:
	std::string value_;

};

class Cell::FormulaImpl : public Cell::Impl {
public:
	explicit FormulaImpl(const std::string& str);
	CellInterface::Value GetValue(const SheetInterface& sheet) const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

private:
	std::unique_ptr<FormulaInterface> value_;
};

/*--- Cell ---*/
Cell::Cell(Sheet& sheet)
	: sheet_(sheet)
	, cache_(std::nullopt)
	, impl_(std::make_unique<EmptyImpl>())
{

}

Cell::~Cell() {}

void Cell::Set(const std::string& text) {
	std::unique_ptr<Cell::Impl> impl;
	if (text.empty()) {
		impl = std::make_unique<EmptyImpl>();
	}
	else if (text.at(0) == FORMULA_SIGN && text.length() > 1) {
		impl = std::make_unique<FormulaImpl>(text.substr(1));
	}
	else {
		impl = std::make_unique<TextImpl>(text);
	}

	if (HaveCircularDependencies(impl)) {
		throw CircularDependencyException("Circular dependency"s);
	}

	impl_ = std::move(impl);
	CorrectReferences();
	ClearCache();
}

void Cell::Clear() {
	Set(""s);
}

CellInterface::Value Cell::GetValue() const {
	if (!cache_) {
		cache_ = impl_->GetValue(sheet_);
	}
	return *cache_;
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
	return cells_where_this_is_used_.size() > 0;
}

bool Cell::HaveCircularDependencies(const std::unique_ptr<Impl>& new_impl) const {
	std::unordered_set<Position, PositionHasher> visited_cells;
	std::queue<Position> cell_queue;
	for (Position pos : new_impl->GetReferencedCells()) {
		cell_queue.push(pos);
	}

	while (!cell_queue.empty()) {
		Position next_pos = cell_queue.front();
		if (this == sheet_.GetRawCell(next_pos)) {
			return true;
		}
		visited_cells.insert(next_pos);

		if (sheet_.GetRawCell(next_pos)) {
			for (Position pos : sheet_.GetRawCell(next_pos)->GetReferencedCells()) {
				if (visited_cells.count(pos) == 0) {
					cell_queue.push(pos);
				}
			}
		}
		cell_queue.pop();
	}

	return false;
}

void Cell::CorrectReferences() {
	for (Cell* cell : used_cells_) {
		cell->cells_where_this_is_used_.erase(this);
	}

	used_cells_.clear();

	for (Position pos : impl_->GetReferencedCells()) {
		if (!sheet_.GetRawCell(pos)) {
			sheet_.SetCell(pos, ""s);
		}
		used_cells_.insert(sheet_.GetRawCell(pos));
	}

	for (Cell* cell : used_cells_) {
		cell->cells_where_this_is_used_.insert(this);
	}
}

void Cell::ClearCache() { 
	std::unordered_set<Cell*> visited_cells;
	std::queue<Cell*> cell_queue;
	cell_queue.push(this);

	while (!cell_queue.empty()) {
		Cell* cell = cell_queue.front();
		visited_cells.insert(cell);
		cell->cache_ = std::nullopt;
		for (Cell* cell_for_clearing : cell->cells_where_this_is_used_) {
			if (visited_cells.count(cell_for_clearing) == 0) {
				cell_queue.push(cell_for_clearing);
			}
		}
		cell_queue.pop();
	}
}

/*--- EmptyImpl ---*/
Cell::EmptyImpl::EmptyImpl() {

}

CellInterface::Value Cell::EmptyImpl::GetValue(const SheetInterface& sheet) const {
	return ""s;
}

std::string Cell::EmptyImpl::GetText() const {
	return ""s;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
	return {};
}

/*--- TextImpl ---*/
Cell::TextImpl::TextImpl(const std::string& str)
	: value_(str)
{

}

CellInterface::Value Cell::TextImpl::GetValue(const SheetInterface& sheet) const {
	if (value_.at(0) == ESCAPE_SIGN) {
		return value_.substr(1);
	}
	return value_;
}

std::string Cell::TextImpl::GetText() const {
	return value_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
	return {};
}

/*--- FormulaImpl ---*/
Cell::FormulaImpl::FormulaImpl(const std::string& str)
	: value_(ParseFormula(str))
{

}

CellInterface::Value Cell::FormulaImpl::GetValue(const SheetInterface& sheet) const {
	auto res = value_->Evaluate(sheet);
	if (std::holds_alternative<double>(res)) {
		return std::get<double>(res);
	}

	return std::get<FormulaError>(res);
}

std::string Cell::FormulaImpl::GetText() const {
	return std::string(1, FORMULA_SIGN) + value_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return value_->GetReferencedCells();
}