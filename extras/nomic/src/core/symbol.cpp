#include "nomic/core/symbol.h"

namespace nomic {
namespace core {

Symbol::Symbol(SymbolKind kind)
    : kind_(kind),
      isGlobal_(false),
      isStatic_(false),
      isExtern_(false),
      isInline_(false) {
}

std::string Symbol::getKindName() const {
    switch (kind_) {
        case SymbolKind::VARIABLE: return "Variable";
        case SymbolKind::FUNCTION: return "Function";
        case SymbolKind::PARAMETER: return "Parameter";
        case SymbolKind::FIELD: return "Field";
        case SymbolKind::TYPEDEF: return "Typedef";
        case SymbolKind::STRUCT: return "Struct";
        case SymbolKind::UNION: return "Union";
        case SymbolKind::ENUM: return "Enum";
        case SymbolKind::ENUMERATOR: return "Enumerator";
        case SymbolKind::LABEL: return "Label";
        case SymbolKind::UNKNOWN:
        default: return "Unknown";
    }
}

} // namespace core
} // namespace nomic