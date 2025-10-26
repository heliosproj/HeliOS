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

#include "nomic/analysis/taint_analysis.h"
#include <algorithm>
#include <sstream>

namespace nomic::analysis {

// ==================== TaintAnalyzer Implementation ====================

TaintAnalyzer::TaintAnalyzer() {
    loadDefaultPatterns();
}

TaintAnalyzer::~TaintAnalyzer() = default;

void TaintAnalyzer::loadDefaultPatterns() {
    // User input sources
    source_patterns_["scanf"] = TaintSourceType::USER_INPUT;
    source_patterns_["gets"] = TaintSourceType::USER_INPUT;
    source_patterns_["fgets"] = TaintSourceType::USER_INPUT;
    source_patterns_["getchar"] = TaintSourceType::USER_INPUT;
    source_patterns_["getc"] = TaintSourceType::USER_INPUT;
    source_patterns_["fgetc"] = TaintSourceType::USER_INPUT;
    source_patterns_["read"] = TaintSourceType::USER_INPUT;
    source_patterns_["fread"] = TaintSourceType::FILE_INPUT;
    source_patterns_["fscanf"] = TaintSourceType::FILE_INPUT;

    // Network sources
    source_patterns_["recv"] = TaintSourceType::NETWORK_INPUT;
    source_patterns_["recvfrom"] = TaintSourceType::NETWORK_INPUT;
    source_patterns_["recvmsg"] = TaintSourceType::NETWORK_INPUT;

    // Environment sources
    source_patterns_["getenv"] = TaintSourceType::ENVIRONMENT;
    source_patterns_["secure_getenv"] = TaintSourceType::ENVIRONMENT;

    // Command execution sinks
    sink_patterns_["system"] = TaintSinkType::COMMAND_EXECUTION;
    sink_patterns_["popen"] = TaintSinkType::COMMAND_EXECUTION;
    sink_patterns_["execl"] = TaintSinkType::COMMAND_EXECUTION;
    sink_patterns_["execlp"] = TaintSinkType::COMMAND_EXECUTION;
    sink_patterns_["execle"] = TaintSinkType::COMMAND_EXECUTION;
    sink_patterns_["execv"] = TaintSinkType::COMMAND_EXECUTION;
    sink_patterns_["execvp"] = TaintSinkType::COMMAND_EXECUTION;
    sink_patterns_["execve"] = TaintSinkType::COMMAND_EXECUTION;

    // Format string sinks
    sink_patterns_["printf"] = TaintSinkType::FORMAT_STRING;
    sink_patterns_["fprintf"] = TaintSinkType::FORMAT_STRING;
    sink_patterns_["sprintf"] = TaintSinkType::FORMAT_STRING;
    sink_patterns_["snprintf"] = TaintSinkType::FORMAT_STRING;
    sink_patterns_["vprintf"] = TaintSinkType::FORMAT_STRING;
    sink_patterns_["vfprintf"] = TaintSinkType::FORMAT_STRING;
    sink_patterns_["vsprintf"] = TaintSinkType::FORMAT_STRING;
    sink_patterns_["vsnprintf"] = TaintSinkType::FORMAT_STRING;

    // File path sinks
    sink_patterns_["fopen"] = TaintSinkType::FILE_PATH;
    sink_patterns_["open"] = TaintSinkType::FILE_PATH;
    sink_patterns_["creat"] = TaintSinkType::FILE_PATH;

    // Memory operation sinks
    sink_patterns_["strcpy"] = TaintSinkType::MEMORY_OPERATION;
    sink_patterns_["strcat"] = TaintSinkType::MEMORY_OPERATION;
    sink_patterns_["memcpy"] = TaintSinkType::MEMORY_OPERATION;
    sink_patterns_["memmove"] = TaintSinkType::MEMORY_OPERATION;

    // SQL sinks (common libraries)
    sink_patterns_["mysql_query"] = TaintSinkType::SQL_QUERY;
    sink_patterns_["PQexec"] = TaintSinkType::SQL_QUERY;
    sink_patterns_["sqlite3_exec"] = TaintSinkType::SQL_QUERY;

    // Sanitizers
    sanitizer_patterns_.insert("sanitize");
    sanitizer_patterns_.insert("escape");
    sanitizer_patterns_.insert("validate");
    sanitizer_patterns_.insert("filter");
    sanitizer_patterns_.insert("clean");
}

void TaintAnalyzer::addSourcePattern(const std::string& func_name, TaintSourceType type) {
    source_patterns_[func_name] = type;
}

void TaintAnalyzer::addSinkPattern(const std::string& func_name, TaintSinkType type) {
    sink_patterns_[func_name] = type;
}

void TaintAnalyzer::addSanitizerPattern(const std::string& func_name) {
    sanitizer_patterns_.insert(func_name);
}

bool TaintAnalyzer::isTaintSource(const std::string& func_name) const {
    return source_patterns_.find(func_name) != source_patterns_.end();
}

bool TaintAnalyzer::isTaintSink(const std::string& func_name) const {
    return sink_patterns_.find(func_name) != sink_patterns_.end();
}

bool TaintAnalyzer::isSanitizer(const std::string& func_name) const {
    // Check if function name contains any sanitizer pattern
    for (const auto& pattern : sanitizer_patterns_) {
        if (func_name.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

TaintSourceType TaintAnalyzer::getSourceType(const std::string& func_name) const {
    auto it = source_patterns_.find(func_name);
    if (it != source_patterns_.end()) {
        return it->second;
    }
    return TaintSourceType::CUSTOM;
}

TaintSinkType TaintAnalyzer::getSinkType(const std::string& func_name) const {
    auto it = sink_patterns_.find(func_name);
    if (it != sink_patterns_.end()) {
        return it->second;
    }
    return TaintSinkType::CUSTOM;
}

void TaintAnalyzer::identifySources(const Function& func) {
    // Check function parameters (command line arguments)
    for (const auto& param : func.getParameters()) {
        if (param.getName() == "argv" || param.getName() == "argc") {
            TaintSource source(param.getName(), TaintSourceType::COMMAND_LINE, param.getLocation().getStart());
            source.setFunctionName(func.getName());
            source.setDescription("Command line argument");
            sources_.push_back(source);
            tainted_vars_.insert({param.getName(), source});
        }
    }

    // Check function calls for taint sources
    for (const auto& call_site : func.getCallSites()) {
        const std::string& callee = call_site.getName();

        if (isTaintSource(callee)) {
            // Find the variable that receives the tainted data
            // This is typically the first argument for input functions
            // or the return value for functions like getenv

            std::string tainted_var;

            // For functions like scanf, fgets - first argument is the destination
            if (callee == "scanf" || callee == "fscanf" || callee == "gets" ||
                callee == "fgets" || callee == "read" || callee == "fread") {
                // The destination variable is in the arguments
                // For now, we'll use a placeholder - need to track actual arguments
                tainted_var = "tainted_input_" + std::to_string(sources_.size());
            }
            // For functions like getenv - return value is tainted
            else if (callee == "getenv" || callee == "secure_getenv") {
                tainted_var = "env_var_" + std::to_string(sources_.size());
            }

            TaintSource source(tainted_var, getSourceType(callee), call_site.getLocation());
            source.setFunctionName(func.getName());
            source.setDescription("Taint source from " + callee);
            sources_.push_back(source);
            tainted_vars_.insert({tainted_var, source});
        }
    }
}

void TaintAnalyzer::identifySinks(const Function& func) {
    for (const auto& call_site : func.getCallSites()) {
        const std::string& callee = call_site.getName();

        if (isTaintSink(callee)) {
            TaintSink sink(callee, getSinkType(callee), call_site.getLocation());
            sink.setDescription("Potential taint sink");
            sinks_.push_back(sink);
        }
    }
}

void TaintAnalyzer::propagateTaint(const Function& func) {
    // Track taint propagation through variable assignments
    // This is a simplified version - full implementation would use CFG

    // For each statement in the function, check if it's an assignment
    // involving a tainted variable
    for (const auto& stmt : func.getStatements()) {
        // Check if this is an assignment
        // In a full implementation, we would parse the statement and check:
        // 1. If LHS is a new variable and RHS contains tainted variable -> taint LHS
        // 2. If function call returns tainted data -> taint result variable
        // 3. If tainted variable is used in an expression -> taint result

        // For now, we'll use a simple heuristic based on call sites
        for (const auto& call_site : func.getCallSites()) {
            // Check if any tainted variable is used in this call
            // This is simplified - would need actual argument tracking
        }
    }
}

void TaintAnalyzer::detectFlows() {
    // Detect flows from sources to sinks
    for (const auto& source : sources_) {
        for (const auto& sink : sinks_) {
            // Check if there's a path from source variable to sink
            // For now, create a flow if both exist in the same function
            TaintFlow flow(source, sink);

            // Check if the tainted variable reaches the sink
            // This is simplified - full implementation would use data flow analysis
            flow.addToPath(source.getVariableName());

            // Check if there's sanitization in between
            bool sanitized = false;
            // Would check for sanitizer calls here

            flow.setSanitized(sanitized);

            if (!sanitized) {
                taint_flows_.push_back(flow);
            }
        }
    }
}

void TaintAnalyzer::analyzeFunction(const Function& func) {
    // Clear previous results
    sources_.clear();
    sinks_.clear();
    taint_flows_.clear();
    tainted_vars_.clear();

    // Perform analysis
    identifySources(func);
    identifySinks(func);
    propagateTaint(func);
    detectFlows();
}

std::vector<TaintFlow> TaintAnalyzer::getTaintFlows() const {
    return taint_flows_;
}

bool TaintAnalyzer::isVariableTainted(const std::string& var_name) const {
    return tainted_vars_.find(var_name) != tainted_vars_.end();
}

std::vector<TaintSource> TaintAnalyzer::getTaintSources() const {
    return sources_;
}

std::vector<TaintSink> TaintAnalyzer::getTaintSinks() const {
    return sinks_;
}

std::set<std::string> TaintAnalyzer::getTaintedVariables() const {
    std::set<std::string> result;
    for (const auto& pair : tainted_vars_) {
        result.insert(pair.first);
    }
    return result;
}

// ==================== Helper Functions ====================

namespace taint {

std::string sourceTypeToString(TaintSourceType type) {
    switch (type) {
        case TaintSourceType::USER_INPUT: return "USER_INPUT";
        case TaintSourceType::NETWORK_INPUT: return "NETWORK_INPUT";
        case TaintSourceType::FILE_INPUT: return "FILE_INPUT";
        case TaintSourceType::COMMAND_LINE: return "COMMAND_LINE";
        case TaintSourceType::ENVIRONMENT: return "ENVIRONMENT";
        case TaintSourceType::EXTERNAL_API: return "EXTERNAL_API";
        case TaintSourceType::CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

std::string sinkTypeToString(TaintSinkType type) {
    switch (type) {
        case TaintSinkType::COMMAND_EXECUTION: return "COMMAND_EXECUTION";
        case TaintSinkType::SQL_QUERY: return "SQL_QUERY";
        case TaintSinkType::FILE_PATH: return "FILE_PATH";
        case TaintSinkType::FORMAT_STRING: return "FORMAT_STRING";
        case TaintSinkType::MEMORY_OPERATION: return "MEMORY_OPERATION";
        case TaintSinkType::POINTER_DEREF: return "POINTER_DEREF";
        case TaintSinkType::BUFFER_OPERATION: return "BUFFER_OPERATION";
        case TaintSinkType::CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

bool isKnownSource(const std::string& func_name) {
    static const std::set<std::string> known_sources = {
        "scanf", "gets", "fgets", "getchar", "getc", "fgetc",
        "read", "fread", "fscanf",
        "recv", "recvfrom", "recvmsg",
        "getenv", "secure_getenv"
    };
    return known_sources.find(func_name) != known_sources.end();
}

bool isKnownSink(const std::string& func_name) {
    static const std::set<std::string> known_sinks = {
        "system", "popen",
        "execl", "execlp", "execle", "execv", "execvp", "execve",
        "printf", "fprintf", "sprintf", "snprintf",
        "vprintf", "vfprintf", "vsprintf", "vsnprintf",
        "fopen", "open", "creat",
        "strcpy", "strcat", "memcpy", "memmove",
        "mysql_query", "PQexec", "sqlite3_exec"
    };
    return known_sinks.find(func_name) != known_sinks.end();
}

bool isKnownSanitizer(const std::string& func_name) {
    return func_name.find("sanitize") != std::string::npos ||
           func_name.find("escape") != std::string::npos ||
           func_name.find("validate") != std::string::npos ||
           func_name.find("filter") != std::string::npos ||
           func_name.find("clean") != std::string::npos;
}

} // namespace taint

} // namespace nomic::analysis
