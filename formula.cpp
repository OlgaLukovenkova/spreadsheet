#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression) 
            : ast_(ParseFormulaAST(expression)) {
        }

        Value Evaluate(const SheetInterface& sheet) const override {
            double res;
            try {
                res = ast_.Execute(sheet);
            }
            catch (const FormulaError& err) {
                return err;
            }
            return res;
        }

        std::string GetExpression() const override {
            std::ostringstream oss;
            ast_.PrintFormula(oss);
            return oss.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::forward_list<Position> list = ast_.GetCells();
            std::vector<Position> unique_pos(list.begin(), list.end());
            std::sort(unique_pos.begin(), unique_pos.end());
            auto last = std::unique(unique_pos.begin(), unique_pos.end());
            unique_pos.erase(last, unique_pos.end());
            return unique_pos;
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("Invalid formula");
    }
}