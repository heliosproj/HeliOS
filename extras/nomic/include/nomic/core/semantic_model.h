/*
 * Nomic C Semantic Source Code Analyzer Copyright (C) 2025 Manny Peterson
 * <me@mannypeterson.com>
 *
 * This file is part of Nomic.
 *
 * Nomic is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Nomic is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Nomic. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NOMIC_CORE_SEMANTIC_MODEL_H
#define NOMIC_CORE_SEMANTIC_MODEL_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <map>
#include <set>

namespace nomic {
// Forward declarations
  class ControlFlowGraph;


/**
 * @brief Represents a location in source code
 */
  class SourceLocation {
public: SourceLocation() = default;
    SourceLocation(const std::string &file, unsigned line, unsigned column) : file_(file), line_(line), column_(column) {
    } const std::string & getFile() const {
      return(file_);
    }


    unsigned getLine() const {
      return(line_);
    }


    unsigned getColumn() const {
      return(column_);
    }


    std::string toString() const;
    bool operator == (const SourceLocation &other) const;
    bool operator < (const SourceLocation &other) const;
private: std::string file_;
    unsigned line_ = 0;
    unsigned column_ = 0;
  };


/**
 * @brief Represents a range in source code
 */
  class SourceRange {
public: SourceRange() = default;
    SourceRange(const SourceLocation &start, const SourceLocation &end) : start_(start), end_(end) {
    } const SourceLocation & getStart() const {
      return(start_);
    }


    const SourceLocation & getEnd() const {
      return(end_);
    }


    bool contains(const SourceLocation &loc) const;
    bool overlaps(const SourceRange &other) const;
    std::string toString() const;
private: SourceLocation start_;
    SourceLocation end_;
  };


/**
 * @brief Preprocessor context information
 */
  class PreprocessorContext {
public: void addDefine(const std::string &define) {
      active_defines_.push_back(define);
    }


    void setGuardCondition(const std::string &guard) {
      guard_condition_ = guard;
    }


    void setInMacroExpansion(bool in_macro) {
      in_macro_expansion_ = in_macro;
    }


    void setExpandedFrom(const std::string &macro) {
      expanded_from_ = macro;
    }


    const std::vector < std::string > &getActiveDefines() const {
      return(active_defines_);
    } const std::optional < std::string > &getGuardCondition() const {
      return(guard_condition_);
    } bool isInMacroExpansion() const {
      return(in_macro_expansion_);
    }


    const std::optional < std::string > &getExpandedFrom() const {
      return(expanded_from_);
    } private: std::vector < std::string > active_defines_;
    std::optional < std::string > guard_condition_;
    bool in_macro_expansion_ = false;
    std::optional < std::string > expanded_from_;
  };


/**
 * @brief Type information for variables and expressions
 */
  class TypeInfo {
public: TypeInfo() = default;


    explicit TypeInfo(const std::string &canonical_type) : canonical_type_(canonical_type) {
    }


    // Setters
    void setCanonicalType(const std::string &type) {
      canonical_type_ = type;
    }


    void setDesugaredType(const std::string &type) {
      desugared_type_ = type;
    }


    void setIsPointer(bool is_ptr) {
      is_pointer_ = is_ptr;
    }


    void setIsArray(bool is_array) {
      is_array_ = is_array;
    }


    void setIsConst(bool is_const) {
      is_const_ = is_const;
    }


    void setIsVolatile(bool is_volatile) {
      is_volatile_ = is_volatile;
    }


    void setIsStatic(bool is_static) {
      is_static_ = is_static;
    }


    void setPointerDepth(unsigned depth) {
      pointer_depth_ = depth;
    }


    void setArraySize(size_t size) {
      array_size_ = size;
    }


    // Getters
    const std::string & getCanonicalType() const {
      return(canonical_type_);
    }


    const std::string & getDesugaredType() const {
      return(desugared_type_);
    }


    bool isPointer() const {
      return(is_pointer_);
    }


    bool isArray() const {
      return(is_array_);
    }


    bool isConst() const {
      return(is_const_);
    }


    bool isVolatile() const {
      return(is_volatile_);
    }


    bool isStatic() const {
      return(is_static_);
    }


    unsigned getPointerDepth() const {
      return(pointer_depth_);
    }


    const std::optional < size_t > &getArraySize() const {
      return(array_size_);
    } std::string toString() const;
private: std::string canonical_type_;
    std::string desugared_type_;
    bool is_pointer_ = false;
    bool is_array_ = false;
    bool is_const_ = false;
    bool is_volatile_ = false;
    bool is_static_ = false;
    unsigned pointer_depth_ = 0;
    std::optional < size_t > array_size_;
  };


/**
 * @brief Represents a variable in the semantic model
 */
/**
 * @brief Represents a statement in the code
 */
  class Statement {
public: enum Type {
      EXPRESSION,
      IF_STMT,
      WHILE_STMT,
      FOR_STMT,
      RETURN_STMT,
      COMPOUND_STMT,
      OTHER
    };
    Statement(Type type = OTHER, const SourceRange &loc = SourceRange(), const std::string &text = "") : type_(type), location_(loc), text_(text) {
    } Type getType() const {
      return(type_);
    }


    const SourceRange & getLocation() const {
      return(location_);
    }


    const std::string & getText() const {
      return(text_);
    }


private: Type type_;
    SourceRange location_;
    std::string text_;
  };


/**
 * @brief Represents a function call site
 */
  class CallSite {
public: CallSite(const std::string &name = "", const SourceLocation &loc = SourceLocation()) : name_(name), location_(loc) {
    }


    // Getters
    const std::string & getName() const {
      return(name_);
    }


    const SourceLocation & getLocation() const {
      return(location_);
    }


    const std::string & getQualifiedName() const {
      return(qualified_name_);
    }


    const std::string & getExpressionText() const {
      return(expression_text_);
    }


    unsigned getArgumentCount() const {
      return(argument_count_);
    }


    const std::vector < std::string > &getArgumentExpressions() const {
      return(argument_expressions_);
    } bool isVirtual() const {
      return(is_virtual_);
    }


    bool isIndirect() const {
      return(is_indirect_);
    }


    // Setters
    void setQualifiedName(const std::string &name) {
      qualified_name_ = name;
    }


    void setExpressionText(const std::string &text) {
      expression_text_ = text;
    }


    void setArgumentCount(unsigned count) {
      argument_count_ = count;
    }


    void addArgumentExpression(const std::string &arg) {
      argument_expressions_.push_back(arg);
    }


    void setIsVirtual(bool is_virtual) {
      is_virtual_ = is_virtual;
    }


    void setIsIndirect(bool is_indirect) {
      is_indirect_ = is_indirect;
    }


private: std::string name_;                      // Simple function name
    std::string qualified_name_; // Fully qualified name
    SourceLocation location_; // Location of the call
    std::string expression_text_; // Full call expression as text
    unsigned argument_count_ = 0; // Number of arguments
    std::vector < std::string > argument_expressions_; // Text of each argument
    bool is_virtual_ = false; // Is this a virtual call?
    bool is_indirect_ = false; // Is this an indirect (function pointer) call?
  };


/**
 * @brief Represents an array subscript access
 */
  class ArrayAccess {
public: ArrayAccess(const std::string &array_name = "", const SourceLocation &loc = SourceLocation()) : array_name_(array_name), location_(loc) {
    }


    // Getters
    const std::string & getArrayName() const {
      return(array_name_);
    }


    const SourceLocation & getLocation() const {
      return(location_);
    }


    const std::string & getIndexExpression() const {
      return(index_expression_);
    }


    const std::string & getFullExpression() const {
      return(full_expression_);
    }


    bool isWrite() const {
      return(is_write_);
    }


    // Setters
    void setIndexExpression(const std::string &index) {
      index_expression_ = index;
    }


    void setFullExpression(const std::string &expr) {
      full_expression_ = expr;
    }


    void setIsWrite(bool is_write) {
      is_write_ = is_write;
    }


private: std::string array_name_;      // Name of the array being accessed
    SourceLocation location_; // Location of the access
    std::string index_expression_; // Text of the index expression (e.g., "i+1")
    std::string full_expression_; // Full access expression (e.g., "arr[i+1]")
    bool is_write_ = false; // Is this a write access?
  };


/**
 * @brief Represents a return statement with its expression
 */
  class ReturnStatement {
public: ReturnStatement(const SourceLocation &loc = SourceLocation()) : location_(loc) {
    }


    // Getters
    const SourceLocation & getLocation() const {
      return(location_);
    }


    const std::string & getReturnExpression() const {
      return(return_expression_);
    }


    bool hasValue() const {
      return(has_value_);
    }


    // Setters
    void setReturnExpression(const std::string &expr) {
      return_expression_ = expr;
    }


    void setHasValue(bool has_value) {
      has_value_ = has_value;
    }


private: SourceLocation location_;
    std::string return_expression_;
    bool has_value_ = false;
  };


/**
 * @brief Represents an annotation/attribute
 */
  class Annotation {
public: Annotation(const std::string &name = "", const std::string &value = "") : name_(name), value_(value) {
    } const std::string & getName() const {
      return(name_);
    }


    const std::string & getValue() const {
      return(value_);
    }


private: std::string name_;
    std::string value_;
  };
  class Variable {
public: Variable(const std::string &name, const TypeInfo &type, const SourceRange &location) : name_(name), type_(type), location_(location) {
    }


    // Setters
    void setGlobal(bool is_global) {
      is_global_ = is_global;
    }


    void setStatic(bool is_static) {
      is_static_ = is_static;
    }


    void setParameter(bool is_param) {
      is_parameter_ = is_param;
    }


    void setInitialized(bool is_init) {
      is_initialized_ = is_init;
    }


    void setInitialValue(const std::string &value) {
      initial_value_ = value;
    }


    void addReadSite(const SourceLocation &loc) {
      read_sites_.push_back(loc);
    }


    void addWriteSite(const SourceLocation &loc) {
      write_sites_.push_back(loc);
    }


    // Taint tracking setters
    void setTainted(bool is_tainted) {
      is_tainted_ = is_tainted;
    }


    void setTaintSource(const std::string &source) {
      taint_source_ = source;
    }


    void setTaintConfidence(float confidence) {
      taint_confidence_ = confidence;
    }


    // Getters
    const std::string & getName() const {
      return(name_);
    }


    const TypeInfo & getType() const {
      return(type_);
    }


    const SourceRange & getLocation() const {
      return(location_);
    }


    bool isGlobal() const {
      return(is_global_);
    }


    bool isStatic() const {
      return(is_static_);
    }


    bool isParameter() const {
      return(is_parameter_);
    }


    bool isInitialized() const {
      return(is_initialized_);
    }


    const std::optional < std::string > &getInitialValue() const {
      return(initial_value_);
    } const std::vector < SourceLocation > &getReadSites() const {
      return(read_sites_);
    } const std::vector < SourceLocation > &getWriteSites() const {
      return(write_sites_);
    }


    // Taint tracking getters
    bool isTainted() const {
      return(is_tainted_);
    }


    const std::optional < std::string > &getTaintSource() const {
      return(taint_source_);
    } float getTaintConfidence() const {
      return(taint_confidence_);
    }


private: std::string name_;
    TypeInfo type_;
    SourceRange location_;
    bool is_global_ = false;
    bool is_static_ = false;
    bool is_parameter_ = false;
    bool is_initialized_ = false;
    std::vector < SourceLocation > read_sites_;
    std::vector < SourceLocation > write_sites_;
    std::optional < std::string > initial_value_;
    // Taint tracking fields
    bool is_tainted_ = false;
    std::optional < std::string > taint_source_; // Source function/origin of
                                                 // taint
    float taint_confidence_ = 0.0f; // Confidence level (0.0 - 1.0)
  };


/**
 * @brief File-level information for TIER 3 properties
 */
  class FileInfo {
public: FileInfo(const std::string &path) : path_(path) {
      // Extract file name and extension from path
      size_t last_slash = path.find_last_of("/\\");
      name_ = (last_slash != std::string::npos) ? path.substr(last_slash + 1) : path;
      size_t last_dot = name_.find_last_of('.');
      extension_ = (last_dot != std::string::npos) ? name_.substr(last_dot) : "";
      // Check if header based on extension
      is_header_ = (extension_ == ".h" || extension_ == ".hpp" || extension_ == ".hh");
    }


    // Setters
    void setLineCount(size_t count) {
      line_count_ = count;
    }


    void setFunctionCount(size_t count) {
      function_count_ = count;
    }


    void setGlobalVarCount(size_t count) {
      global_var_count_ = count;
    }


    void setIncludeCount(size_t count) {
      include_count_ = count;
    }


    void setHasHeaderGuard(bool has_guard) {
      has_header_guard_ = has_guard;
    }


    void addSystemInclude(const std::string &include) {
      system_includes_.push_back(include);
    }


    void incrementFunctionCount() {
      function_count_++;
    }


    void incrementGlobalVarCount() {
      global_var_count_++;
    }


    void incrementIncludeCount() {
      include_count_++;
    }


    // Getters
    const std::string & getPath() const {
      return(path_);
    }


    const std::string & getName() const {
      return(name_);
    }


    const std::string & getExtension() const {
      return(extension_);
    }


    size_t getLineCount() const {
      return(line_count_);
    }


    size_t getFunctionCount() const {
      return(function_count_);
    }


    size_t getGlobalVarCount() const {
      return(global_var_count_);
    }


    size_t getIncludeCount() const {
      return(include_count_);
    }


    bool hasHeaderGuard() const {
      return(has_header_guard_);
    }


    bool isHeader() const {
      return(is_header_);
    }


    const std::vector < std::string > &getSystemIncludes() const {
      return(system_includes_);
    } private: std::string path_;
    std::string name_;
    std::string extension_;
    size_t line_count_ = 0;
    size_t function_count_ = 0;
    size_t global_var_count_ = 0;
    size_t include_count_ = 0;
    bool has_header_guard_ = false;
    bool is_header_ = false;
    std::vector < std::string > system_includes_;
  };


/**
 * @brief Represents a function in the semantic model
 */
  class Function {
public: Function(const std::string &name, const std::string &signature, const SourceRange &location) : name_(name), signature_(signature), location_(location) {
    }


    // Setters
    void setReturnType(const TypeInfo &type) {
      return_type_ = type;
    }


    void addParameter(const Variable &param) {
      parameters_.push_back(param);
    }


    void addLocalVariable(const Variable &var) {
      local_vars_.push_back(var);
    }


    void addCallSite(const CallSite &call);


    void addArrayAccess(const ArrayAccess &access) {
      array_accesses_.push_back(access);
    }


    void addReturnStatement(const ReturnStatement &ret) {
      return_statements_.push_back(ret);
    }


    void setCFG(std::unique_ptr < ControlFlowGraph > cfg);
    void addStatement(const Statement &stmt);


    void setCyclomaticComplexity(unsigned complexity) {
      cyclomatic_complexity_ = complexity;
    }


    void setPreprocessorContext(const PreprocessorContext &ctx) {
      preprocessor_ = ctx;
    }


    void addAnnotation(const Annotation &annotation);


    void setStatic(bool is_static) {
      is_static_ = is_static;
    }


    void setInline(bool is_inline) {
      is_inline_ = is_inline;
    }


    void setLinkage(const std::string &linkage) {
      linkage_ = linkage;
    }


    // Getters
    const std::string & getName() const {
      return(name_);
    }


    const std::string & getSignature() const {
      return(signature_);
    }


    const TypeInfo & getReturnType() const {
      return(return_type_);
    }


    const std::vector < Variable > &getParameters() const {
      return(parameters_);
    } const std::vector < Variable > &getLocalVariables() const {
      return(local_vars_);
    } const std::vector < CallSite > &getCallSites() const {
      return(calls_);
    } const std::vector < ArrayAccess > &getArrayAccesses() const {
      return(array_accesses_);
    } const std::vector < ReturnStatement > &getReturnStatements() const {
      return(return_statements_);
    } ControlFlowGraph * getCFG() const {
      return(cfg_.get());
    }


    const std::vector < Statement > &getStatements() const {
      return(statements_);
    } unsigned getCyclomaticComplexity() const {
      return(cyclomatic_complexity_);
    }


    const SourceRange & getLocation() const {
      return(location_);
    }


    const PreprocessorContext & getPreprocessorContext() const {
      return(preprocessor_);
    }


    const std::vector < Annotation > &getAnnotations() const {
      return(annotations_);
    } bool isStatic() const {
      return(is_static_);
    }


    bool isInline() const {
      return(is_inline_);
    }


    const std::string & getLinkage() const {
      return(linkage_);
    }


private: std::string name_;
    std::string signature_;
    TypeInfo return_type_;
    std::vector < Variable > parameters_;
    std::vector < Variable > local_vars_;
    std::vector < CallSite > calls_;
    std::vector < ArrayAccess > array_accesses_;
    std::vector < ReturnStatement > return_statements_;
    std::unique_ptr < ControlFlowGraph > cfg_;
    std::vector < Statement > statements_;
    unsigned cyclomatic_complexity_ = 1;
    SourceRange location_;
    PreprocessorContext preprocessor_;
    std::vector < Annotation > annotations_;
    bool is_static_ = false;
    bool is_inline_ = false;
    std::string linkage_ = "external";
  };


/**
 * @brief Basic block in control flow graph
 */
  class BasicBlock {
public: explicit BasicBlock(unsigned id) : id_(id) {
    }
    // Setters
    void addStatement(const Statement &stmt);


    void addPredecessor(unsigned pred_id) {
      predecessors_.insert(pred_id);
    }


    void addSuccessor(unsigned succ_id) {
      successors_.insert(succ_id);
    }


    void setEntry(bool is_entry) {
      is_entry_ = is_entry;
    }


    void setExit(bool is_exit) {
      is_exit_ = is_exit;
    }


    void setCondition(const std::string &condition) {
      condition_ = condition;
    }


    // Getters
    unsigned getId() const {
      return(id_);
    }


    const std::vector < Statement > &getStatements() const {
      return(statements_);
    } const std::set < unsigned > &getPredecessors() const {
      return(predecessors_);
    } const std::set < unsigned > &getSuccessors() const {
      return(successors_);
    } bool isEntry() const {
      return(is_entry_);
    }


    bool isExit() const {
      return(is_exit_);
    }


    const std::optional < std::string > &getCondition() const {
      return(condition_);
    } private: unsigned id_;
    std::vector < Statement > statements_;
    std::set < unsigned > predecessors_;
    std::set < unsigned > successors_;
    bool is_entry_ = false;
    bool is_exit_ = false;
    std::optional < std::string > condition_;
  };


/**
 * @brief Control flow graph for a function
 */
  class ControlFlowGraph {
public: ControlFlowGraph();
    // Builder methods
    BasicBlock * createBasicBlock();
    void setEntryBlock(BasicBlock *block);
    void addExitBlock(BasicBlock *block);
    void addEdge(BasicBlock *from, BasicBlock *to);
    // Analysis methods
    void computeDominanceTree();
    void computePostDominanceTree();
    bool dominates(const BasicBlock *dominator, const BasicBlock *dominated) const;
    bool postDominates(const BasicBlock *post_dominator, const BasicBlock *dominated) const;
    // Data flow analysis
    void computeReachingDefinitions();
    void computeLiveVariables();
    void computeDefUseChains();
    void computeUseDefChains();
    // Taint analysis
    void computeTaintPropagation(const std::set < std::string > &taint_sources);
    void markVariableTainted(const std::string &var_name, unsigned block_id);
    bool isVariableTaintedAt(const std::string &var_name, unsigned block_id) const;
    std::set < std::string > getTaintedVariablesAt(unsigned block_id) const;
    std::map < std::string, std::vector < std::string >> getTaintPaths(const std::string &var_name) const;
    // Data flow queries
    std::set < std::string > getReachingDefinitions(unsigned block_id) const;
    std::set < std::string > getLiveVariablesIn(unsigned block_id) const;
    std::set < std::string > getLiveVariablesOut(unsigned block_id) const;
    std::set < unsigned > getDefSites(const std::string &variable) const;
    std::set < unsigned > getUseSites(const std::string &variable) const;
    std::vector < unsigned > getDefUseChain(const std::string &variable, unsigned def_site) const;
    std::vector < unsigned > getUseDefChain(const std::string &variable, unsigned use_site) const;
    // Getters
    BasicBlock * getEntryBlock() const;
    std::vector < BasicBlock * > getExitBlocks() const;
    const std::vector < std::unique_ptr < BasicBlock >> &getBlocks() const {
      return(blocks_);
    } const std::map < unsigned, std::set < unsigned >> &getDominanceTree() const {
      return(dominance_tree_);
    } const std::map < unsigned, unsigned > &getImmediateDominators() const {
      return(immediate_dominators_);
    } private: std::vector < std::unique_ptr < BasicBlock >> blocks_;
    unsigned entry_block_id_ = 0;
    std::set < unsigned > exit_block_ids_;
    std::map < unsigned, std::set < unsigned >> dominance_tree_;
    std::map < unsigned, unsigned > immediate_dominators_;
    std::map < unsigned, std::set < unsigned >> post_dominance_tree_;
    // Data flow analysis results
    std::map < unsigned, std::set < std::string >> reaching_defs_in_; // Reaching
                                                                      // definitions
                                                                      // at
                                                                      // entry
    std::map < unsigned, std::set < std::string >> reaching_defs_out_; // Reaching
                                                                       // definitions
                                                                       // at
                                                                       // exit
    std::map < unsigned, std::set < std::string >> live_vars_in_; // Live
                                                                  // variables
                                                                  // at entry
    std::map < unsigned, std::set < std::string >> live_vars_out_; // Live
                                                                   // variables
                                                                   // at exit
    // Def-use and use-def chains
    // Format: variable -> {def_site -> [use_sites]}
    std::map < std::string, std::map < unsigned, std::vector < unsigned >> > def_use_chains_;
    // Format: variable -> {use_site -> [def_sites]}
    std::map < std::string, std::map < unsigned, std::vector < unsigned >> > use_def_chains_;
    // Variable definitions and uses per block
    std::map < unsigned, std::set < std::string >> gen_defs_; // Variables
                                                              // defined in
                                                              // block
    std::map < unsigned, std::set < std::string >> kill_defs_; // Definitions
                                                               // killed in
                                                               // block
    std::map < unsigned, std::set < std::string >> gen_uses_; // Variables used
                                                              // in block
    std::map < unsigned, std::set < std::string >> def_sites_; // Blocks where
                                                               // variable is
                                                               // defined
    std::map < unsigned, std::set < std::string >> use_sites_; // Blocks where
                                                               // variable is
                                                               // used
    // Taint analysis results
    std::map < unsigned, std::set < std::string >> tainted_vars_in_; // Tainted
                                                                     // variables
                                                                     // at entry
    std::map < unsigned, std::set < std::string >> tainted_vars_out_; // Tainted
                                                                      // variables
                                                                      // at exit
    std::map < std::string, std::vector < std::string >> taint_paths_; // Taint
                                                                       // propagation
                                                                       // paths
  };


/**
 * @brief Pattern match result
 */
  class PatternMatch {
public: PatternMatch(const SourceRange &location, const std::string &matched_text) : location_(location), matched_text_(matched_text) {
    } void addCapture(const std::string &name, const std::string &value) {
      captures_[name] = value;
    }


    void setConfidenceScore(double score) {
      confidence_score_ = score;
    }


    const SourceRange & getLocation() const {
      return(location_);
    }


    const std::map < std::string, std::string > &getCaptures() const {
      return(captures_);
    } const std::string & getMatchedText() const {
      return(matched_text_);
    }


    double getConfidenceScore() const {
      return(confidence_score_);
    }


private: SourceRange location_;
    std::map < std::string, std::string > captures_;
    std::string matched_text_;
    double confidence_score_ = 1.0;
  };


/**
 * @brief Semantic database storing all analyzed entities
 */
  class SemanticDatabase {
public: SemanticDatabase() = default;
    // Add entities
    void addFunction(std::unique_ptr < Function > func);
    void addGlobalVariable(std::unique_ptr < Variable > var);
    void addTypeDefinition(const std::string &name, const TypeInfo &type);
    // File tracking
    FileInfo * getOrCreateFileInfo(const std::string &file_path);
    FileInfo * findFileInfo(const std::string &file_path) const;
    // Query methods
    Function * findFunction(const std::string &name) const;
    Variable * findGlobalVariable(const std::string &name) const;
    std::vector < Function * > getFunctionsInFile(const std::string &file) const;
    // Getters
    const std::vector < std::unique_ptr < Function >> &getFunctions() const {
      return(functions_);
    } const std::vector < std::unique_ptr < Variable >> &getGlobalVariables() const {
      return(global_variables_);
    } const std::map < std::string, TypeInfo > &getTypeDefinitions() const {
      return(type_definitions_);
    } const std::map < std::string, std::unique_ptr < FileInfo >> &getFiles() const {
      return(files_);
    } private: std::vector < std::unique_ptr < Function >> functions_;
    std::vector < std::unique_ptr < Variable >> global_variables_;
    std::map < std::string, TypeInfo > type_definitions_;
    std::map < std::string, std::unique_ptr < FileInfo >> files_;
    // Indexes for fast lookup
    std::map < std::string, Function * > function_index_;
    std::map < std::string, Variable * > variable_index_;
    std::map < std::string, std::vector < Function * >> file_function_index_;
  };
} // namespace nomic

#endif // NOMIC_CORE_SEMANTIC_MODEL_H