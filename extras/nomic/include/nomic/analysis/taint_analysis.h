/*
 * Nomic C Semantic Source Code Analyzer
 * Copyright (C) 2025 Manny Peterson <me@mannypeterson.com>
 *
 * This file is part of Nomic.
 *
 * Nomic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nomic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nomic. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NOMIC_ANALYSIS_TAINT_ANALYSIS_H
#define NOMIC_ANALYSIS_TAINT_ANALYSIS_H

#include "nomic/core/semantic_model.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <optional>

namespace nomic::analysis {

/**
 * @brief Type of taint source
 */
enum class TaintSourceType {
    USER_INPUT,          // scanf, gets, fgets, getenv, etc.
    NETWORK_INPUT,       // recv, recvfrom, read from socket
    FILE_INPUT,          // fread, fscanf, read from file
    COMMAND_LINE,        // argv, argc
    ENVIRONMENT,         // getenv, environ
    EXTERNAL_API,        // Any external function
    CUSTOM              // User-defined source
};

/**
 * @brief Type of taint sink
 */
enum class TaintSinkType {
    COMMAND_EXECUTION,   // system, exec*, popen
    SQL_QUERY,           // SQL-related functions
    FILE_PATH,           // fopen, open with path
    FORMAT_STRING,       // printf, sprintf with format
    MEMORY_OPERATION,    // strcpy, memcpy without bounds
    POINTER_DEREF,       // Dereferencing tainted pointer
    BUFFER_OPERATION,    // Array access with tainted index
    CUSTOM              // User-defined sink
};

/**
 * @brief Represents a taint source in the code
 */
class TaintSource {
public:
    TaintSource(const std::string& var_name, TaintSourceType type, SourceLocation loc)
        : variable_name_(var_name), source_type_(type), location_(loc) {}

    const std::string& getVariableName() const { return variable_name_; }
    TaintSourceType getSourceType() const { return source_type_; }
    const SourceLocation& getLocation() const { return location_; }
    const std::string& getFunctionName() const { return function_name_; }

    void setFunctionName(const std::string& name) { function_name_ = name; }
    void setDescription(const std::string& desc) { description_ = desc; }
    const std::string& getDescription() const { return description_; }

private:
    std::string variable_name_;
    TaintSourceType source_type_;
    SourceLocation location_;
    std::string function_name_;
    std::string description_;
};

/**
 * @brief Represents a taint sink in the code
 */
class TaintSink {
public:
    TaintSink(const std::string& func_name, TaintSinkType type, SourceLocation loc)
        : function_name_(func_name), sink_type_(type), location_(loc) {}

    const std::string& getFunctionName() const { return function_name_; }
    TaintSinkType getSinkType() const { return sink_type_; }
    const SourceLocation& getLocation() const { return location_; }
    const std::vector<std::string>& getTaintedArgs() const { return tainted_args_; }

    void addTaintedArg(const std::string& arg) { tainted_args_.push_back(arg); }
    void setDescription(const std::string& desc) { description_ = desc; }
    const std::string& getDescription() const { return description_; }

private:
    std::string function_name_;
    TaintSinkType sink_type_;
    SourceLocation location_;
    std::vector<std::string> tainted_args_;
    std::string description_;
};

/**
 * @brief Represents a taint flow from source to sink
 */
class TaintFlow {
public:
    TaintFlow(const TaintSource& src, const TaintSink& sink)
        : source_(src), sink_(sink), confidence_(1.0f) {}

    const TaintSource& getSource() const { return source_; }
    const TaintSink& getSink() const { return sink_; }
    const std::vector<std::string>& getPath() const { return propagation_path_; }
    float getConfidence() const { return confidence_; }

    void addToPath(const std::string& var) { propagation_path_.push_back(var); }
    void setConfidence(float conf) { confidence_ = conf; }
    void setSanitized(bool sanitized) { is_sanitized_ = sanitized; }
    bool isSanitized() const { return is_sanitized_; }

private:
    TaintSource source_;
    TaintSink sink_;
    std::vector<std::string> propagation_path_;
    float confidence_ = 1.0f;
    bool is_sanitized_ = false;
};

/**
 * @brief Taint analysis engine
 */
class TaintAnalyzer {
public:
    TaintAnalyzer();
    ~TaintAnalyzer();

    // Configuration
    void addSourcePattern(const std::string& func_name, TaintSourceType type);
    void addSinkPattern(const std::string& func_name, TaintSinkType type);
    void addSanitizerPattern(const std::string& func_name);

    // Analysis
    void analyzeFunction(const Function& func);
    std::vector<TaintFlow> getTaintFlows() const;

    // Queries
    bool isVariableTainted(const std::string& var_name) const;
    std::vector<TaintSource> getTaintSources() const;
    std::vector<TaintSink> getTaintSinks() const;
    std::set<std::string> getTaintedVariables() const;

    // Built-in source/sink patterns
    void loadDefaultPatterns();

private:
    // Track tainted variables
    std::map<std::string, TaintSource> tainted_vars_;

    // Source and sink patterns
    std::map<std::string, TaintSourceType> source_patterns_;
    std::map<std::string, TaintSinkType> sink_patterns_;
    std::set<std::string> sanitizer_patterns_;

    // Analysis results
    std::vector<TaintSource> sources_;
    std::vector<TaintSink> sinks_;
    std::vector<TaintFlow> taint_flows_;

    // Helper methods
    void identifySources(const Function& func);
    void identifySinks(const Function& func);
    void propagateTaint(const Function& func);
    void detectFlows();

    bool isTaintSource(const std::string& func_name) const;
    bool isTaintSink(const std::string& func_name) const;
    bool isSanitizer(const std::string& func_name) const;

    TaintSourceType getSourceType(const std::string& func_name) const;
    TaintSinkType getSinkType(const std::string& func_name) const;
};

/**
 * @brief Helper functions for taint analysis
 */
namespace taint {

// Get source type as string
std::string sourceTypeToString(TaintSourceType type);

// Get sink type as string
std::string sinkTypeToString(TaintSinkType type);

// Check if function is a known source
bool isKnownSource(const std::string& func_name);

// Check if function is a known sink
bool isKnownSink(const std::string& func_name);

// Check if function is a sanitizer
bool isKnownSanitizer(const std::string& func_name);

} // namespace taint

} // namespace nomic::analysis

#endif // NOMIC_ANALYSIS_TAINT_ANALYSIS_H
