/**
 * @file sql_engine.cpp
 * @brief SQL Query Engine implementation with virtual tables using SQLite
 */

#include "nomic/query/sql_engine.h"
#include "nomic/core/interfaces.h"
#include <sqlite3.h>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <jsoncpp/json/json.h>
#include <iomanip>

namespace nomic {
namespace query {

// Forward declarations
class SQLResult;
class SQLPreparedStatement;
class SQLEngine;

/**
 * @brief SQL Result implementation
 */
class SQLResult : public ISQLResult {
private:
    bool success_;
    std::string error_message_;
    std::vector<std::string> column_names_;
    std::vector<std::vector<std::string>> rows_;

public:
    SQLResult() : success_(false) {}

    void setSuccess(bool success) { success_ = success; }
    void setError(const std::string& error) {
        success_ = false;
        error_message_ = error;
    }

    void setColumns(const std::vector<std::string>& columns) {
        column_names_ = columns;
    }

    void addRow(const std::vector<std::string>& row) {
        rows_.push_back(row);
    }

    // ISQLResult implementation
    bool isSuccess() const override { return success_; }
    std::string getErrorMessage() const override { return error_message_; }
    size_t getRowCount() const override { return rows_.size(); }
    size_t getColumnCount() const override { return column_names_.size(); }
    std::vector<std::string> getColumnNames() const override { return column_names_; }

    std::vector<std::unordered_map<std::string, std::string>> getRows() const override {
        std::vector<std::unordered_map<std::string, std::string>> result;
        for (const auto& row : rows_) {
            std::unordered_map<std::string, std::string> row_map;
            for (size_t i = 0; i < column_names_.size() && i < row.size(); ++i) {
                row_map[column_names_[i]] = row[i];
            }
            result.push_back(row_map);
        }
        return result;
    }

    std::unordered_map<std::string, std::string> getRow(size_t index) const override {
        if (index >= rows_.size()) {
            return {};
        }
        std::unordered_map<std::string, std::string> row_map;
        for (size_t i = 0; i < column_names_.size() && i < rows_[index].size(); ++i) {
            row_map[column_names_[i]] = rows_[index][i];
        }
        return row_map;
    }

    std::string getValue(size_t row, size_t col) const override {
        if (row >= rows_.size() || col >= rows_[row].size()) {
            return "";
        }
        return rows_[row][col];
    }

    std::string getValue(size_t row, const std::string& column) const override {
        auto it = std::find(column_names_.begin(), column_names_.end(), column);
        if (it == column_names_.end() || row >= rows_.size()) {
            return "";
        }
        size_t col = std::distance(column_names_.begin(), it);
        return getValue(row, col);
    }

    std::string toJSON() const override {
        Json::Value root;
        root["success"] = success_;
        if (!success_) {
            root["error"] = error_message_;
        }

        Json::Value columns(Json::arrayValue);
        for (const auto& col : column_names_) {
            columns.append(col);
        }
        root["columns"] = columns;

        Json::Value rows_json(Json::arrayValue);
        for (const auto& row : rows_) {
            Json::Value row_json(Json::objectValue);
            for (size_t i = 0; i < column_names_.size() && i < row.size(); ++i) {
                row_json[column_names_[i]] = row[i];
            }
            rows_json.append(row_json);
        }
        root["rows"] = rows_json;

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    }

    std::string toCSV() const override {
        std::ostringstream oss;

        // Header
        for (size_t i = 0; i < column_names_.size(); ++i) {
            if (i > 0) oss << ",";
            oss << column_names_[i];
        }
        oss << "\n";

        // Rows
        for (const auto& row : rows_) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) oss << ",";
                // Quote if contains comma or newline
                if (row[i].find(',') != std::string::npos ||
                    row[i].find('\n') != std::string::npos) {
                    oss << "\"" << row[i] << "\"";
                } else {
                    oss << row[i];
                }
            }
            oss << "\n";
        }

        return oss.str();
    }

    std::string toSARIF() const override {
        Json::Value sarif;
        sarif["version"] = "2.1.0";
        sarif["$schema"] = "https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json";

        Json::Value run;
        Json::Value tool;
        tool["driver"]["name"] = "nomic";
        tool["driver"]["version"] = "1.0.0";
        run["tool"] = tool;

        Json::Value results(Json::arrayValue);
        for (const auto& row_map : getRows()) {
            Json::Value result;

            auto msg_it = row_map.find("message");
            if (msg_it != row_map.end()) {
                result["message"]["text"] = msg_it->second;
            }

            auto rule_it = row_map.find("rule_id");
            if (rule_it != row_map.end()) {
                result["ruleId"] = rule_it->second;
            }

            auto severity_it = row_map.find("severity");
            if (severity_it != row_map.end()) {
                result["level"] = severity_it->second;
            }

            Json::Value location;
            auto file_it = row_map.find("file");
            if (file_it != row_map.end()) {
                location["artifactLocation"]["uri"] = file_it->second;
            }

            auto line_it = row_map.find("line");
            auto col_it = row_map.find("column");
            if (line_it != row_map.end() || col_it != row_map.end()) {
                Json::Value region;
                if (line_it != row_map.end()) {
                    region["startLine"] = std::stoi(line_it->second);
                }
                if (col_it != row_map.end()) {
                    region["startColumn"] = std::stoi(col_it->second);
                }
                location["region"] = region;
            }

            if (!location.empty()) {
                result["locations"][0]["physicalLocation"] = location;
            }

            results.append(result);
        }
        run["results"] = results;

        sarif["runs"][0] = run;

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, sarif);
    }

    std::string toXML() const override {
        std::ostringstream oss;
        oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        oss << "<result success=\"" << (success_ ? "true" : "false") << "\">\n";

        if (!success_) {
            oss << "  <error>" << error_message_ << "</error>\n";
        }

        oss << "  <columns>\n";
        for (const auto& col : column_names_) {
            oss << "    <column>" << col << "</column>\n";
        }
        oss << "  </columns>\n";

        oss << "  <rows>\n";
        for (const auto& row : rows_) {
            oss << "    <row>\n";
            for (size_t i = 0; i < column_names_.size() && i < row.size(); ++i) {
                oss << "      <" << column_names_[i] << ">" << row[i]
                    << "</" << column_names_[i] << ">\n";
            }
            oss << "    </row>\n";
        }
        oss << "  </rows>\n";
        oss << "</result>\n";

        return oss.str();
    }
};

/**
 * @brief Prepared Statement implementation
 */
class SQLPreparedStatement : public ISQLPreparedStatement {
private:
    sqlite3_stmt* stmt_;
    std::string sql_;
    SQLEngine* engine_;
    std::map<size_t, std::string> indexed_params_;
    std::map<std::string, std::string> named_params_;

public:
    SQLPreparedStatement(sqlite3_stmt* stmt, const std::string& sql, SQLEngine* engine)
        : stmt_(stmt), sql_(sql), engine_(engine) {}

    ~SQLPreparedStatement() {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
    }

    SQLResultPtr execute(const std::vector<std::string>& params) override;

    void bindParam(size_t index, const std::string& value) override {
        indexed_params_[index] = value;
        if (stmt_) {
            sqlite3_bind_text(stmt_, index + 1, value.c_str(), -1, SQLITE_TRANSIENT);
        }
    }

    void bindParam(const std::string& name, const std::string& value) override {
        named_params_[name] = value;
        if (stmt_) {
            int index = sqlite3_bind_parameter_index(stmt_, name.c_str());
            if (index > 0) {
                sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT);
            }
        }
    }

    void clearBindings() override {
        indexed_params_.clear();
        named_params_.clear();
        if (stmt_) {
            sqlite3_clear_bindings(stmt_);
        }
    }

    std::string getSQL() const override { return sql_; }
};

/**
 * @brief Main SQL Engine implementation
 */
class SQLEngine : public ISQLEngine {
private:
    sqlite3* db_;
    core::SemanticModelPtr semantic_model_;
    core::ASTNodePtr root_node_;
    std::map<std::string, VirtualTableProvider> virtual_tables_;
    bool in_transaction_;
    std::mutex mutex_;
    bool cache_enabled_;
    std::map<std::string, SQLResultPtr> query_cache_;
    std::chrono::milliseconds timeout_;

    // Virtual table callback data
    struct VTableData {
        std::string name;
        std::vector<std::string> columns;
        VirtualTableProvider provider;
        SQLEngine* engine;
    };

public:
    SQLEngine() : db_(nullptr), in_transaction_(false), cache_enabled_(false),
                  timeout_(std::chrono::milliseconds(5000)) {
        initializeDatabase();
    }

    ~SQLEngine() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    void initializeDatabase() {
        // Open in-memory database
        if (sqlite3_open(":memory:", &db_) != SQLITE_OK) {
            throw std::runtime_error("Failed to open SQLite database");
        }

        // Register default virtual tables
        registerDefaultVirtualTables();
    }

    void registerDefaultVirtualTables() {
        // Register AST virtual table
        registerVirtualTable(VirtualTables::AST,
            {VirtualTables::AST_ID, VirtualTables::AST_TYPE, VirtualTables::AST_PARENT_ID,
             VirtualTables::AST_FILE, VirtualTables::AST_LINE, VirtualTables::AST_COLUMN},
            [this](const std::string&) { return getASTData(); });

        // Register Symbols virtual table
        registerVirtualTable(VirtualTables::SYMBOLS,
            {VirtualTables::SYMBOLS_ID, VirtualTables::SYMBOLS_NAME, VirtualTables::SYMBOLS_KIND,
             VirtualTables::SYMBOLS_TYPE, VirtualTables::SYMBOLS_SCOPE_ID, VirtualTables::SYMBOLS_AST_ID},
            [this](const std::string&) { return getSymbolsData(); });

        // Register Types virtual table
        registerVirtualTable(VirtualTables::TYPES,
            {VirtualTables::TYPES_ID, VirtualTables::TYPES_NAME, VirtualTables::TYPES_KIND,
             VirtualTables::TYPES_SIZE, VirtualTables::TYPES_ALIGNMENT},
            [this](const std::string&) { return getTypesData(); });

        // Register Scopes virtual table
        registerVirtualTable(VirtualTables::SCOPES,
            {VirtualTables::SCOPES_ID, VirtualTables::SCOPES_NAME, VirtualTables::SCOPES_KIND,
             VirtualTables::SCOPES_PARENT_ID, VirtualTables::SCOPES_AST_ID},
            [this](const std::string&) { return getScopesData(); });

        // Register Files virtual table
        registerVirtualTable(VirtualTables::FILES,
            {"id", "path", "name", "size", "lines", "language"},
            [this](const std::string&) { return getFilesData(); });

        // Register Metrics virtual table
        registerVirtualTable(VirtualTables::METRICS,
            {VirtualTables::METRICS_AST_ID, VirtualTables::METRICS_COMPLEXITY,
             VirtualTables::METRICS_LOC, VirtualTables::METRICS_SLOC},
            [this](const std::string&) { return getMetricsData(); });

        // Register CFG virtual table
        registerVirtualTable(VirtualTables::CFG,
            {VirtualTables::CFG_FUNCTION_ID, VirtualTables::CFG_FROM_ID,
             VirtualTables::CFG_TO_ID, VirtualTables::CFG_EDGE_TYPE},
            [this](const std::string&) { return getCFGData(); });

        // Register DataFlow virtual table
        registerVirtualTable(VirtualTables::DATAFLOW,
            {VirtualTables::DATAFLOW_VAR_NAME, VirtualTables::DATAFLOW_DEF_SITE,
             VirtualTables::DATAFLOW_USE_SITES, VirtualTables::DATAFLOW_IS_TAINTED},
            [this](const std::string&) { return getDataFlowData(); });
    }

    // Virtual table data providers
    std::vector<std::vector<std::string>> getASTData() {
        std::vector<std::vector<std::string>> data;
        if (root_node_) {
            collectASTData(root_node_, "", data);
        }
        return data;
    }

    void collectASTData(core::ASTNodePtr node, const std::string& parent_id,
                       std::vector<std::vector<std::string>>& data) {
        if (!node) return;

        std::string node_id = std::to_string(reinterpret_cast<uintptr_t>(node.get()));
        std::string type_str = node->getKindName();

        std::string location = node->getSourceLocation();
        auto line_col = node->getLineColumn();
        std::string file = location;  // For simplicity, using full location as file
        std::string line = std::to_string(line_col.first);
        std::string column = std::to_string(line_col.second);

        data.push_back({node_id, type_str, parent_id, file, line, column});

        for (const auto& child : node->getChildren()) {
            collectASTData(child, node_id, data);
        }
    }

    std::vector<std::vector<std::string>> getSymbolsData() {
        std::vector<std::vector<std::string>> data;
        if (semantic_model_) {
            for (const auto& symbol : semantic_model_->getAllSymbols()) {
                if (!symbol) continue;
                std::string id = std::to_string(reinterpret_cast<uintptr_t>(symbol.get()));
                std::string name = symbol->getName();
                std::string kind = symbolKindToString(symbol->getKind());
                std::string type = symbol->getType() ? symbol->getType()->getTypeName() : "";
                std::string scope_id = symbol->getScope() ?
                    std::to_string(reinterpret_cast<uintptr_t>(symbol->getScope().get())) : "";
                std::string ast_id = ""; // Would need to track AST association

                data.push_back({id, name, kind, type, scope_id, ast_id});
            }
        }
        return data;
    }

    std::string symbolKindToString(core::ISymbol::SymbolKind kind) {
        switch (kind) {
            case core::ISymbol::SymbolKind::FUNCTION: return "FUNCTION";
            case core::ISymbol::SymbolKind::VARIABLE: return "VARIABLE";
            case core::ISymbol::SymbolKind::PARAMETER: return "PARAMETER";
            case core::ISymbol::SymbolKind::FIELD: return "FIELD";
            case core::ISymbol::SymbolKind::TYPEDEF: return "TYPEDEF";
            case core::ISymbol::SymbolKind::STRUCT: return "STRUCT";
            case core::ISymbol::SymbolKind::UNION: return "UNION";
            case core::ISymbol::SymbolKind::ENUM: return "ENUM";
            case core::ISymbol::SymbolKind::ENUMERATOR: return "ENUMERATOR";
            case core::ISymbol::SymbolKind::LABEL: return "LABEL";
            default: return "UNKNOWN";
        }
    }

    std::vector<std::vector<std::string>> getTypesData() {
        std::vector<std::vector<std::string>> data;
        // Stub - would iterate through types in semantic model
        return data;
    }

    std::vector<std::vector<std::string>> getScopesData() {
        std::vector<std::vector<std::string>> data;
        if (semantic_model_) {
            collectScopeData(semantic_model_->getGlobalScope(), "", data);
        }
        return data;
    }

    void collectScopeData(core::ScopePtr scope, const std::string& parent_id,
                         std::vector<std::vector<std::string>>& data) {
        if (!scope) return;

        std::string scope_id = std::to_string(reinterpret_cast<uintptr_t>(scope.get()));
        std::string name = scopeKindToString(scope->getKind());  // Use kind as name for now
        std::string kind = scopeKindToString(scope->getKind());
        std::string ast_id = ""; // Would need to track AST association

        data.push_back({scope_id, name, kind, parent_id, ast_id});

        for (const auto& child : scope->getChildren()) {
            collectScopeData(child, scope_id, data);
        }
    }

    std::string scopeKindToString(core::IScope::ScopeKind kind) {
        switch (kind) {
            case core::IScope::ScopeKind::GLOBAL: return "GLOBAL";
            case core::IScope::ScopeKind::FILE: return "FILE";
            case core::IScope::ScopeKind::FUNCTION: return "FUNCTION";
            case core::IScope::ScopeKind::BLOCK: return "BLOCK";
            case core::IScope::ScopeKind::STRUCT: return "STRUCT";
            case core::IScope::ScopeKind::UNION: return "UNION";
            case core::IScope::ScopeKind::ENUM: return "ENUM";
            case core::IScope::ScopeKind::NAMESPACE: return "NAMESPACE";
            default: return "UNKNOWN";
        }
    }

    std::vector<std::vector<std::string>> getFilesData() {
        // Stub - would return file information
        return {};
    }

    std::vector<std::vector<std::string>> getMetricsData() {
        // Stub - would calculate metrics
        return {};
    }

    std::vector<std::vector<std::string>> getCFGData() {
        // Stub - would return control flow graph
        return {};
    }

    std::vector<std::vector<std::string>> getDataFlowData() {
        // Stub - would return data flow information
        return {};
    }

    // ISQLEngine implementation
    SQLResultPtr execute(const std::string& sql) override {
        std::lock_guard<std::mutex> lock(mutex_);

        // Check cache
        if (cache_enabled_) {
            auto it = query_cache_.find(sql);
            if (it != query_cache_.end()) {
                return it->second;
            }
        }

        auto result = std::make_shared<SQLResult>();

        // Create virtual table data as temporary tables
        for (const auto& [name, provider] : virtual_tables_) {
            createTemporaryTable(name, provider);
        }

        // Execute query
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            result->setError(sqlite3_errmsg(db_));
            return result;
        }

        // Get column names
        int col_count = sqlite3_column_count(stmt);
        std::vector<std::string> columns;
        for (int i = 0; i < col_count; ++i) {
            columns.push_back(sqlite3_column_name(stmt, i));
        }
        result->setColumns(columns);

        // Fetch rows
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::vector<std::string> row;
            for (int i = 0; i < col_count; ++i) {
                const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row.push_back(text ? text : "");
            }
            result->addRow(row);
        }

        sqlite3_finalize(stmt);

        if (rc != SQLITE_DONE) {
            result->setError(sqlite3_errmsg(db_));
        } else {
            result->setSuccess(true);
        }

        // Cache result
        if (cache_enabled_ && result->isSuccess()) {
            query_cache_[sql] = result;
        }

        return result;
    }

    void createTemporaryTable(const std::string& name, VirtualTableProvider provider) {
        // Drop existing temp table
        std::string drop_sql = "DROP TABLE IF EXISTS " + name;
        sqlite3_exec(db_, drop_sql.c_str(), nullptr, nullptr, nullptr);

        // Get data from provider
        auto data = provider("");
        if (data.empty()) return;

        // Assume first row has correct number of columns
        size_t col_count = data[0].size();

        // Create table
        std::ostringstream create_sql;
        create_sql << "CREATE TEMP TABLE " << name << " (";
        for (size_t i = 0; i < col_count; ++i) {
            if (i > 0) create_sql << ", ";
            create_sql << "col" << i << " TEXT";
        }
        create_sql << ")";

        sqlite3_exec(db_, create_sql.str().c_str(), nullptr, nullptr, nullptr);

        // Insert data
        for (const auto& row : data) {
            std::ostringstream insert_sql;
            insert_sql << "INSERT INTO " << name << " VALUES (";
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) insert_sql << ", ";
                insert_sql << "'" << row[i] << "'";
            }
            insert_sql << ")";

            sqlite3_exec(db_, insert_sql.str().c_str(), nullptr, nullptr, nullptr);
        }
    }

    std::vector<SQLResultPtr> executeBatch(const std::vector<std::string>& queries) override {
        std::vector<SQLResultPtr> results;
        for (const auto& query : queries) {
            results.push_back(execute(query));
        }
        return results;
    }

    SQLPreparedStatementPtr prepare(const std::string& sql) override {
        std::lock_guard<std::mutex> lock(mutex_);

        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            return nullptr;
        }

        return std::make_shared<SQLPreparedStatement>(stmt, sql, this);
    }

    void beginTransaction() override {
        std::lock_guard<std::mutex> lock(mutex_);
        sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
        in_transaction_ = true;
    }

    void commit() override {
        std::lock_guard<std::mutex> lock(mutex_);
        sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
        in_transaction_ = false;
    }

    void rollback() override {
        std::lock_guard<std::mutex> lock(mutex_);
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        in_transaction_ = false;
    }

    bool inTransaction() const override { return in_transaction_; }

    bool registerVirtualTable(const std::string& name,
                            const std::vector<std::string>& columns,
                            VirtualTableProvider provider) override {
        std::lock_guard<std::mutex> lock(mutex_);
        virtual_tables_[name] = provider;
        return true;
    }

    bool unregisterVirtualTable(const std::string& name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        return virtual_tables_.erase(name) > 0;
    }

    std::vector<std::string> getRegisteredTables() const override {
        std::vector<std::string> tables;
        for (const auto& [name, _] : virtual_tables_) {
            tables.push_back(name);
        }
        return tables;
    }

    std::vector<std::string> getTableColumns(const std::string& table) const override {
        // Stub - would query SQLite metadata
        return {};
    }

    std::string getTableSchema(const std::string& table) const override {
        // Stub - would query SQLite metadata
        return "";
    }

    SQLResultPtr explain(const std::string& sql) override {
        return execute("EXPLAIN QUERY PLAN " + sql);
    }

    std::string getQueryPlan(const std::string& sql) override {
        auto result = explain(sql);
        if (!result->isSuccess()) {
            return result->getErrorMessage();
        }
        return result->toJSON();
    }

    void setSemanticModel(core::SemanticModelPtr model) override {
        semantic_model_ = model;
    }

    core::SemanticModelPtr getSemanticModel() const override {
        return semantic_model_;
    }

    void setRootNode(core::ASTNodePtr root) override {
        root_node_ = root;
    }

    core::ASTNodePtr getRootNode() const override {
        return root_node_;
    }

    void enableQueryCache(bool enable) override {
        cache_enabled_ = enable;
        if (!enable) {
            clearQueryCache();
        }
    }

    void clearQueryCache() override {
        query_cache_.clear();
    }

    size_t getQueryCacheSize() const override {
        return query_cache_.size();
    }

    void setQueryTimeout(std::chrono::milliseconds timeout) override {
        timeout_ = timeout;
        // Would set SQLite busy timeout
        sqlite3_busy_timeout(db_, timeout.count());
    }

    friend class SQLPreparedStatement;
};

// SQLPreparedStatement::execute implementation
SQLResultPtr SQLPreparedStatement::execute(const std::vector<std::string>& params) {
    auto result = std::make_shared<SQLResult>();

    // Reset statement
    sqlite3_reset(stmt_);

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt_, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    // Get column names
    int col_count = sqlite3_column_count(stmt_);
    std::vector<std::string> columns;
    for (int i = 0; i < col_count; ++i) {
        columns.push_back(sqlite3_column_name(stmt_, i));
    }
    result->setColumns(columns);

    // Execute and fetch rows
    int rc;
    while ((rc = sqlite3_step(stmt_)) == SQLITE_ROW) {
        std::vector<std::string> row;
        for (int i = 0; i < col_count; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, i));
            row.push_back(text ? text : "");
        }
        result->addRow(row);
    }

    if (rc != SQLITE_DONE) {
        result->setError(sqlite3_errstr(rc));
    } else {
        result->setSuccess(true);
    }

    return result;
}

// Factory function
SQLEnginePtr createSQLEngine() {
    return std::make_shared<SQLEngine>();
}

} // namespace query
} // namespace nomic