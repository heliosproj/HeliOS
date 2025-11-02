#ifndef NOMIC_QUERY_DSL_ENGINE_H
#define NOMIC_QUERY_DSL_ENGINE_H

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <any>
#include <variant>
#include "nomic/core/interfaces.h"

namespace nomic {
namespace query {

// Forward declarations
class IDSLExpression;
class IDSLContext;
class IDSLFunction;
class IDSLEngine;

// Forward declaration for map type
using DSLMap = std::shared_ptr<std::unordered_map<std::string, std::any>>;

// Forward declare lambda type
class LambdaExpression;
using DSLLambda = std::shared_ptr<LambdaExpression>;

using DSLValue = std::variant<
    std::nullptr_t,
    bool,
    int,
    double,
    std::string,
    std::vector<std::any>,
    DSLMap,  // Using shared_ptr to avoid comparison issues
    DSLLambda,  // Lambda expressions
    core::ASTNodePtr,
    core::TypeInfoPtr,
    core::SymbolPtr,
    core::ScopePtr
>;

using DSLExpressionPtr = std::shared_ptr<IDSLExpression>;
using DSLContextPtr = std::shared_ptr<IDSLContext>;
using DSLFunctionPtr = std::shared_ptr<IDSLFunction>;
using DSLEnginePtr = std::shared_ptr<IDSLEngine>;

/**
 * @brief DSL Expression interface
 * Represents an expression in the DSL that can be evaluated
 */
class IDSLExpression {
public:
    virtual ~IDSLExpression() = default;

    enum class ExpressionType {
        LITERAL,
        IDENTIFIER,
        BINARY_OP,
        UNARY_OP,
        FUNCTION_CALL,
        LAMBDA,
        IF_ELSE,
        WHILE_LOOP,
        FOR_LOOP,
        LOOP,
        SWITCH,
        MATCH,
        PATTERN,
        MEMBER_ACCESS,
        INDEX_ACCESS,
        ASSIGNMENT
    };

    virtual ExpressionType getType() const = 0;
    virtual DSLValue evaluate(DSLContextPtr context) = 0;
    virtual std::string toString() const = 0;
};

/**
 * @brief DSL execution context
 * Manages variables, scopes, and function bindings
 */
class IDSLContext {
public:
    virtual ~IDSLContext() = default;

    // Variable management
    virtual void setVariable(const std::string& name, const DSLValue& value) = 0;
    virtual DSLValue getVariable(const std::string& name) const = 0;
    virtual bool hasVariable(const std::string& name) const = 0;
    virtual void deleteVariable(const std::string& name) = 0;

    // Scope management
    virtual void pushScope() = 0;
    virtual void popScope() = 0;
    virtual size_t getScopeDepth() const = 0;

    // Function management
    virtual void registerFunction(const std::string& name, DSLFunctionPtr function) = 0;
    virtual DSLFunctionPtr getFunction(const std::string& name) const = 0;
    virtual bool hasFunction(const std::string& name) const = 0;

    // Semantic model access
    virtual core::SemanticModelPtr getSemanticModel() const = 0;
    virtual void setSemanticModel(core::SemanticModelPtr model) = 0;
};

/**
 * @brief DSL Function interface
 * Represents a callable function in the DSL
 */
class IDSLFunction {
public:
    virtual ~IDSLFunction() = default;

    enum class FunctionKind {
        BUILTIN,        // Built-in function
        USER_DEFINED,   // User-defined function
        LAMBDA,         // Anonymous function
        NATIVE          // Native C++ function
    };

    virtual FunctionKind getKind() const = 0;
    virtual std::string getName() const = 0;
    virtual size_t getArity() const = 0;  // Number of parameters (-1 for variadic)
    virtual bool isVariadic() const = 0;
    virtual DSLValue call(const std::vector<DSLValue>& args, DSLContextPtr context) = 0;
    virtual std::string getSignature() const = 0;
    virtual std::string getDescription() const = 0;
};

/**
 * @brief Main DSL Engine interface
 * Provides parsing, evaluation, and built-in functions for the DSL
 */
class IDSLEngine {
public:
    virtual ~IDSLEngine() = default;

    // Parsing
    virtual DSLExpressionPtr parse(const std::string& expression) = 0;
    virtual std::vector<DSLExpressionPtr> parseMultiple(const std::string& expressions) = 0;
    virtual bool validate(const std::string& expression) = 0;
    virtual std::string getParseError() const = 0;

    // Evaluation
    virtual DSLValue evaluate(const std::string& expression, DSLContextPtr context = nullptr) = 0;
    virtual DSLValue evaluate(DSLExpressionPtr expr, DSLContextPtr context = nullptr) = 0;

    // Context management
    virtual DSLContextPtr createContext() = 0;
    virtual DSLContextPtr getCurrentContext() = 0;

    // Built-in functions registry
    virtual std::vector<std::string> getBuiltinFunctionNames() const = 0;
    virtual DSLFunctionPtr getBuiltinFunction(const std::string& name) const = 0;
    virtual std::string getBuiltinFunctionHelp(const std::string& name) const = 0;

    // Pattern matching
    virtual bool match(const std::string& pattern, core::ASTNodePtr node) = 0;
    virtual std::vector<core::ASTNodePtr> findMatches(const std::string& pattern) = 0;
};

/**
 * @brief Built-in function categories as per REQ-005
 */
struct BuiltinFunctions {
    // AST Navigation functions
    static constexpr const char* PARENT = "parent";
    static constexpr const char* CHILDREN = "children";
    static constexpr const char* ANCESTORS = "ancestors";
    static constexpr const char* DESCENDANTS = "descendants";
    static constexpr const char* SIBLINGS = "siblings";
    static constexpr const char* NEXT_SIBLING = "nextSibling";
    static constexpr const char* PREV_SIBLING = "prevSibling";
    static constexpr const char* FIRST_CHILD = "firstChild";
    static constexpr const char* LAST_CHILD = "lastChild";
    static constexpr const char* NTH_CHILD = "nthChild";

    // Type Analysis functions
    static constexpr const char* TYPEOF = "typeof";
    static constexpr const char* SIZEOF = "sizeof";
    static constexpr const char* IS_POINTER = "isPointer";
    static constexpr const char* IS_ARRAY = "isArray";
    static constexpr const char* IS_CONST = "isConst";
    static constexpr const char* IS_VOLATILE = "isVolatile";
    static constexpr const char* IS_FUNCTION = "isFunction";
    static constexpr const char* IS_STRUCT = "isStruct";
    static constexpr const char* IS_UNION = "isUnion";
    static constexpr const char* IS_ENUM = "isEnum";
    static constexpr const char* IS_TYPEDEF = "isTypedef";
    static constexpr const char* IS_COMPLETE = "isComplete";
    static constexpr const char* GET_POINTEE = "getPointee";
    static constexpr const char* GET_RETURN_TYPE = "getReturnType";
    static constexpr const char* GET_PARAM_TYPES = "getParamTypes";

    // Control Flow functions
    static constexpr const char* COMPLEXITY = "complexity";
    static constexpr const char* PATHS = "paths";
    static constexpr const char* DOMINATES = "dominates";
    static constexpr const char* REACHES = "reaches";
    static constexpr const char* IS_REACHABLE = "isReachable";
    static constexpr const char* PREDECESSORS = "predecessors";
    static constexpr const char* SUCCESSORS = "successors";
    static constexpr const char* ENTRY_POINTS = "entryPoints";
    static constexpr const char* EXIT_POINTS = "exitPoints";
    static constexpr const char* LOOPS = "loops";
    static constexpr const char* IS_IN_LOOP = "isInLoop";
    static constexpr const char* LOOP_DEPTH = "loopDepth";

    // Data Flow functions
    static constexpr const char* DEFINES = "defines";
    static constexpr const char* USES = "uses";
    static constexpr const char* TAINTS = "taints";
    static constexpr const char* FLOWS = "flows";
    static constexpr const char* DEPENDS_ON = "dependsOn";
    static constexpr const char* INFLUENCES = "influences";
    static constexpr const char* IS_DEFINED = "isDefined";
    static constexpr const char* IS_USED = "isUsed";
    static constexpr const char* DEF_USE_CHAIN = "defUseChain";
    static constexpr const char* USE_DEF_CHAIN = "useDefChain";
    static constexpr const char* REACHING_DEFS = "reachingDefs";
    static constexpr const char* LIVE_VARS = "liveVars";

    // Metrics functions
    static constexpr const char* LOC = "loc";
    static constexpr const char* SLOC = "sloc";
    static constexpr const char* CYCLOMATIC = "cyclomatic";
    static constexpr const char* COUPLING = "coupling";
    static constexpr const char* COHESION = "cohesion";
    static constexpr const char* HALSTEAD = "halstead";
    static constexpr const char* MAINTAINABILITY = "maintainability";
    static constexpr const char* FAN_IN = "fanIn";
    static constexpr const char* FAN_OUT = "fanOut";
    static constexpr const char* DEPTH = "depth";
    static constexpr const char* NESTING = "nesting";
    static constexpr const char* PARAMS = "params";

    // Pattern Matching functions
    static constexpr const char* MATCHES = "matches";
    static constexpr const char* CONTAINS = "contains";
    static constexpr const char* STARTS_WITH = "startsWith";
    static constexpr const char* ENDS_WITH = "endsWith";
    static constexpr const char* REGEX = "regex";
    static constexpr const char* GLOB = "glob";
    static constexpr const char* XPATH = "xpath";
    static constexpr const char* PATTERN = "pattern";
    static constexpr const char* EXTRACT = "extract";
    static constexpr const char* REPLACE_PATTERN = "replacePattern";

    // Collection Operations
    static constexpr const char* MAP = "map";
    static constexpr const char* FILTER = "filter";
    static constexpr const char* REDUCE = "reduce";
    static constexpr const char* ANY = "any";
    static constexpr const char* ALL = "all";
    static constexpr const char* NONE = "none";
    static constexpr const char* COUNT = "count";
    static constexpr const char* SUM = "sum";
    static constexpr const char* AVG = "avg";
    static constexpr const char* MIN = "min";
    static constexpr const char* MAX = "max";
    static constexpr const char* SORT = "sort";
    static constexpr const char* REVERSE = "reverse";
    static constexpr const char* UNIQUE = "unique";
    static constexpr const char* GROUP_BY = "groupBy";
    static constexpr const char* PARTITION = "partition";
    static constexpr const char* ZIP = "zip";
    static constexpr const char* FLATTEN = "flatten";
    static constexpr const char* TAKE = "take";
    static constexpr const char* DROP = "drop";
    static constexpr const char* SLICE = "slice";

    // String Operations
    static constexpr const char* FORMAT = "format";
    static constexpr const char* SPLIT = "split";
    static constexpr const char* JOIN = "join";
    static constexpr const char* REPLACE = "replace";
    static constexpr const char* TRIM = "trim";
    static constexpr const char* UPPER = "upper";
    static constexpr const char* LOWER = "lower";
    static constexpr const char* SUBSTRING = "substring";
    static constexpr const char* INDEX_OF = "indexOf";
    static constexpr const char* LAST_INDEX_OF = "lastIndexOf";
    static constexpr const char* LENGTH = "length";
    static constexpr const char* IS_EMPTY = "isEmpty";
    static constexpr const char* PAD_LEFT = "padLeft";
    static constexpr const char* PAD_RIGHT = "padRight";

    // Additional utility functions
    static constexpr const char* GET_NAME = "getName";
    static constexpr const char* GET_LOCATION = "getLocation";
    static constexpr const char* GET_FILE = "getFile";
    static constexpr const char* GET_LINE = "getLine";
    static constexpr const char* GET_COLUMN = "getColumn";
    static constexpr const char* GET_SOURCE = "getSource";
    static constexpr const char* GET_SYMBOL = "getSymbol";
    static constexpr const char* GET_SCOPE = "getScope";
    static constexpr const char* FIND_SYMBOL = "findSymbol";
    static constexpr const char* FIND_SYMBOLS_BY_KIND = "findSymbolsByKind";
    static constexpr const char* FIND_TYPE = "findType";
    static constexpr const char* FIND_CALLS = "findCalls";
    static constexpr const char* FIND_REFS = "findRefs";
};

/**
 * @brief Factory function to create a DSL engine instance
 */
DSLEnginePtr createDSLEngine();

} // namespace query
} // namespace nomic

#endif // NOMIC_QUERY_DSL_ENGINE_H