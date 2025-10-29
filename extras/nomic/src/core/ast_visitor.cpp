/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
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

#include "nomic/core/ast_visitor.h"
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/Casting.h>
#include <spdlog/spdlog.h>

using llvm::isa;
using llvm::dyn_cast;

namespace nomic {

// SemanticASTVisitor implementation
SemanticASTVisitor::SemanticASTVisitor(clang::ASTContext& ctx, SemanticDatabase& db)
    : ast_context_(ctx), semantic_db_(db) {
    spdlog::debug("SemanticASTVisitor initialized");
}

bool SemanticASTVisitor::VisitFunctionDecl(clang::FunctionDecl* decl) {
    if (!decl || !decl->hasBody()) {
        return true;
    }

    spdlog::debug("Visiting function: {}", decl->getNameAsString());

    // Track file-level metrics
    auto loc = extractSourceLocation(decl->getLocation());
    if (!loc.getFile().empty()) {
        FileInfo* file_info = semantic_db_.getOrCreateFileInfo(loc.getFile());
        if (file_info) {
            file_info->incrementFunctionCount();
            current_file_ = file_info;
        }
    }

    // Create function object
    auto func = std::make_unique<Function>(
        decl->getNameAsString(),
        decl->getQualifiedNameAsString(),
        extractSourceRange(decl->getSourceRange())
    );

    // Extract parameters
    for (const auto* param : decl->parameters()) {
        Variable param_var(
            param->getNameAsString(),
            extractTypeInfo(param->getType()),
            extractSourceRange(param->getSourceRange())
        );
        param_var.setParameter(true);
        func->addParameter(param_var);
    }

    // Set function properties
    func->setStatic(decl->isStatic());
    func->setInline(decl->isInlineSpecified());

    // Determine linkage
    if (decl->hasExternalFormalLinkage()) {
        func->setLinkage("external");
    } else if (decl->isStatic()) {
        func->setLinkage("internal");
    } else {
        func->setLinkage("none");
    }

    // Initialize cyclomatic complexity (base = 1)
    func->setCyclomaticComplexity(1);

    // Set current function for nested visiting
    current_function_ = func.get();
    function_stack_.push(func.get());

    // Create CFG
    auto cfg = std::make_unique<ControlFlowGraph>();
    func->setCFG(std::move(cfg));

    // Visit function body (this will increment complexity for decision points)
    if (decl->hasBody()) {
        TraverseStmt(decl->getBody());
    }

    // Note: Final complexity is already set by the visitor methods

    // Pop function context
    function_stack_.pop();
    if (!function_stack_.empty()) {
        current_function_ = function_stack_.top();
    } else {
        current_function_ = nullptr;
    }

    // Add function to database
    semantic_db_.addFunction(std::move(func));

    return true;
}

bool SemanticASTVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (!decl) {
        return true;
    }

    spdlog::debug("Visiting variable: {}", decl->getNameAsString());

    TypeInfo type = extractTypeInfo(decl->getType());
    auto var = std::make_unique<Variable>(
        decl->getNameAsString(),
        type,
        extractSourceRange(decl->getSourceRange())
    );

    // Set variable properties
    var->setGlobal(decl->hasGlobalStorage());
    var->setStatic(decl->isStaticLocal() || decl->isStaticDataMember());
    var->setInitialized(decl->hasInit());

    // Extract initial value if present
    if (decl->hasInit()) {
        // Extract the initialization expression text
        std::string init_str = extractStatementText(decl->getInit());
        var->setInitialValue(init_str);
    }

    // Add to appropriate context
    if (decl->hasGlobalStorage()) {
        // Track global variable in file info
        auto loc = extractSourceLocation(decl->getLocation());
        if (!loc.getFile().empty()) {
            FileInfo* file_info = semantic_db_.getOrCreateFileInfo(loc.getFile());
            if (file_info) {
                file_info->incrementGlobalVarCount();
            }
        }
        semantic_db_.addGlobalVariable(std::move(var));
    } else if (current_function_) {
        current_function_->addLocalVariable(*var);
        // NOTE: variable_map_ tracking disabled due to pointer invalidation issue
        // When vector reallocates, all stored pointers become invalid causing bus errors
        // TODO: Implement safe variable tracking using indices or stable containers
    }

    return true;
}

bool SemanticASTVisitor::VisitTypedefDecl(clang::TypedefDecl* decl) {
    if (!decl) {
        return true;
    }

    spdlog::debug("Visiting typedef: {}", decl->getNameAsString());

    TypeInfo type = extractTypeInfo(decl->getUnderlyingType());
    semantic_db_.addTypeDefinition(decl->getNameAsString(), type);

    return true;
}

bool SemanticASTVisitor::VisitRecordDecl(clang::RecordDecl* decl) {
    if (!decl) {
        return true;
    }

    std::string kind = decl->isStruct() ? "struct" : "union";
    spdlog::debug("Visiting {}: {}", kind, decl->getNameAsString());

    // Create type info for the record
    TypeInfo type(decl->getTypeForDecl()->getCanonicalTypeInternal().getAsString());

    // Add to type definitions
    if (!decl->getNameAsString().empty()) {
        semantic_db_.addTypeDefinition(decl->getNameAsString(), type);
    }

    return true;
}

bool SemanticASTVisitor::VisitEnumDecl(clang::EnumDecl* decl) {
    if (!decl) {
        return true;
    }

    spdlog::debug("Visiting enum: {}", decl->getNameAsString());

    TypeInfo type("enum " + decl->getNameAsString());
    semantic_db_.addTypeDefinition(decl->getNameAsString(), type);

    return true;
}

bool SemanticASTVisitor::VisitFieldDecl(clang::FieldDecl* decl) {
    // Handle struct/union fields
    (void)decl;
    return true;
}

bool SemanticASTVisitor::VisitStmt(clang::Stmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }

    // Add statement to current function
    Statement::Type stmt_type = Statement::OTHER;

    if (isa<clang::IfStmt>(stmt)) {
        stmt_type = Statement::IF_STMT;
    } else if (isa<clang::WhileStmt>(stmt)) {
        stmt_type = Statement::WHILE_STMT;
    } else if (isa<clang::ForStmt>(stmt)) {
        stmt_type = Statement::FOR_STMT;
    } else if (isa<clang::ReturnStmt>(stmt)) {
        stmt_type = Statement::RETURN_STMT;
    } else if (isa<clang::CompoundStmt>(stmt)) {
        stmt_type = Statement::COMPOUND_STMT;
    } else if (isa<clang::Expr>(stmt)) {
        stmt_type = Statement::EXPRESSION;
    }

    // Extract the source text for the statement
    std::string text = extractStatementText(stmt);

    Statement s(stmt_type, extractSourceRange(stmt->getSourceRange()), text);
    current_function_->addStatement(s);

    return true;
}

bool SemanticASTVisitor::VisitIfStmt(clang::IfStmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }

    // Update cyclomatic complexity
    unsigned complexity = current_function_->getCyclomaticComplexity();
    current_function_->setCyclomaticComplexity(complexity + 1);

    return true;
}

bool SemanticASTVisitor::VisitForStmt(clang::ForStmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }

    // Update cyclomatic complexity
    unsigned complexity = current_function_->getCyclomaticComplexity();
    current_function_->setCyclomaticComplexity(complexity + 1);

    return true;
}

bool SemanticASTVisitor::VisitWhileStmt(clang::WhileStmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }

    // Update cyclomatic complexity
    unsigned complexity = current_function_->getCyclomaticComplexity();
    current_function_->setCyclomaticComplexity(complexity + 1);

    return true;
}

bool SemanticASTVisitor::VisitCallExpr(clang::CallExpr* expr) {
    if (!expr || !current_function_) {
        return true;
    }

    if (auto* callee = expr->getDirectCallee()) {
        std::string callee_name = callee->getNameAsString();
        SourceLocation call_loc = extractSourceLocation(expr->getExprLoc());

        CallSite call(callee_name, call_loc);

        // Set qualified name
        call.setQualifiedName(callee->getQualifiedNameAsString());

        // Extract the full call expression text
        std::string expr_text = extractStatementText(expr);
        call.setExpressionText(expr_text);

        // Set argument count
        unsigned num_args = expr->getNumArgs();
        call.setArgumentCount(num_args);

        // Extract each argument's text
        for (unsigned i = 0; i < num_args; ++i) {
            if (auto* arg = expr->getArg(i)) {
                std::string arg_text = extractStatementText(arg);
                call.addArgumentExpression(arg_text);
            }
        }

        // Check if this is an indirect call (function pointer)
        call.setIsIndirect(expr->getDirectCallee() == nullptr);

        // For C++, check if this is a virtual call
        if (auto* member_call = dyn_cast<clang::CXXMemberCallExpr>(expr)) {
            if (auto* method = dyn_cast<clang::CXXMethodDecl>(member_call->getMethodDecl())) {
                call.setIsVirtual(method->isVirtual());
            }
        }

        current_function_->addCallSite(call);

        spdlog::debug("Found call to: {} at {} with {} args",
                     callee_name, call_loc.toString(), num_args);
    } else {
        // Handle indirect calls (function pointers)
        SourceLocation call_loc = extractSourceLocation(expr->getExprLoc());
        CallSite call("<indirect>", call_loc);
        call.setIsIndirect(true);
        call.setExpressionText(extractStatementText(expr));
        call.setArgumentCount(expr->getNumArgs());

        for (unsigned i = 0; i < expr->getNumArgs(); ++i) {
            if (auto* arg = expr->getArg(i)) {
                call.addArgumentExpression(extractStatementText(arg));
            }
        }

        current_function_->addCallSite(call);
        spdlog::debug("Found indirect call at {}", call_loc.toString());
    }

    return true;
}

bool SemanticASTVisitor::VisitBinaryOperator(clang::BinaryOperator* op) {
    if (!op || !current_function_) {
        return true;
    }

    // NOTE: Variable usage tracking disabled due to pointer invalidation bug
    // The variable_map_ stored pointers to vector elements which became invalid
    // when the vector reallocated, causing bus errors
    // TODO: Re-implement with safe pointer management

    // Still track operator for completeness
    (void)op;

    return true;
}

bool SemanticASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr* expr) {
    if (!expr) {
        return true;
    }

    trackVariableUsage(expr);
    return true;
}

// Helper methods implementation
SourceLocation SemanticASTVisitor::extractSourceLocation(clang::SourceLocation loc) const {
    if (loc.isInvalid()) {
        return SourceLocation();
    }

    const clang::SourceManager& sm = ast_context_.getSourceManager();
    clang::PresumedLoc ploc = sm.getPresumedLoc(loc);

    if (ploc.isInvalid()) {
        return SourceLocation();
    }

    return SourceLocation(
        ploc.getFilename(),
        ploc.getLine(),
        ploc.getColumn()
    );
}

SourceRange SemanticASTVisitor::extractSourceRange(clang::SourceRange range) const {
    return SourceRange(
        extractSourceLocation(range.getBegin()),
        extractSourceLocation(range.getEnd())
    );
}

TypeInfo SemanticASTVisitor::extractTypeInfo(clang::QualType type) const {
    TypeInfo info;

    // Get canonical type
    clang::QualType canonical = type.getCanonicalType();
    info.setCanonicalType(canonical.getAsString());

    // Get desugared type
    clang::QualType desugared = type.getDesugaredType(ast_context_);
    info.setDesugaredType(desugared.getAsString());

    // Check type properties
    info.setIsPointer(type->isPointerType());
    info.setIsArray(type->isArrayType());
    info.setIsConst(type.isConstQualified());
    info.setIsVolatile(type.isVolatileQualified());

    // Calculate pointer depth
    unsigned depth = 0;
    clang::QualType curr = type;
    while (curr->isPointerType()) {
        depth++;
        curr = curr->getPointeeType();
    }
    info.setPointerDepth(depth);

    // Get array size if applicable
    if (const auto* array_type = dyn_cast<clang::ConstantArrayType>(type)) {
        info.setArraySize(array_type->getSize().getLimitedValue());
    }

    return info;
}

std::string SemanticASTVisitor::extractStatementText(clang::Stmt* stmt) const {
    if (!stmt) {
        return "";
    }

    const clang::SourceManager& sm = ast_context_.getSourceManager();
    clang::SourceRange range = stmt->getSourceRange();

    if (range.isInvalid()) {
        return "";
    }

    // Create a character range from the statement's source range
    clang::CharSourceRange char_range = clang::CharSourceRange::getTokenRange(range);

    // Extract the source text using Clang's Lexer
    llvm::StringRef text = clang::Lexer::getSourceText(
        char_range,
        sm,
        ast_context_.getLangOpts()
    );

    return text.str();
}

std::string SemanticASTVisitor::getQualifiedName(const clang::NamedDecl* decl) const {
    if (!decl) {
        return "";
    }
    return decl->getQualifiedNameAsString();
}

std::string SemanticASTVisitor::getDeclContext(const clang::Decl* decl) const {
    if (!decl) {
        return "";
    }

    std::string context;
    const clang::DeclContext* ctx = decl->getDeclContext();

    while (ctx && !ctx->isTranslationUnit()) {
        if (const auto* named = dyn_cast<clang::NamedDecl>(ctx)) {
            if (!context.empty()) {
                context = "::" + context;
            }
            context = named->getNameAsString() + context;
        }
        ctx = ctx->getParent();
    }

    return context;
}

void SemanticASTVisitor::trackVariableUsage(const clang::DeclRefExpr* expr) {
    // NOTE: Variable usage tracking disabled due to pointer invalidation bug
    // This function relied on variable_map_ which stored pointers to vector elements
    // TODO: Re-implement with safe pointer management
    (void)expr;
    (void)current_function_;
}

void SemanticASTVisitor::trackVariableDefinition(const clang::VarDecl* var) {
    if (!var || !current_function_) {
        return;
    }

    // Track variable definition in current scope
    std::string var_name = var->getNameAsString();
    if (!scope_stack_.empty()) {
        // Add to current scope
        // Implementation would track scoped variables
    }
}

std::optional<llvm::APSInt> SemanticASTVisitor::evaluateAsConstant(const clang::Expr* expr) const {
    if (!expr) {
        return std::nullopt;
    }

    clang::Expr::EvalResult result;
    if (expr->EvaluateAsInt(result, ast_context_)) {
        return result.Val.getInt();
    }

    return std::nullopt;
}

std::optional<std::string> SemanticASTVisitor::getStringLiteralValue(const clang::Expr* expr) const {
    if (!expr) {
        return std::nullopt;
    }

    if (const auto* str_literal = dyn_cast<clang::StringLiteral>(expr)) {
        return str_literal->getString().str();
    }

    return std::nullopt;
}

// Additional visitor methods that were declared but not implemented
bool SemanticASTVisitor::VisitCompoundStmt(clang::CompoundStmt* stmt) {
    // Track compound statement blocks
    (void)stmt;
    return true;
}

bool SemanticASTVisitor::VisitDoStmt(clang::DoStmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }
    // Update cyclomatic complexity
    unsigned complexity = current_function_->getCyclomaticComplexity();
    current_function_->setCyclomaticComplexity(complexity + 1);
    return true;
}

bool SemanticASTVisitor::VisitSwitchStmt(clang::SwitchStmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }
    // Update cyclomatic complexity for each case
    return true;
}

bool SemanticASTVisitor::VisitCaseStmt(clang::CaseStmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }
    // Update cyclomatic complexity
    unsigned complexity = current_function_->getCyclomaticComplexity();
    current_function_->setCyclomaticComplexity(complexity + 1);
    return true;
}

bool SemanticASTVisitor::VisitDefaultStmt(clang::DefaultStmt* stmt) {
    (void)stmt;
    return true;
}

bool SemanticASTVisitor::VisitBreakStmt(clang::BreakStmt* stmt) {
    (void)stmt;
    return true;
}

bool SemanticASTVisitor::VisitContinueStmt(clang::ContinueStmt* stmt) {
    (void)stmt;
    return true;
}

bool SemanticASTVisitor::VisitReturnStmt(clang::ReturnStmt* stmt) {
    if (!stmt || !current_function_) {
        return true;
    }

    SourceLocation loc = extractSourceLocation(stmt->getReturnLoc());
    ReturnStatement ret_stmt(loc);

    // Check if there's a return value
    if (auto* ret_value = stmt->getRetValue()) {
        ret_stmt.setHasValue(true);
        std::string expr_text = extractStatementText(ret_value);
        ret_stmt.setReturnExpression(expr_text);
        spdlog::debug("Found return statement with value: {} at {}", expr_text, loc.toString());
    } else {
        ret_stmt.setHasValue(false);
        spdlog::debug("Found return statement (void) at {}", loc.toString());
    }

    current_function_->addReturnStatement(ret_stmt);

    return true;
}

bool SemanticASTVisitor::VisitGotoStmt(clang::GotoStmt* stmt) {
    (void)stmt;
    return true;
}

bool SemanticASTVisitor::VisitLabelStmt(clang::LabelStmt* stmt) {
    (void)stmt;
    return true;
}

bool SemanticASTVisitor::VisitUnaryOperator(clang::UnaryOperator* op) {
    (void)op;
    return true;
}

bool SemanticASTVisitor::VisitMemberExpr(clang::MemberExpr* expr) {
    (void)expr;
    return true;
}

bool SemanticASTVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr) {
    if (!expr || !current_function_) {
        return true;
    }

    // Extract array name from base expression
    std::string array_name = "<unknown>";
    if (auto* base_ref = dyn_cast<clang::DeclRefExpr>(expr->getBase()->IgnoreParenCasts())) {
        array_name = base_ref->getNameInfo().getAsString();
    }

    SourceLocation loc = extractSourceLocation(expr->getExprLoc());
    ArrayAccess access(array_name, loc);

    // Extract index expression text
    if (auto* idx = expr->getIdx()) {
        std::string idx_text = extractStatementText(idx);
        access.setIndexExpression(idx_text);
    }

    // Extract full expression text
    std::string full_text = extractStatementText(expr);
    access.setFullExpression(full_text);

    // TODO: Determine if this is a write access (would need parent analysis)
    // For now, mark as read by default
    access.setIsWrite(false);

    current_function_->addArrayAccess(access);

    spdlog::debug("Found array access: {} at {}", full_text, loc.toString());

    return true;
}

bool SemanticASTVisitor::VisitCastExpr(clang::CastExpr* expr) {
    (void)expr;
    return true;
}

bool SemanticASTVisitor::VisitConditionalOperator(clang::ConditionalOperator* op) {
    if (!op || !current_function_) {
        return true;
    }
    // Ternary operator adds to complexity
    unsigned complexity = current_function_->getCyclomaticComplexity();
    current_function_->setCyclomaticComplexity(complexity + 1);
    return true;
}

bool SemanticASTVisitor::VisitInitListExpr(clang::InitListExpr* expr) {
    (void)expr;
    return true;
}

bool SemanticASTVisitor::VisitStringLiteral(clang::StringLiteral* literal) {
    (void)literal;
    return true;
}

bool SemanticASTVisitor::VisitIntegerLiteral(clang::IntegerLiteral* literal) {
    (void)literal;
    return true;
}

bool SemanticASTVisitor::VisitFloatingLiteral(clang::FloatingLiteral* literal) {
    (void)literal;
    return true;
}

bool SemanticASTVisitor::VisitCharacterLiteral(clang::CharacterLiteral* literal) {
    (void)literal;
    return true;
}

// Preprocessing callbacks
void SemanticASTVisitor::HandleMacroDefinition(const clang::Token& macro_name_tok,
                                              const clang::MacroDirective* md) {
    // Track macro definitions
    (void)macro_name_tok;
    (void)md;
}

void SemanticASTVisitor::HandleInclusionDirective(clang::SourceLocation hash_loc,
                                                 const clang::Token& include_tok,
                                                 llvm::StringRef file_name,
                                                 bool is_angled,
                                                 clang::CharSourceRange filename_range,
                                                 const clang::FileEntry* file,
                                                 llvm::StringRef search_path,
                                                 llvm::StringRef relative_path,
                                                 const clang::Module* imported) {
    // Track include directives in file info
    auto loc = extractSourceLocation(hash_loc);
    if (!loc.getFile().empty()) {
        FileInfo* file_info = semantic_db_.getOrCreateFileInfo(loc.getFile());
        if (file_info) {
            file_info->incrementIncludeCount();
        }
    }

    (void)include_tok;
    (void)file_name;
    (void)is_angled;
    (void)filename_range;
    (void)file;
    (void)search_path;
    (void)relative_path;
    (void)imported;
}

// CFG building helpers
void SemanticASTVisitor::enterBasicBlock() {
    // Implementation for entering a new basic block
}

void SemanticASTVisitor::exitBasicBlock() {
    // Implementation for exiting a basic block
}

void SemanticASTVisitor::addStatementToCurrentBlock(const Statement& stmt) {
    if (current_block_) {
        current_block_->addStatement(stmt);
    }
}

void SemanticASTVisitor::createBranch(const std::string& condition) {
    // Implementation for creating a branch in CFG
    (void)condition;
}

void SemanticASTVisitor::mergeBranches() {
    // Implementation for merging branches in CFG
}

void SemanticASTVisitor::setCurrentFile(const std::string& file_path) {
    current_file_ = semantic_db_.getOrCreateFileInfo(file_path);
}

} // namespace nomic