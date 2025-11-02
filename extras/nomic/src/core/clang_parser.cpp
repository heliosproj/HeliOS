/**
 * @file clang_parser.cpp
 * @brief Clang/LLVM integration for parsing C source code
 */

#include "nomic/core/clang_parser.h"
#include "nomic/core/ast_factory.h"
#include "nomic/core/semantic_model.h"
#include "nomic/core/type_info.h"
#include "nomic/core/symbol.h"
#include "nomic/core/scope.h"

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/Support/raw_ostream.h>

#include <map>
#include <sstream>
#include <fstream>

namespace nomic {
namespace core {

/**
 * @brief Visitor that converts Clang AST to Nomic AST
 */
class ClangASTVisitor : public clang::RecursiveASTVisitor<ClangASTVisitor> {
public:
    ClangASTVisitor(clang::ASTContext& context, ASTFactory& factory, SemanticModel& model)
        : context_(context), factory_(factory), model_(model) {}

    bool VisitTranslationUnitDecl(clang::TranslationUnitDecl* decl) {
        if (!rootNode_) {
            rootNode_ = factory_.createNode(IASTNode::NodeKind::TRANSLATION_UNIT);
            currentNode_ = rootNode_;
        }
        return true;
    }

    bool VisitFunctionDecl(clang::FunctionDecl* decl) {
        auto funcNode = factory_.createNode(IASTNode::NodeKind::FUNCTION_DECL);

        // Set source location
        setSourceLocation(funcNode, decl->getLocation());

        // Get function name
        std::string funcName = decl->getNameAsString();

        // Create function symbol
        auto funcSymbol = std::make_shared<Symbol>(funcName, ISymbol::SymbolKind::FUNCTION);

        // Map Clang type to Nomic type
        auto funcType = convertType(decl->getType());
        funcSymbol->setType(funcType);

        // Add to semantic model
        model_.addSymbol(funcSymbol);

        // Store node mapping
        nodeMap_[decl] = funcNode;

        return true;
    }

    bool VisitVarDecl(clang::VarDecl* decl) {
        auto varNode = factory_.createNode(IASTNode::NodeKind::VAR_DECL);

        setSourceLocation(varNode, decl->getLocation());

        std::string varName = decl->getNameAsString();
        auto varSymbol = std::make_shared<Symbol>(varName, ISymbol::SymbolKind::VARIABLE);

        auto varType = convertType(decl->getType());
        varSymbol->setType(varType);

        model_.addSymbol(varSymbol);

        nodeMap_[decl] = varNode;

        return true;
    }

    bool VisitParmVarDecl(clang::ParmVarDecl* decl) {
        auto paramNode = factory_.createNode(IASTNode::NodeKind::PARAM_DECL);

        setSourceLocation(paramNode, decl->getLocation());

        std::string paramName = decl->getNameAsString();
        auto paramSymbol = std::make_shared<Symbol>(paramName, ISymbol::SymbolKind::PARAMETER);

        auto paramType = convertType(decl->getType());
        paramSymbol->setType(paramType);

        model_.addSymbol(paramSymbol);

        nodeMap_[decl] = paramNode;

        return true;
    }

    bool VisitFieldDecl(clang::FieldDecl* decl) {
        auto fieldNode = factory_.createNode(IASTNode::NodeKind::FIELD_DECL);

        setSourceLocation(fieldNode, decl->getLocation());

        std::string fieldName = decl->getNameAsString();
        auto fieldSymbol = std::make_shared<Symbol>(fieldName, ISymbol::SymbolKind::FIELD);

        auto fieldType = convertType(decl->getType());
        fieldSymbol->setType(fieldType);

        model_.addSymbol(fieldSymbol);

        nodeMap_[decl] = fieldNode;

        return true;
    }

    bool VisitRecordDecl(clang::RecordDecl* decl) {
        if (!decl->isCompleteDefinition()) {
            return true; // Skip forward declarations
        }

        auto kind = decl->isStruct() ? IASTNode::NodeKind::STRUCT_DECL :
                    decl->isUnion() ? IASTNode::NodeKind::UNION_DECL :
                    IASTNode::NodeKind::STRUCT_DECL;

        auto recordNode = factory_.createNode(kind);
        setSourceLocation(recordNode, decl->getLocation());

        std::string recordName = decl->getNameAsString();
        if (recordName.empty()) {
            recordName = "<anonymous>";
        }

        auto symbolKind = decl->isStruct() ? ISymbol::SymbolKind::STRUCT :
                          decl->isUnion() ? ISymbol::SymbolKind::UNION :
                          ISymbol::SymbolKind::STRUCT;

        auto recordSymbol = std::make_shared<Symbol>(recordName, symbolKind);

        auto recordType = convertType(context_.getRecordType(decl));
        recordSymbol->setType(recordType);

        model_.addSymbol(recordSymbol);

        nodeMap_[decl] = recordNode;

        return true;
    }

    bool VisitEnumDecl(clang::EnumDecl* decl) {
        auto enumNode = factory_.createNode(IASTNode::NodeKind::ENUM_DECL);

        setSourceLocation(enumNode, decl->getLocation());

        std::string enumName = decl->getNameAsString();
        if (enumName.empty()) {
            enumName = "<anonymous>";
        }

        auto enumSymbol = std::make_shared<Symbol>(enumName, ISymbol::SymbolKind::ENUM);

        auto enumType = convertType(context_.getEnumType(decl));
        enumSymbol->setType(enumType);

        model_.addSymbol(enumSymbol);

        nodeMap_[decl] = enumNode;

        return true;
    }

    bool VisitTypedefDecl(clang::TypedefDecl* decl) {
        auto typedefNode = factory_.createNode(IASTNode::NodeKind::TYPE_DEF);

        setSourceLocation(typedefNode, decl->getLocation());

        std::string typedefName = decl->getNameAsString();
        auto typedefSymbol = std::make_shared<Symbol>(typedefName, ISymbol::SymbolKind::TYPEDEF);

        auto underlyingType = convertType(decl->getUnderlyingType());
        typedefSymbol->setType(underlyingType);

        model_.addSymbol(typedefSymbol);

        nodeMap_[decl] = typedefNode;

        return true;
    }

    bool VisitIfStmt(clang::IfStmt* stmt) {
        auto ifNode = factory_.createNode(IASTNode::NodeKind::IF_STMT);
        setSourceLocation(ifNode, stmt->getIfLoc());
        nodeMap_[stmt] = ifNode;
        return true;
    }

    bool VisitWhileStmt(clang::WhileStmt* stmt) {
        auto whileNode = factory_.createNode(IASTNode::NodeKind::WHILE_STMT);
        setSourceLocation(whileNode, stmt->getWhileLoc());
        nodeMap_[stmt] = whileNode;
        return true;
    }

    bool VisitForStmt(clang::ForStmt* stmt) {
        auto forNode = factory_.createNode(IASTNode::NodeKind::FOR_STMT);
        setSourceLocation(forNode, stmt->getForLoc());
        nodeMap_[stmt] = forNode;
        return true;
    }

    bool VisitDoStmt(clang::DoStmt* stmt) {
        auto doNode = factory_.createNode(IASTNode::NodeKind::DO_STMT);
        setSourceLocation(doNode, stmt->getDoLoc());
        nodeMap_[stmt] = doNode;
        return true;
    }

    bool VisitSwitchStmt(clang::SwitchStmt* stmt) {
        auto switchNode = factory_.createNode(IASTNode::NodeKind::SWITCH_STMT);
        setSourceLocation(switchNode, stmt->getSwitchLoc());
        nodeMap_[stmt] = switchNode;
        return true;
    }

    bool VisitCaseStmt(clang::CaseStmt* stmt) {
        auto caseNode = factory_.createNode(IASTNode::NodeKind::CASE_STMT);
        setSourceLocation(caseNode, stmt->getCaseLoc());
        nodeMap_[stmt] = caseNode;
        return true;
    }

    bool VisitReturnStmt(clang::ReturnStmt* stmt) {
        auto returnNode = factory_.createNode(IASTNode::NodeKind::RETURN_STMT);
        setSourceLocation(returnNode, stmt->getReturnLoc());
        nodeMap_[stmt] = returnNode;
        return true;
    }

    bool VisitBreakStmt(clang::BreakStmt* stmt) {
        auto breakNode = factory_.createNode(IASTNode::NodeKind::BREAK_STMT);
        setSourceLocation(breakNode, stmt->getBreakLoc());
        nodeMap_[stmt] = breakNode;
        return true;
    }

    bool VisitContinueStmt(clang::ContinueStmt* stmt) {
        auto continueNode = factory_.createNode(IASTNode::NodeKind::CONTINUE_STMT);
        setSourceLocation(continueNode, stmt->getContinueLoc());
        nodeMap_[stmt] = continueNode;
        return true;
    }

    bool VisitCompoundStmt(clang::CompoundStmt* stmt) {
        auto compoundNode = factory_.createNode(IASTNode::NodeKind::COMPOUND_STMT);
        setSourceLocation(compoundNode, stmt->getLBracLoc());
        nodeMap_[stmt] = compoundNode;
        return true;
    }

    bool VisitCallExpr(clang::CallExpr* expr) {
        auto callNode = factory_.createNode(IASTNode::NodeKind::CALL_EXPR);
        setSourceLocation(callNode, expr->getExprLoc());
        nodeMap_[expr] = callNode;
        return true;
    }

    bool VisitBinaryOperator(clang::BinaryOperator* expr) {
        auto binopNode = factory_.createNode(IASTNode::NodeKind::BINARY_OP);
        setSourceLocation(binopNode, expr->getExprLoc());
        nodeMap_[expr] = binopNode;
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator* expr) {
        auto unopNode = factory_.createNode(IASTNode::NodeKind::UNARY_OP);
        setSourceLocation(unopNode, expr->getExprLoc());
        nodeMap_[expr] = unopNode;
        return true;
    }

    ASTNodePtr getRootNode() const { return rootNode_; }

private:
    void setSourceLocation(ASTNodePtr node, clang::SourceLocation loc) {
        if (loc.isValid()) {
            auto& sm = context_.getSourceManager();
            auto presumedLoc = sm.getPresumedLoc(loc);
            if (presumedLoc.isValid()) {
                std::string location = std::string(presumedLoc.getFilename()) + ":" +
                                      std::to_string(presumedLoc.getLine()) + ":" +
                                      std::to_string(presumedLoc.getColumn());
                // Note: ASTNode doesn't have setSourceLocation, so we skip this for now
                // This would require extending the IASTNode interface
            }
        }
    }

    TypeInfoPtr convertType(clang::QualType qualType) {
        // Remove qualifiers for base type
        clang::QualType baseType = qualType.getUnqualifiedType();

        // Check if we've already converted this type
        auto it = typeMap_.find(baseType.getAsOpaquePtr());
        if (it != typeMap_.end()) {
            return it->second;
        }

        TypeInfoPtr nomicType;
        const clang::Type* type = baseType.getTypePtr();

        if (type->isVoidType()) {
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::VOID, "void");
        }
        else if (type->isBooleanType()) {
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::BOOL, "_Bool");
            nomicType->setSize(1);
            nomicType->setAlignment(1);
        }
        else if (type->isCharType()) {
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::CHAR, "char");
            nomicType->setSize(1);
            nomicType->setAlignment(1);
        }
        else if (type->isIntegerType()) {
            std::string typeName = qualType.getAsString();
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::INT, typeName);
            auto typeSize = context_.getTypeSize(qualType) / 8;  // bits to bytes
            auto typeAlign = context_.getTypeAlign(qualType) / 8;
            nomicType->setSize(typeSize);
            nomicType->setAlignment(typeAlign);
        }
        else if (type->isFloatingType()) {
            std::string typeName = qualType.getAsString();
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::FLOAT, typeName);
            auto typeSize = context_.getTypeSize(qualType) / 8;
            auto typeAlign = context_.getTypeAlign(qualType) / 8;
            nomicType->setSize(typeSize);
            nomicType->setAlignment(typeAlign);
        }
        else if (type->isPointerType()) {
            clang::QualType pointeeType = type->getPointeeType();
            auto pointeeNomicType = convertType(pointeeType);
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::POINTER, qualType.getAsString());
            nomicType->setSize(context_.getTypeSize(qualType) / 8);
            nomicType->setAlignment(context_.getTypeAlign(qualType) / 8);
            // Note: TypeInfo doesn't have setPointeeType, would need to extend interface
        }
        else if (type->isArrayType()) {
            const clang::ArrayType* arrayType = type->getAsArrayTypeUnsafe();
            clang::QualType elementType = arrayType->getElementType();
            auto elementNomicType = convertType(elementType);
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::ARRAY, qualType.getAsString());
            nomicType->setSize(context_.getTypeSize(qualType) / 8);
            nomicType->setAlignment(context_.getTypeAlign(qualType) / 8);
        }
        else if (type->isStructureType()) {
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::STRUCT, qualType.getAsString());
            nomicType->setSize(context_.getTypeSize(qualType) / 8);
            nomicType->setAlignment(context_.getTypeAlign(qualType) / 8);
        }
        else if (type->isUnionType()) {
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::UNION, qualType.getAsString());
            nomicType->setSize(context_.getTypeSize(qualType) / 8);
            nomicType->setAlignment(context_.getTypeAlign(qualType) / 8);
        }
        else if (type->isEnumeralType()) {
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::ENUM, qualType.getAsString());
            nomicType->setSize(context_.getTypeSize(qualType) / 8);
            nomicType->setAlignment(context_.getTypeAlign(qualType) / 8);
        }
        else if (type->isFunctionType()) {
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::FUNCTION, qualType.getAsString());
        }
        else {
            // Unknown type
            nomicType = std::make_shared<TypeInfo>(ITypeInfo::TypeKind::VOID, qualType.getAsString());
        }

        // Cache the type
        typeMap_[baseType.getAsOpaquePtr()] = nomicType;

        return nomicType;
    }

    clang::ASTContext& context_;
    ASTFactory& factory_;
    SemanticModel& model_;
    ASTNodePtr rootNode_;
    ASTNodePtr currentNode_;
    std::map<const void*, ASTNodePtr> nodeMap_;
    std::map<const void*, TypeInfoPtr> typeMap_;
};

/**
 * @brief AST Consumer that uses our visitor
 */
class ClangASTConsumer : public clang::ASTConsumer {
public:
    ClangASTConsumer(ASTFactory& factory, SemanticModel& model)
        : factory_(factory), model_(model), visitor_(nullptr) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        visitor_ = std::make_unique<ClangASTVisitor>(context, factory_, model_);
        visitor_->TraverseDecl(context.getTranslationUnitDecl());
    }

    ASTNodePtr getRootNode() const {
        return visitor_ ? visitor_->getRootNode() : nullptr;
    }

private:
    ASTFactory& factory_;
    SemanticModel& model_;
    std::unique_ptr<ClangASTVisitor> visitor_;
};

/**
 * @brief Frontend action that creates our consumer
 */
class ClangFrontendAction : public clang::ASTFrontendAction {
public:
    ClangFrontendAction(ASTFactory& factory, SemanticModel& model)
        : factory_(factory), model_(model) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& CI, llvm::StringRef file) override {
        consumer_ = std::make_unique<ClangASTConsumer>(factory_, model_);
        return std::unique_ptr<clang::ASTConsumer>(consumer_.get());
    }

    ASTNodePtr getRootNode() const {
        return consumer_ ? consumer_->getRootNode() : nullptr;
    }

private:
    ASTFactory& factory_;
    SemanticModel& model_;
    std::unique_ptr<ClangASTConsumer> consumer_;
};

/**
 * @brief Implementation class (PIMPL pattern)
 */
class ClangParser::Impl {
public:
    Impl() : factory_(std::make_unique<ASTFactory>()) {}

    ParseResult parseFile(const std::string& filePath, const ParseOptions& options) {
        ParseResult result;

        // Create semantic model
        auto semanticModel = factory_->createSemanticModel({});

        // Build compiler arguments
        std::vector<std::string> args;
        args.push_back("-fsyntax-only");
        args.push_back("-std=c11");

        // Add include paths
        for (const auto& includePath : options.includePaths) {
            args.push_back("-I" + includePath);
        }

        // Add defines
        for (const auto& define : options.defines) {
            args.push_back("-D" + define);
        }

        // Add custom flags
        for (const auto& flag : options.compilerFlags) {
            args.push_back(flag);
        }

        // Create action
        auto action = std::make_unique<ClangFrontendAction>(*factory_, *semanticModel);
        ClangFrontendAction* actionPtr = action.get();

        // Run the tool
        bool success = clang::tooling::runToolOnCodeWithArgs(
            std::move(action),
            readFile(filePath),
            args,
            filePath
        );

        if (success && actionPtr) {
            result.ast = actionPtr->getRootNode();
            result.semanticModel = semanticModel;
            result.success = true;
        } else {
            result.success = false;
            result.errors.push_back("Failed to parse file: " + filePath);
        }

        return result;
    }

    ParseResult parseString(const std::string& source, const std::string& filename, const ParseOptions& options) {
        ParseResult result;

        auto semanticModel = factory_->createSemanticModel({});

        std::vector<std::string> args;
        args.push_back("-fsyntax-only");
        args.push_back("-std=c11");

        for (const auto& includePath : options.includePaths) {
            args.push_back("-I" + includePath);
        }

        for (const auto& define : options.defines) {
            args.push_back("-D" + define);
        }

        for (const auto& flag : options.compilerFlags) {
            args.push_back(flag);
        }

        auto action = std::make_unique<ClangFrontendAction>(*factory_, *semanticModel);
        ClangFrontendAction* actionPtr = action.get();

        bool success = clang::tooling::runToolOnCodeWithArgs(
            std::move(action),
            source,
            args,
            filename
        );

        if (success && actionPtr) {
            result.ast = actionPtr->getRootNode();
            result.semanticModel = semanticModel;
            result.success = true;
        } else {
            result.success = false;
            result.errors.push_back("Failed to parse source");
        }

        return result;
    }

private:
    std::string readFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file) {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::unique_ptr<ASTFactory> factory_;
};

// ClangParser implementation

ClangParser::ClangParser() : pImpl(std::make_unique<Impl>()) {}

ClangParser::~ClangParser() = default;

ClangParser::ParseResult ClangParser::parseFile(const std::string& filePath, const ParseOptions& options) {
    return pImpl->parseFile(filePath, options);
}

ClangParser::ParseResult ClangParser::parseFiles(const std::vector<std::string>& filePaths, const ParseOptions& options) {
    ParseResult combinedResult;
    combinedResult.success = true;

    // For now, just parse first file
    // TODO: Properly combine multiple translation units
    if (!filePaths.empty()) {
        combinedResult = parseFile(filePaths[0], options);
    }

    return combinedResult;
}

ClangParser::ParseResult ClangParser::parseString(const std::string& source, const std::string& filename, const ParseOptions& options) {
    return pImpl->parseString(source, filename, options);
}

} // namespace core
} // namespace nomic
