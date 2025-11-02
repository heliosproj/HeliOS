#ifndef NOMIC_QUERY_SQL_ENGINE_H
#define NOMIC_QUERY_SQL_ENGINE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include "nomic/core/interfaces.h"

namespace nomic {
namespace query {

// Forward declarations
class ISQLResult;
class ISQLPreparedStatement;
class ISQLEngine;

using SQLResultPtr = std::shared_ptr<ISQLResult>;
using SQLPreparedStatementPtr = std::shared_ptr<ISQLPreparedStatement>;
using SQLEnginePtr = std::shared_ptr<ISQLEngine>;

/**
 * @brief SQL Query Result interface
 * Represents the result of an SQL query execution
 */
class ISQLResult {
public:
    virtual ~ISQLResult() = default;

    // Query execution status
    virtual bool isSuccess() const = 0;
    virtual std::string getErrorMessage() const = 0;

    // Result data access
    virtual size_t getRowCount() const = 0;
    virtual size_t getColumnCount() const = 0;
    virtual std::vector<std::string> getColumnNames() const = 0;
    virtual std::vector<std::unordered_map<std::string, std::string>> getRows() const = 0;

    // Row access
    virtual std::unordered_map<std::string, std::string> getRow(size_t index) const = 0;
    virtual std::string getValue(size_t row, size_t col) const = 0;
    virtual std::string getValue(size_t row, const std::string& column) const = 0;

    // Export formats
    virtual std::string toJSON() const = 0;
    virtual std::string toCSV() const = 0;
    virtual std::string toSARIF() const = 0;
    virtual std::string toXML() const = 0;
};

/**
 * @brief Prepared SQL Statement interface
 * Represents a pre-compiled SQL statement with parameter placeholders
 */
class ISQLPreparedStatement {
public:
    virtual ~ISQLPreparedStatement() = default;

    virtual SQLResultPtr execute(const std::vector<std::string>& params) = 0;
    virtual void bindParam(size_t index, const std::string& value) = 0;
    virtual void bindParam(const std::string& name, const std::string& value) = 0;
    virtual void clearBindings() = 0;
    virtual std::string getSQL() const = 0;
};

/**
 * @brief Virtual table data provider function type
 * Returns rows of data for a virtual table query
 */
using VirtualTableProvider = std::function<std::vector<std::vector<std::string>>(const std::string& query)>;

/**
 * @brief Main SQL Query Engine interface
 * Provides SQL query capabilities over the semantic model
 */
class ISQLEngine {
public:
    virtual ~ISQLEngine() = default;

    // Query execution
    virtual SQLResultPtr execute(const std::string& sql) = 0;
    virtual std::vector<SQLResultPtr> executeBatch(const std::vector<std::string>& queries) = 0;
    virtual SQLPreparedStatementPtr prepare(const std::string& sql) = 0;

    // Transaction management
    virtual void beginTransaction() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
    virtual bool inTransaction() const = 0;

    // Virtual table management
    virtual bool registerVirtualTable(
        const std::string& name,
        const std::vector<std::string>& columns,
        VirtualTableProvider provider
    ) = 0;
    virtual bool unregisterVirtualTable(const std::string& name) = 0;
    virtual std::vector<std::string> getRegisteredTables() const = 0;

    // Schema information
    virtual std::vector<std::string> getTableColumns(const std::string& table) const = 0;
    virtual std::string getTableSchema(const std::string& table) const = 0;

    // Query analysis
    virtual SQLResultPtr explain(const std::string& sql) = 0;
    virtual std::string getQueryPlan(const std::string& sql) = 0;

    // Semantic model integration
    virtual void setSemanticModel(core::SemanticModelPtr model) = 0;
    virtual core::SemanticModelPtr getSemanticModel() const = 0;
    virtual void setRootNode(core::ASTNodePtr root) = 0;
    virtual core::ASTNodePtr getRootNode() const = 0;

    // Performance and optimization
    virtual void enableQueryCache(bool enable) = 0;
    virtual void clearQueryCache() = 0;
    virtual size_t getQueryCacheSize() const = 0;
    virtual void setQueryTimeout(std::chrono::milliseconds timeout) = 0;
};

/**
 * @brief Virtual table definitions as per REQ-007
 * Standard virtual tables exposed by the SQL engine
 */
struct VirtualTables {
    // AST Virtual Table
    static constexpr const char* AST = "ast";
    static constexpr const char* AST_ID = "id";
    static constexpr const char* AST_TYPE = "type";
    static constexpr const char* AST_PARENT_ID = "parent_id";
    static constexpr const char* AST_FILE = "file";
    static constexpr const char* AST_LINE = "line";
    static constexpr const char* AST_COLUMN = "column";
    static constexpr const char* AST_TEXT = "text";

    // Symbols Virtual Table
    static constexpr const char* SYMBOLS = "symbols";
    static constexpr const char* SYMBOLS_ID = "id";
    static constexpr const char* SYMBOLS_NAME = "name";
    static constexpr const char* SYMBOLS_KIND = "kind";
    static constexpr const char* SYMBOLS_TYPE = "type";
    static constexpr const char* SYMBOLS_SCOPE_ID = "scope_id";
    static constexpr const char* SYMBOLS_AST_ID = "ast_id";
    static constexpr const char* SYMBOLS_IS_DEFINITION = "is_definition";
    static constexpr const char* SYMBOLS_IS_DECLARATION = "is_declaration";

    // Types Virtual Table
    static constexpr const char* TYPES = "types";
    static constexpr const char* TYPES_ID = "id";
    static constexpr const char* TYPES_NAME = "name";
    static constexpr const char* TYPES_KIND = "kind";
    static constexpr const char* TYPES_SIZE = "size";
    static constexpr const char* TYPES_ALIGNMENT = "alignment";
    static constexpr const char* TYPES_IS_CONST = "is_const";
    static constexpr const char* TYPES_IS_VOLATILE = "is_volatile";
    static constexpr const char* TYPES_IS_POINTER = "is_pointer";
    static constexpr const char* TYPES_IS_REFERENCE = "is_reference";

    // Scopes Virtual Table
    static constexpr const char* SCOPES = "scopes";
    static constexpr const char* SCOPES_ID = "id";
    static constexpr const char* SCOPES_NAME = "name";
    static constexpr const char* SCOPES_KIND = "kind";
    static constexpr const char* SCOPES_PARENT_ID = "parent_id";
    static constexpr const char* SCOPES_AST_ID = "ast_id";

    // Files Virtual Table
    static constexpr const char* FILES = "files";
    static constexpr const char* FILES_ID = "id";
    static constexpr const char* FILES_PATH = "path";
    static constexpr const char* FILES_NAME = "name";
    static constexpr const char* FILES_SIZE = "size";
    static constexpr const char* FILES_LINES = "lines";
    static constexpr const char* FILES_LANGUAGE = "language";

    // Metrics Virtual Table
    static constexpr const char* METRICS = "metrics";
    static constexpr const char* METRICS_AST_ID = "ast_id";
    static constexpr const char* METRICS_COMPLEXITY = "complexity";
    static constexpr const char* METRICS_LOC = "loc";
    static constexpr const char* METRICS_SLOC = "sloc";
    static constexpr const char* METRICS_HALSTEAD = "halstead";
    static constexpr const char* METRICS_MAINTAINABILITY = "maintainability";
    static constexpr const char* METRICS_FAN_IN = "fan_in";
    static constexpr const char* METRICS_FAN_OUT = "fan_out";

    // Control Flow Graph Virtual Table
    static constexpr const char* CFG = "cfg";
    static constexpr const char* CFG_FUNCTION_ID = "function_id";
    static constexpr const char* CFG_FROM_ID = "from_id";
    static constexpr const char* CFG_TO_ID = "to_id";
    static constexpr const char* CFG_EDGE_TYPE = "edge_type";
    static constexpr const char* CFG_CONDITION = "condition";

    // Data Flow Virtual Table
    static constexpr const char* DATAFLOW = "dataflow";
    static constexpr const char* DATAFLOW_VAR_NAME = "var_name";
    static constexpr const char* DATAFLOW_DEF_SITE = "def_site";
    static constexpr const char* DATAFLOW_USE_SITES = "use_sites";
    static constexpr const char* DATAFLOW_IS_TAINTED = "is_tainted";
    static constexpr const char* DATAFLOW_TAINT_SOURCE = "taint_source";

    // Analysis Results Virtual Table
    static constexpr const char* ANALYSIS_RESULTS = "analysis_results";
    static constexpr const char* ANALYSIS_FILE = "file";
    static constexpr const char* ANALYSIS_LINE = "line";
    static constexpr const char* ANALYSIS_COLUMN = "column";
    static constexpr const char* ANALYSIS_MESSAGE = "message";
    static constexpr const char* ANALYSIS_SEVERITY = "severity";
    static constexpr const char* ANALYSIS_RULE_ID = "rule_id";
};

/**
 * @brief Factory function to create an SQL engine instance
 */
SQLEnginePtr createSQLEngine();

} // namespace query
} // namespace nomic

#endif // NOMIC_QUERY_SQL_ENGINE_H