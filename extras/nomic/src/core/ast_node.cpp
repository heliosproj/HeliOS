#include "nomic/core/ast_node.h"

namespace nomic {
namespace core {

ASTNode::ASTNode(NodeKind kind)
    : kind_(kind),
      kindName_(getKindNameForKind(kind)),
      lineColumn_({0, 0}) {
}

void ASTNode::accept(IVisitor& visitor) {
    visitor.visitNode(*this);

    // Dispatch to specific visit methods based on kind
    switch (kind_) {
        case NodeKind::FUNCTION_DECL:
            visitor.visitFunctionDecl(*this);
            break;
        case NodeKind::VAR_DECL:
        case NodeKind::PARAM_DECL:
        case NodeKind::FIELD_DECL:
            visitor.visitVarDecl(*this);
            break;
        case NodeKind::STATEMENT:
        case NodeKind::COMPOUND_STMT:
        case NodeKind::IF_STMT:
        case NodeKind::WHILE_STMT:
        case NodeKind::FOR_STMT:
        case NodeKind::DO_STMT:
        case NodeKind::SWITCH_STMT:
        case NodeKind::CASE_STMT:
        case NodeKind::RETURN_STMT:
        case NodeKind::BREAK_STMT:
        case NodeKind::CONTINUE_STMT:
        case NodeKind::GOTO_STMT:
        case NodeKind::LABEL_STMT:
        case NodeKind::DECL_STMT:
        case NodeKind::EXPR_STMT:
            visitor.visitStatement(*this);
            break;
        case NodeKind::EXPRESSION:
        case NodeKind::LITERAL:
        case NodeKind::IDENTIFIER:
        case NodeKind::OPERATOR:
        case NodeKind::CALL_EXPR:
        case NodeKind::BINARY_OP:
        case NodeKind::UNARY_OP:
        case NodeKind::CAST_EXPR:
        case NodeKind::ARRAY_SUBSCRIPT:
        case NodeKind::MEMBER_EXPR:
            visitor.visitExpression(*this);
            break;
        default:
            // Just use the generic visitNode for unknown types
            break;
    }

    // Traverse children if visitor wants to
    if (visitor.shouldTraverseChildren()) {
        for (auto& child : children_) {
            if (child) {
                child->accept(visitor);
            }
        }
    }
}

std::string ASTNode::getKindNameForKind(NodeKind kind) const {
    switch (kind) {
        case NodeKind::UNKNOWN: return "Unknown";
        case NodeKind::TRANSLATION_UNIT: return "TranslationUnit";
        case NodeKind::FUNCTION_DECL: return "FunctionDecl";
        case NodeKind::VAR_DECL: return "VarDecl";
        case NodeKind::PARAM_DECL: return "ParamDecl";
        case NodeKind::FIELD_DECL: return "FieldDecl";
        case NodeKind::TYPE_DEF: return "TypeDef";
        case NodeKind::STRUCT_DECL: return "StructDecl";
        case NodeKind::UNION_DECL: return "UnionDecl";
        case NodeKind::ENUM_DECL: return "EnumDecl";
        case NodeKind::STATEMENT: return "Statement";
        case NodeKind::EXPRESSION: return "Expression";
        case NodeKind::LITERAL: return "Literal";
        case NodeKind::IDENTIFIER: return "Identifier";
        case NodeKind::OPERATOR: return "Operator";
        case NodeKind::CALL_EXPR: return "CallExpr";
        case NodeKind::BINARY_OP: return "BinaryOp";
        case NodeKind::UNARY_OP: return "UnaryOp";
        case NodeKind::CAST_EXPR: return "CastExpr";
        case NodeKind::ARRAY_SUBSCRIPT: return "ArraySubscript";
        case NodeKind::MEMBER_EXPR: return "MemberExpr";
        case NodeKind::COMPOUND_STMT: return "CompoundStmt";
        case NodeKind::IF_STMT: return "IfStmt";
        case NodeKind::WHILE_STMT: return "WhileStmt";
        case NodeKind::FOR_STMT: return "ForStmt";
        case NodeKind::DO_STMT: return "DoStmt";
        case NodeKind::SWITCH_STMT: return "SwitchStmt";
        case NodeKind::CASE_STMT: return "CaseStmt";
        case NodeKind::RETURN_STMT: return "ReturnStmt";
        case NodeKind::BREAK_STMT: return "BreakStmt";
        case NodeKind::CONTINUE_STMT: return "ContinueStmt";
        case NodeKind::GOTO_STMT: return "GotoStmt";
        case NodeKind::LABEL_STMT: return "LabelStmt";
        case NodeKind::DECL_STMT: return "DeclStmt";
        case NodeKind::EXPR_STMT: return "ExprStmt";
        default: return "Unknown";
    }
}

} // namespace core
} // namespace nomic