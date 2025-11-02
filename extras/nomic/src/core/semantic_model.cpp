#include "nomic/core/semantic_model.h"
#include "nomic/core/ast_node.h"
#include "nomic/core/symbol.h"
#include "nomic/core/scope.h"
#include "nomic/core/type_info.h"

namespace nomic {
namespace core {

SemanticModel::SemanticModel() {
    initializeGlobalScope();
}

} // namespace core
} // namespace nomic