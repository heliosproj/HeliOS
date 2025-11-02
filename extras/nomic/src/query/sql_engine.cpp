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
#include <set>
#include <map>

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

    // Virtual table callback data
    struct VTableData {
        std::string name;
        std::vector<std::string> columns;
        VirtualTableProvider provider;
        SQLEngine* engine;
    };

    std::map<std::string, VTableData> virtual_tables_;
    bool in_transaction_;
    std::mutex mutex_;
    bool cache_enabled_;
    std::map<std::string, SQLResultPtr> query_cache_;
    std::chrono::milliseconds timeout_;

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
            {VirtualTables::AST_ID, "name", VirtualTables::AST_TYPE, VirtualTables::AST_PARENT_ID,
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
        std::string name = node->getSourceText();  // Use source text as name
        std::string type_str = node->getKindName();

        std::string location = node->getSourceLocation();
        auto line_col = node->getLineColumn();
        std::string file = location;  // For simplicity, using full location as file
        std::string line = std::to_string(line_col.first);
        std::string column = std::to_string(line_col.second);

        data.push_back({node_id, name, type_str, parent_id, file, line, column});

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
        if (semantic_model_) {
            // Collect all unique types from symbols
            std::set<core::TypeInfoPtr> unique_types;
            for (const auto& symbol : semantic_model_->getAllSymbols()) {
                if (symbol && symbol->getType()) {
                    unique_types.insert(symbol->getType());

                    // Also add canonical type if different
                    auto canonical = symbol->getType()->getCanonicalType();
                    if (canonical) {
                        unique_types.insert(canonical);
                    }
                }
            }

            // Add type information for each unique type
            for (const auto& type : unique_types) {
                if (!type) continue;

                std::string id = std::to_string(reinterpret_cast<uintptr_t>(type.get()));
                std::string name = type->getTypeName();
                std::string kind = typeKindToString(type->getKind());
                std::string size = std::to_string(type->getSize());
                std::string alignment = std::to_string(type->getAlignment());

                data.push_back({id, name, kind, size, alignment});
            }
        }
        return data;
    }

    std::string typeKindToString(core::ITypeInfo::TypeKind kind) {
        switch (kind) {
            case core::ITypeInfo::TypeKind::VOID: return "VOID";
            case core::ITypeInfo::TypeKind::BOOL: return "BOOL";
            case core::ITypeInfo::TypeKind::CHAR: return "CHAR";
            case core::ITypeInfo::TypeKind::INT: return "INT";
            case core::ITypeInfo::TypeKind::FLOAT: return "FLOAT";
            case core::ITypeInfo::TypeKind::DOUBLE: return "DOUBLE";
            case core::ITypeInfo::TypeKind::POINTER: return "POINTER";
            case core::ITypeInfo::TypeKind::ARRAY: return "ARRAY";
            case core::ITypeInfo::TypeKind::FUNCTION: return "FUNCTION";
            case core::ITypeInfo::TypeKind::STRUCT: return "STRUCT";
            case core::ITypeInfo::TypeKind::UNION: return "UNION";
            case core::ITypeInfo::TypeKind::ENUM: return "ENUM";
            case core::ITypeInfo::TypeKind::TYPEDEF: return "TYPEDEF";
            case core::ITypeInfo::TypeKind::QUALIFIED: return "QUALIFIED";
            default: return "UNKNOWN";
        }
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
        std::vector<std::vector<std::string>> data;
        if (root_node_) {
            // Collect unique file paths from AST nodes
            std::map<std::string, std::vector<core::ASTNodePtr>> fileNodes;
            collectFileNodes(root_node_, fileNodes);

            int file_id = 1;
            for (const auto& [path, nodes] : fileNodes) {
                if (path.empty()) continue;

                // Extract file name from path
                size_t last_slash = path.find_last_of("/\\");
                std::string name = (last_slash != std::string::npos) ?
                                  path.substr(last_slash + 1) : path;

                // Count lines (approximate from node line numbers)
                int max_line = 0;
                for (const auto& node : nodes) {
                    auto line_col = node->getLineColumn();
                    max_line = std::max(max_line, line_col.first);
                }

                std::string id = std::to_string(file_id++);
                std::string size = "0";  // Would need file system access
                std::string lines = std::to_string(max_line);
                std::string language = detectLanguage(name);

                data.push_back({id, path, name, size, lines, language});
            }
        }
        return data;
    }

    void collectFileNodes(core::ASTNodePtr node,
                         std::map<std::string, std::vector<core::ASTNodePtr>>& fileNodes) {
        if (!node) return;

        std::string location = node->getSourceLocation();
        // Extract file path from location (format: "file:line:column")
        size_t colon = location.find(':');
        std::string file_path = (colon != std::string::npos) ?
                               location.substr(0, colon) : location;

        if (!file_path.empty()) {
            fileNodes[file_path].push_back(node);
        }

        for (const auto& child : node->getChildren()) {
            collectFileNodes(child, fileNodes);
        }
    }

    std::string detectLanguage(const std::string& filename) {
        auto endsWith = [](const std::string& str, const std::string& suffix) {
            if (suffix.size() > str.size()) return false;
            return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        };

        if (endsWith(filename, ".c")) return "C";
        if (endsWith(filename, ".cpp") || endsWith(filename, ".cc") ||
            endsWith(filename, ".cxx")) return "C++";
        if (endsWith(filename, ".h") || endsWith(filename, ".hpp")) return "Header";
        return "Unknown";
    }

    std::vector<std::vector<std::string>> getMetricsData() {
        std::vector<std::vector<std::string>> data;
        if (semantic_model_) {
            // Calculate metrics for each function
            auto functions = semantic_model_->getNodesByKind(core::IASTNode::NodeKind::FUNCTION_DECL);
            for (const auto& func : functions) {
                if (!func) continue;

                std::string ast_id = std::to_string(reinterpret_cast<uintptr_t>(func.get()));

                // Calculate metrics
                CodeMetrics metrics = calculateMetrics(func);

                std::string complexity = std::to_string(metrics.complexity);
                std::string loc = std::to_string(metrics.loc);
                std::string statements = std::to_string(metrics.statements);
                std::string branches = std::to_string(metrics.branches);
                std::string depth = std::to_string(metrics.max_depth);

                data.push_back({ast_id, complexity, loc, statements, branches, depth});
            }
        }
        return data;
    }

    struct CodeMetrics {
        int complexity = 1;  // Start with 1 (base complexity)
        int loc = 0;         // Lines of code
        int statements = 0;
        int branches = 0;
        int max_depth = 0;
    };

    CodeMetrics calculateMetrics(core::ASTNodePtr node, int current_depth = 0) {
        CodeMetrics metrics;
        if (!node) return metrics;

        metrics.max_depth = current_depth;

        // Count this node based on its kind
        auto kind = node->getKind();
        switch (kind) {
            case core::IASTNode::NodeKind::IF_STMT:
            case core::IASTNode::NodeKind::WHILE_STMT:
            case core::IASTNode::NodeKind::FOR_STMT:
            case core::IASTNode::NodeKind::DO_STMT:
                metrics.complexity++;
                metrics.branches++;
                metrics.statements++;
                break;

            case core::IASTNode::NodeKind::SWITCH_STMT:
                metrics.complexity++;
                metrics.statements++;
                break;

            case core::IASTNode::NodeKind::CASE_STMT:
                metrics.complexity++;
                metrics.branches++;
                break;

            case core::IASTNode::NodeKind::RETURN_STMT:
            case core::IASTNode::NodeKind::BREAK_STMT:
            case core::IASTNode::NodeKind::CONTINUE_STMT:
            case core::IASTNode::NodeKind::GOTO_STMT:
            case core::IASTNode::NodeKind::EXPR_STMT:
            case core::IASTNode::NodeKind::DECL_STMT:
                metrics.statements++;
                break;

            default:
                break;
        }

        // Approximate LOC from line numbers
        auto line_col = node->getLineColumn();
        if (line_col.first > 0) {
            metrics.loc = std::max(metrics.loc, 1);
        }

        // Recursively calculate for children
        for (const auto& child : node->getChildren()) {
            CodeMetrics child_metrics = calculateMetrics(child, current_depth + 1);
            metrics.complexity += child_metrics.complexity - 1;  // Subtract base to avoid double counting
            metrics.loc = std::max(metrics.loc, child_metrics.loc);
            metrics.statements += child_metrics.statements;
            metrics.branches += child_metrics.branches;
            metrics.max_depth = std::max(metrics.max_depth, child_metrics.max_depth);
        }

        return metrics;
    }

    std::vector<std::vector<std::string>> getCFGData() {
        std::vector<std::vector<std::string>> data;
        if (semantic_model_) {
            // Build control flow graph for each function
            auto functions = semantic_model_->getNodesByKind(core::IASTNode::NodeKind::FUNCTION_DECL);
            for (const auto& func : functions) {
                if (!func) continue;
                std::string func_id = std::to_string(reinterpret_cast<uintptr_t>(func.get()));
                buildCFG(func, func_id, data);
            }
        }
        return data;
    }

    void buildCFG(core::ASTNodePtr node, const std::string& func_id,
                  std::vector<std::vector<std::string>>& data,
                  const std::string& from_id = "") {
        if (!node) return;

        std::string node_id = std::to_string(reinterpret_cast<uintptr_t>(node.get()));
        auto kind = node->getKind();

        // Add edge from previous node to current
        if (!from_id.empty()) {
            data.push_back({func_id, from_id, node_id, "sequential"});
        }

        // Handle control flow nodes specially
        switch (kind) {
            case core::IASTNode::NodeKind::IF_STMT: {
                auto children = node->getChildren();
                if (children.size() >= 1) {
                    // Condition edge
                    data.push_back({func_id, node_id, std::to_string(reinterpret_cast<uintptr_t>(children[0].get())), "condition"});
                }
                if (children.size() >= 2) {
                    // True branch
                    data.push_back({func_id, node_id, std::to_string(reinterpret_cast<uintptr_t>(children[1].get())), "true"});
                }
                if (children.size() >= 3) {
                    // False branch
                    data.push_back({func_id, node_id, std::to_string(reinterpret_cast<uintptr_t>(children[2].get())), "false"});
                }
                break;
            }

            case core::IASTNode::NodeKind::WHILE_STMT:
            case core::IASTNode::NodeKind::FOR_STMT:
            case core::IASTNode::NodeKind::DO_STMT: {
                // Loop back edge
                auto children = node->getChildren();
                if (!children.empty()) {
                    data.push_back({func_id, node_id, std::to_string(reinterpret_cast<uintptr_t>(children[0].get())), "loop_entry"});
                    // Back edge from last statement to loop condition
                    if (children.size() > 1) {
                        data.push_back({func_id, std::to_string(reinterpret_cast<uintptr_t>(children.back().get())), node_id, "loop_back"});
                    }
                }
                break;
            }

            case core::IASTNode::NodeKind::SWITCH_STMT: {
                auto children = node->getChildren();
                for (size_t i = 0; i < children.size(); ++i) {
                    if (children[i]->getKind() == core::IASTNode::NodeKind::CASE_STMT) {
                        data.push_back({func_id, node_id, std::to_string(reinterpret_cast<uintptr_t>(children[i].get())), "case"});
                    }
                }
                break;
            }

            case core::IASTNode::NodeKind::RETURN_STMT:
                data.push_back({func_id, node_id, "EXIT", "return"});
                break;

            case core::IASTNode::NodeKind::BREAK_STMT:
                data.push_back({func_id, node_id, "BREAK_TARGET", "break"});
                break;

            case core::IASTNode::NodeKind::CONTINUE_STMT:
                data.push_back({func_id, node_id, "CONTINUE_TARGET", "continue"});
                break;

            default:
                // For other nodes, continue building CFG recursively
                std::string last_child_id = node_id;
                for (const auto& child : node->getChildren()) {
                    buildCFG(child, func_id, data, last_child_id);
                    last_child_id = std::to_string(reinterpret_cast<uintptr_t>(child.get()));
                }
                break;
        }
    }

    std::vector<std::vector<std::string>> getDataFlowData() {
        std::vector<std::vector<std::string>> data;
        if (semantic_model_) {
            // Track variable definitions and uses
            auto variables = semantic_model_->findSymbolsByKind(core::ISymbol::SymbolKind::VARIABLE);
            variables.insert(variables.end(),
                semantic_model_->findSymbolsByKind(core::ISymbol::SymbolKind::PARAMETER).begin(),
                semantic_model_->findSymbolsByKind(core::ISymbol::SymbolKind::PARAMETER).end());

            for (const auto& var : variables) {
                if (!var) continue;

                std::string var_name = var->getName();
                std::string def_site = var->getDeclaration() ?
                    std::to_string(reinterpret_cast<uintptr_t>(var->getDeclaration().get())) : "";

                // Get all references (use sites)
                auto refs = var->getReferences();
                std::string use_sites;
                for (size_t i = 0; i < refs.size(); ++i) {
                    if (i > 0) use_sites += ",";
                    use_sites += std::to_string(reinterpret_cast<uintptr_t>(refs[i].get()));
                }

                // Simple taint analysis: assume external inputs are tainted
                bool is_tainted = (var_name.find("input") != std::string::npos ||
                                  var_name.find("user") != std::string::npos ||
                                  var_name.find("arg") != std::string::npos);

                std::string flow_type = "local";
                if (var->isGlobal()) flow_type = "global";
                else if (var->getKind() == core::ISymbol::SymbolKind::PARAMETER) flow_type = "parameter";

                data.push_back({var_name, def_site, use_sites,
                               is_tainted ? "true" : "false", flow_type});
            }
        }
        return data;
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
        for (const auto& [name, vtable_data] : virtual_tables_) {
            createTemporaryTable(name, vtable_data.columns, vtable_data.provider);
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

    void createTemporaryTable(const std::string& name, const std::vector<std::string>& columns, VirtualTableProvider provider) {
        // Drop existing temp table
        std::string drop_sql = "DROP TABLE IF EXISTS " + name;
        sqlite3_exec(db_, drop_sql.c_str(), nullptr, nullptr, nullptr);

        // Create table with actual column names - always create even if empty
        std::ostringstream create_sql;
        create_sql << "CREATE TEMP TABLE " << name << " (";
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) create_sql << ", ";
            create_sql << columns[i] << " TEXT";
        }
        create_sql << ")";

        sqlite3_exec(db_, create_sql.str().c_str(), nullptr, nullptr, nullptr);

        // Get data from provider and insert if available
        auto data = provider("");
        if (!data.empty()) {
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

        // Create virtual table data as temporary tables first
        for (const auto& [name, vtable_data] : virtual_tables_) {
            createTemporaryTable(name, vtable_data.columns, vtable_data.provider);
        }

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
        VTableData data;
        data.name = name;
        data.columns = columns;
        data.provider = provider;
        data.engine = this;
        virtual_tables_[name] = data;
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
        auto it = virtual_tables_.find(table);
        if (it != virtual_tables_.end()) {
            return it->second.columns;
        }
        return {};
    }

    std::string getTableSchema(const std::string& table) const override {
        auto it = virtual_tables_.find(table);
        if (it != virtual_tables_.end()) {
            std::ostringstream schema;
            schema << "CREATE TABLE " << table << " (";
            const auto& columns = it->second.columns;
            for (size_t i = 0; i < columns.size(); ++i) {
                if (i > 0) schema << ", ";
                schema << columns[i] << " TEXT";
            }
            schema << ")";
            return schema.str();
        }
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