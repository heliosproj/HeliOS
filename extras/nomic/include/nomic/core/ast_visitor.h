/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproject.org>
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

#ifndef NOMIC_CORE_AST_VISITOR_H
#define NOMIC_CORE_AST_VISITOR_H

#include "nomic/core/semantic_model.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <stack>

namespace nomic {

/**
 * @brief AST visitor for extracting semantic information from Clang AST
 */
class SemanticASTVisitor : public clang::RecursiveASTVisitor<SemanticASTVisitor> {
public:
    explicit SemanticASTVisitor(clang::ASTContext& ctx, SemanticDatabase& db);
    ~SemanticASTVisitor() = default;

    // Visit declarations
    bool VisitFunctionDecl(clang::FunctionDecl* decl);
    bool VisitVarDecl(clang::VarDecl* decl);
    bool VisitTypedefDecl(clang::TypedefDecl* decl);
    bool VisitRecordDecl(clang::RecordDecl* decl);
    bool VisitEnumDecl(clang::EnumDecl* decl);
    bool VisitFieldDecl(clang::FieldDecl* decl);

    // Visit statements
    bool VisitStmt(clang::Stmt* stmt);
    bool VisitCompoundStmt(clang::CompoundStmt* stmt);
    bool VisitIfStmt(clang::IfStmt* stmt);
    bool VisitForStmt(clang::ForStmt* stmt);
    bool VisitWhileStmt(clang::WhileStmt* stmt);
    bool VisitDoStmt(clang::DoStmt* stmt);
    bool VisitSwitchStmt(clang::SwitchStmt* stmt);
    bool VisitCaseStmt(clang::CaseStmt* stmt);
    bool VisitDefaultStmt(clang::DefaultStmt* stmt);
    bool VisitBreakStmt(clang::BreakStmt* stmt);
    bool VisitContinueStmt(clang::ContinueStmt* stmt);
    bool VisitReturnStmt(clang::ReturnStmt* stmt);
    bool VisitGotoStmt(clang::GotoStmt* stmt);
    bool VisitLabelStmt(clang::LabelStmt* stmt);

    // Visit expressions
    bool VisitCallExpr(clang::CallExpr* expr);
    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr);
    bool VisitBinaryOperator(clang::BinaryOperator* op);
    bool VisitUnaryOperator(clang::UnaryOperator* op);
    bool VisitDeclRefExpr(clang::DeclRefExpr* expr);
    bool VisitMemberExpr(clang::MemberExpr* expr);
    bool VisitCastExpr(clang::CastExpr* expr);
    bool VisitConditionalOperator(clang::ConditionalOperator* op);
    bool VisitInitListExpr(clang::InitListExpr* expr);
    bool VisitStringLiteral(clang::StringLiteral* literal);
    bool VisitIntegerLiteral(clang::IntegerLiteral* literal);
    bool VisitFloatingLiteral(clang::FloatingLiteral* literal);
    bool VisitCharacterLiteral(clang::CharacterLiteral* literal);

    // Preprocessing callbacks
    void HandleMacroDefinition(const clang::Token& macro_name_tok,
                               const clang::MacroDirective* md);
    void HandleInclusionDirective(clang::SourceLocation hash_loc,
                                  const clang::Token& include_tok,
                                  llvm::StringRef file_name,
                                  bool is_angled,
                                  clang::CharSourceRange filename_range,
                                  const clang::FileEntry* file,
                                  llvm::StringRef search_path,
                                  llvm::StringRef relative_path,
                                  const clang::Module* imported);

    // Helper methods
    void setCurrentFunction(Function* func) { current_function_ = func; }
    Function* getCurrentFunction() const { return current_function_; }
    void setCurrentFile(const std::string& file_path);
    FileInfo* getCurrentFile() const { return current_file_; }

private:
    // Helper methods for extracting information
    SourceLocation extractSourceLocation(clang::SourceLocation loc) const;
    SourceRange extractSourceRange(clang::SourceRange range) const;
    TypeInfo extractTypeInfo(clang::QualType type) const;
    std::string extractStatementText(clang::Stmt* stmt) const;
    std::string getQualifiedName(const clang::NamedDecl* decl) const;
    std::string getDeclContext(const clang::Decl* decl) const;

    // CFG building helpers
    void enterBasicBlock();
    void exitBasicBlock();
    void addStatementToCurrentBlock(const Statement& stmt);
    void createBranch(const std::string& condition);
    void mergeBranches();

    // Variable tracking
    void trackVariableUsage(const clang::DeclRefExpr* expr);
    void trackVariableDefinition(const clang::VarDecl* var);

    // Expression evaluation helpers
    std::optional<llvm::APSInt> evaluateAsConstant(const clang::Expr* expr) const;
    std::optional<std::string> getStringLiteralValue(const clang::Expr* expr) const;

private:
    clang::ASTContext& ast_context_;
    SemanticDatabase& semantic_db_;

    // Current context tracking
    Function* current_function_ = nullptr;
    BasicBlock* current_block_ = nullptr;
    FileInfo* current_file_ = nullptr;
    std::stack<Function*> function_stack_;
    std::stack<BasicBlock*> block_stack_;

    // Statement tracking
    std::vector<Statement> pending_statements_;

    // Variable scope tracking
    std::map<const clang::VarDecl*, Variable*> variable_map_;
    std::stack<std::map<std::string, Variable*>> scope_stack_;

    // Preprocessing context
    PreprocessorContext current_preprocessor_context_;
};

/**
 * @brief Consumer for handling AST traversal
 */
class SemanticASTConsumer : public clang::ASTConsumer {
public:
    explicit SemanticASTConsumer(clang::ASTContext& context, SemanticDatabase& db)
        : visitor_(context, db) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        visitor_.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    SemanticASTVisitor visitor_;
};

/**
 * @brief Frontend action for semantic analysis
 */
class SemanticAnalysisAction : public clang::ASTFrontendAction {
public:
    explicit SemanticAnalysisAction(SemanticDatabase& db) : semantic_db_(db) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& CI,
        llvm::StringRef file) override {
        (void)file;
        return std::make_unique<SemanticASTConsumer>(CI.getASTContext(), semantic_db_);
    }

private:
    SemanticDatabase& semantic_db_;
};

} // namespace nomic

#endif // NOMIC_CORE_AST_VISITOR_H