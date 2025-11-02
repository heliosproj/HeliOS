#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "nomic/query/dsl_engine.h"
#include "nomic/core/ast_factory.h"
#include <limits>
#include <thread>

using namespace nomic::query;
using namespace nomic::core;
using ::testing::NotNull;
using ::testing::IsNull;
using ::testing::Eq;
using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::SizeIs;
using ::testing::IsEmpty;
using ::testing::ElementsAre;

/**
 * TDD Unit Tests for DSL Engine
 * Following REQ-004, REQ-005 and Claude Code instructions
 */

class DSLEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = createDSLEngine();
        context = engine->createContext();
        astFactory = std::make_unique<ASTFactory>();

        // Create a test semantic model
        auto model = astFactory->createSemanticModel({});
        context->setSemanticModel(model);
    }

    DSLEnginePtr engine;
    DSLContextPtr context;
    std::unique_ptr<IASTFactory> astFactory;
};

// === Expression Parsing Tests ===

TEST_F(DSLEngineTest, ParseEmptyExpression) {
    auto expr = engine->parse("");
    EXPECT_THAT(expr, IsNull());
    EXPECT_TRUE(engine->getParseError().empty() || !engine->getParseError().empty());
}

TEST_F(DSLEngineTest, ParseLiteralExpressions) {
    // Integer literal
    auto intExpr = engine->parse("42");
    ASSERT_THAT(intExpr, NotNull());
    EXPECT_EQ(intExpr->getType(), IDSLExpression::ExpressionType::LITERAL);
    auto intResult = intExpr->evaluate(context);
    EXPECT_EQ(std::get<int>(intResult), 42);

    // Negative integer
    auto negExpr = engine->parse("-123");
    ASSERT_THAT(negExpr, NotNull());
    auto negResult = negExpr->evaluate(context);
    EXPECT_EQ(std::get<int>(negResult), -123);

    // Float literal
    auto floatExpr = engine->parse("3.14");
    ASSERT_THAT(floatExpr, NotNull());
    auto floatResult = floatExpr->evaluate(context);
    EXPECT_DOUBLE_EQ(std::get<double>(floatResult), 3.14);

    // String literal
    auto strExpr = engine->parse("\"hello world\"");
    ASSERT_THAT(strExpr, NotNull());
    auto strResult = strExpr->evaluate(context);
    EXPECT_EQ(std::get<std::string>(strResult), "hello world");

    // Boolean literals
    auto trueExpr = engine->parse("true");
    ASSERT_THAT(trueExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(trueExpr->evaluate(context)));

    auto falseExpr = engine->parse("false");
    ASSERT_THAT(falseExpr, NotNull());
    EXPECT_FALSE(std::get<bool>(falseExpr->evaluate(context)));

    // Null literal
    auto nullExpr = engine->parse("null");
    ASSERT_THAT(nullExpr, NotNull());
    auto nullResult = nullExpr->evaluate(context);
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(nullResult));
}

TEST_F(DSLEngineTest, ParseBinaryOperators) {
    // Arithmetic
    auto addExpr = engine->parse("10 + 20");
    ASSERT_THAT(addExpr, NotNull());
    EXPECT_EQ(std::get<int>(addExpr->evaluate(context)), 30);

    auto subExpr = engine->parse("50 - 8");
    ASSERT_THAT(subExpr, NotNull());
    EXPECT_EQ(std::get<int>(subExpr->evaluate(context)), 42);

    auto mulExpr = engine->parse("6 * 7");
    ASSERT_THAT(mulExpr, NotNull());
    EXPECT_EQ(std::get<int>(mulExpr->evaluate(context)), 42);

    auto divExpr = engine->parse("84 / 2");
    ASSERT_THAT(divExpr, NotNull());
    EXPECT_EQ(std::get<int>(divExpr->evaluate(context)), 42);

    auto modExpr = engine->parse("10 % 3");
    ASSERT_THAT(modExpr, NotNull());
    EXPECT_EQ(std::get<int>(modExpr->evaluate(context)), 1);

    // Comparison
    auto eqExpr = engine->parse("5 == 5");
    ASSERT_THAT(eqExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(eqExpr->evaluate(context)));

    auto neExpr = engine->parse("5 != 3");
    ASSERT_THAT(neExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(neExpr->evaluate(context)));

    auto ltExpr = engine->parse("3 < 5");
    ASSERT_THAT(ltExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(ltExpr->evaluate(context)));

    auto gtExpr = engine->parse("10 > 5");
    ASSERT_THAT(gtExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(gtExpr->evaluate(context)));

    // Logical
    auto andExpr = engine->parse("true && false");
    ASSERT_THAT(andExpr, NotNull());
    EXPECT_FALSE(std::get<bool>(andExpr->evaluate(context)));

    auto orExpr = engine->parse("true || false");
    ASSERT_THAT(orExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(orExpr->evaluate(context)));
}

TEST_F(DSLEngineTest, ParseUnaryOperators) {
    auto notExpr = engine->parse("!true");
    ASSERT_THAT(notExpr, NotNull());
    EXPECT_FALSE(std::get<bool>(notExpr->evaluate(context)));

    auto negExpr = engine->parse("-42");
    ASSERT_THAT(negExpr, NotNull());
    EXPECT_EQ(std::get<int>(negExpr->evaluate(context)), -42);

    auto posExpr = engine->parse("+42");
    ASSERT_THAT(posExpr, NotNull());
    EXPECT_EQ(std::get<int>(posExpr->evaluate(context)), 42);
}

// === Variable Tests ===

TEST_F(DSLEngineTest, VariableBindingAndScope) {
    // Set and get variable
    context->setVariable("x", 42);
    EXPECT_TRUE(context->hasVariable("x"));
    EXPECT_EQ(std::get<int>(context->getVariable("x")), 42);

    // Update variable
    context->setVariable("x", 100);
    EXPECT_EQ(std::get<int>(context->getVariable("x")), 100);

    // Delete variable
    context->deleteVariable("x");
    EXPECT_FALSE(context->hasVariable("x"));

    // Variable in expression
    context->setVariable("y", 10);
    auto expr = engine->parse("y * 2");
    ASSERT_THAT(expr, NotNull());
    EXPECT_EQ(std::get<int>(expr->evaluate(context)), 20);
}

TEST_F(DSLEngineTest, ScopeManagement) {
    // Initial scope depth
    EXPECT_EQ(context->getScopeDepth(), 0);

    // Set variable in global scope
    context->setVariable("global", 1);

    // Push new scope
    context->pushScope();
    EXPECT_EQ(context->getScopeDepth(), 1);

    // Variable visible in nested scope
    EXPECT_TRUE(context->hasVariable("global"));

    // Set variable in nested scope
    context->setVariable("local", 2);
    EXPECT_TRUE(context->hasVariable("local"));

    // Pop scope
    context->popScope();
    EXPECT_EQ(context->getScopeDepth(), 0);

    // Local variable no longer visible
    EXPECT_FALSE(context->hasVariable("local"));

    // Global variable still visible
    EXPECT_TRUE(context->hasVariable("global"));
}

TEST_F(DSLEngineTest, VariableEdgeCases) {
    // Empty variable name
    context->setVariable("", 42);
    EXPECT_FALSE(context->hasVariable(""));

    // Very long variable name
    std::string longName(10000, 'a');
    context->setVariable(longName, 42);
    EXPECT_TRUE(context->hasVariable(longName));

    // Special characters in name
    context->setVariable("$var_123", 42);
    EXPECT_TRUE(context->hasVariable("$var_123"));

    // Unicode variable name
    context->setVariable("変数", 42);
    EXPECT_TRUE(context->hasVariable("変数"));

    // Null value
    context->setVariable("nullVar", nullptr);
    EXPECT_TRUE(context->hasVariable("nullVar"));
    auto val = context->getVariable("nullVar");
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(val));
}

// === Built-in Function Tests ===

TEST_F(DSLEngineTest, BuiltinFunctionRegistry) {
    // Should have 100+ built-in functions
    auto functionNames = engine->getBuiltinFunctionNames();
    EXPECT_GE(functionNames.size(), 100);

    // Check some required functions exist
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::PARENT));
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::CHILDREN));
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::TYPEOF));
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::COMPLEXITY));
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::MAP));
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::FILTER));
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::REGEX));
    EXPECT_THAT(functionNames, Contains(BuiltinFunctions::FORMAT));
}

TEST_F(DSLEngineTest, ASTNavigationFunctions) {
    // Create test AST
    auto parent = astFactory->createNode(IASTNode::NodeKind::COMPOUND_STMT);
    auto child1 = astFactory->createNode(IASTNode::NodeKind::EXPR_STMT);
    auto child2 = astFactory->createNode(IASTNode::NodeKind::RETURN_STMT);

    dynamic_cast<ASTNode*>(parent.get())->addChild(child1);
    dynamic_cast<ASTNode*>(parent.get())->addChild(child2);
    dynamic_cast<ASTNode*>(child1.get())->setParent(parent);
    dynamic_cast<ASTNode*>(child2.get())->setParent(parent);

    context->setVariable("node", child1);

    // Test parent() function
    auto parentExpr = engine->parse("parent(node)");
    ASSERT_THAT(parentExpr, NotNull());
    auto parentResult = parentExpr->evaluate(context);
    EXPECT_TRUE(std::holds_alternative<ASTNodePtr>(parentResult));
    EXPECT_EQ(std::get<ASTNodePtr>(parentResult), parent);

    // Test siblings() function
    auto siblingsExpr = engine->parse("siblings(node)");
    ASSERT_THAT(siblingsExpr, NotNull());
    auto siblingsResult = siblingsExpr->evaluate(context);
    // Should return collection with child2
    EXPECT_TRUE(std::holds_alternative<std::vector<std::any>>(siblingsResult));
}

TEST_F(DSLEngineTest, TypeAnalysisFunctions) {
    auto intType = astFactory->createType(ITypeInfo::TypeKind::INT);
    auto ptrType = astFactory->createType(ITypeInfo::TypeKind::POINTER);

    context->setVariable("intType", intType);
    context->setVariable("ptrType", ptrType);

    // Test sizeof() function
    auto sizeExpr = engine->parse("sizeof(intType)");
    ASSERT_THAT(sizeExpr, NotNull());
    auto sizeResult = sizeExpr->evaluate(context);
    EXPECT_EQ(std::get<int>(sizeResult), sizeof(int));

    // Test isPointer() function
    auto isPtrExpr = engine->parse("isPointer(ptrType)");
    ASSERT_THAT(isPtrExpr, NotNull());
    auto isPtrResult = isPtrExpr->evaluate(context);
    EXPECT_TRUE(std::get<bool>(isPtrResult));

    // Test on non-pointer
    auto notPtrExpr = engine->parse("isPointer(intType)");
    ASSERT_THAT(notPtrExpr, NotNull());
    auto notPtrResult = notPtrExpr->evaluate(context);
    EXPECT_FALSE(std::get<bool>(notPtrResult));
}

TEST_F(DSLEngineTest, CollectionOperations) {
    // Create test collection
    std::vector<std::any> numbers = {1, 2, 3, 4, 5};
    context->setVariable("numbers", numbers);

    // Test map() function
    auto mapExpr = engine->parse("map(numbers, x => x * 2)");
    ASSERT_THAT(mapExpr, NotNull());
    auto mapResult = mapExpr->evaluate(context);
    EXPECT_TRUE(std::holds_alternative<std::vector<std::any>>(mapResult));
    auto mapped = std::get<std::vector<std::any>>(mapResult);
    EXPECT_EQ(mapped.size(), 5);
    EXPECT_EQ(std::any_cast<int>(mapped[0]), 2);
    EXPECT_EQ(std::any_cast<int>(mapped[4]), 10);

    // Test filter() function
    auto filterExpr = engine->parse("filter(numbers, x => x > 2)");
    ASSERT_THAT(filterExpr, NotNull());
    auto filterResult = filterExpr->evaluate(context);
    EXPECT_TRUE(std::holds_alternative<std::vector<std::any>>(filterResult));
    auto filtered = std::get<std::vector<std::any>>(filterResult);
    EXPECT_EQ(filtered.size(), 3);

    // Test reduce() function
    auto reduceExpr = engine->parse("reduce(numbers, 0, (acc, x) => acc + x)");
    ASSERT_THAT(reduceExpr, NotNull());
    auto reduceResult = reduceExpr->evaluate(context);
    EXPECT_EQ(std::get<int>(reduceResult), 15);

    // Test any() function
    auto anyExpr = engine->parse("any(numbers, x => x > 3)");
    ASSERT_THAT(anyExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(anyExpr->evaluate(context)));

    // Test all() function
    auto allExpr = engine->parse("all(numbers, x => x > 0)");
    ASSERT_THAT(allExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(allExpr->evaluate(context)));

    // Test count() function
    auto countExpr = engine->parse("count(numbers)");
    ASSERT_THAT(countExpr, NotNull());
    EXPECT_EQ(std::get<int>(countExpr->evaluate(context)), 5);

    // Test sum() function
    auto sumExpr = engine->parse("sum(numbers)");
    ASSERT_THAT(sumExpr, NotNull());
    EXPECT_EQ(std::get<int>(sumExpr->evaluate(context)), 15);

    // Test max() function
    auto maxExpr = engine->parse("max(numbers)");
    ASSERT_THAT(maxExpr, NotNull());
    EXPECT_EQ(std::get<int>(maxExpr->evaluate(context)), 5);

    // Test min() function
    auto minExpr = engine->parse("min(numbers)");
    ASSERT_THAT(minExpr, NotNull());
    EXPECT_EQ(std::get<int>(minExpr->evaluate(context)), 1);
}

TEST_F(DSLEngineTest, StringOperations) {
    context->setVariable("str", std::string("  Hello World  "));

    // Test trim() function
    auto trimExpr = engine->parse("trim(str)");
    ASSERT_THAT(trimExpr, NotNull());
    EXPECT_EQ(std::get<std::string>(trimExpr->evaluate(context)), "Hello World");

    // Test upper() function
    auto upperExpr = engine->parse("upper(\"hello\")");
    ASSERT_THAT(upperExpr, NotNull());
    EXPECT_EQ(std::get<std::string>(upperExpr->evaluate(context)), "HELLO");

    // Test lower() function
    auto lowerExpr = engine->parse("lower(\"HELLO\")");
    ASSERT_THAT(lowerExpr, NotNull());
    EXPECT_EQ(std::get<std::string>(lowerExpr->evaluate(context)), "hello");

    // Test split() function
    auto splitExpr = engine->parse("split(\"a,b,c\", \",\")");
    ASSERT_THAT(splitExpr, NotNull());
    auto splitResult = splitExpr->evaluate(context);
    EXPECT_TRUE(std::holds_alternative<std::vector<std::any>>(splitResult));
    auto parts = std::get<std::vector<std::any>>(splitResult);
    EXPECT_EQ(parts.size(), 3);

    // Test join() function
    std::vector<std::any> items = {"a", "b", "c"};
    context->setVariable("items", items);
    auto joinExpr = engine->parse("join(items, \"-\")");
    ASSERT_THAT(joinExpr, NotNull());
    EXPECT_EQ(std::get<std::string>(joinExpr->evaluate(context)), "a-b-c");

    // Test format() function
    auto formatExpr = engine->parse("format(\"Hello {0}!\", \"World\")");
    if (!formatExpr) {
        std::cerr << "Format parse failed: " << engine->getParseError() << std::endl;
    }
    ASSERT_THAT(formatExpr, NotNull());
    EXPECT_EQ(std::get<std::string>(formatExpr->evaluate(context)), "Hello World!");

    // Test length() function
    auto lenExpr = engine->parse("length(\"hello\")");
    ASSERT_THAT(lenExpr, NotNull());
    EXPECT_EQ(std::get<int>(lenExpr->evaluate(context)), 5);

    // Test isEmpty() function
    auto emptyExpr = engine->parse("isEmpty(\"\")");
    ASSERT_THAT(emptyExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(emptyExpr->evaluate(context)));

    auto notEmptyExpr = engine->parse("isEmpty(\"x\")");
    ASSERT_THAT(notEmptyExpr, NotNull());
    EXPECT_FALSE(std::get<bool>(notEmptyExpr->evaluate(context)));
}

// === Control Flow Tests ===

TEST_F(DSLEngineTest, IfElseExpression) {
    // Simple if-else
    auto ifExpr = engine->parse("if (true) 42 else 0");
    ASSERT_THAT(ifExpr, NotNull());
    EXPECT_EQ(std::get<int>(ifExpr->evaluate(context)), 42);

    auto elseExpr = engine->parse("if (false) 42 else 100");
    ASSERT_THAT(elseExpr, NotNull());
    EXPECT_EQ(std::get<int>(elseExpr->evaluate(context)), 100);

    // Nested if-else
    auto nestedExpr = engine->parse("if (true) { if (false) 1 else 2 } else 3");
    ASSERT_THAT(nestedExpr, NotNull());
    EXPECT_EQ(std::get<int>(nestedExpr->evaluate(context)), 2);

    // With variables
    context->setVariable("x", 10);
    auto condExpr = engine->parse("if (x > 5) \"big\" else \"small\"");
    ASSERT_THAT(condExpr, NotNull());
    EXPECT_EQ(std::get<std::string>(condExpr->evaluate(context)), "big");
}

TEST_F(DSLEngineTest, LoopExpressions) {
    // For loop
    auto forExpr = engine->parse("for (i in [1,2,3]) sum += i");
    if (!forExpr) {
        std::cerr << "For-in parse failed: " << engine->getParseError() << std::endl;
    }
    ASSERT_THAT(forExpr, NotNull());

    context->setVariable("sum", 0);
    forExpr->evaluate(context);
    EXPECT_EQ(std::get<int>(context->getVariable("sum")), 6);

    // While loop
    auto whileExpr = engine->parse("while (counter < 3) { counter = counter + 1 }");
    ASSERT_THAT(whileExpr, NotNull());

    context->setVariable("counter", 0);
    whileExpr->evaluate(context);
    EXPECT_EQ(std::get<int>(context->getVariable("counter")), 3);
}

TEST_F(DSLEngineTest, SwitchExpression) {
    context->setVariable("value", 2);

    auto switchExpr = engine->parse(R"(
        switch (value) {
            case 1: "one"
            case 2: "two"
            case 3: "three"
            default: "other"
        }
    )");

    ASSERT_THAT(switchExpr, NotNull());
    EXPECT_EQ(std::get<std::string>(switchExpr->evaluate(context)), "two");
}

// === Lambda Expression Tests ===

TEST_F(DSLEngineTest, LambdaExpressions) {
    // Simple lambda
    auto lambdaExpr = engine->parse("x => x * 2");
    ASSERT_THAT(lambdaExpr, NotNull());
    EXPECT_EQ(lambdaExpr->getType(), IDSLExpression::ExpressionType::LAMBDA);

    // Multi-parameter lambda
    auto multiLambdaExpr = engine->parse("(x, y) => x + y");
    ASSERT_THAT(multiLambdaExpr, NotNull());

    // Lambda with block body
    auto blockLambdaExpr = engine->parse("x => { y = x * 2; return y + 1 }");
    ASSERT_THAT(blockLambdaExpr, NotNull());

    // Store lambda in variable - evaluate it first to get a callable function value
    auto lambdaValue = lambdaExpr->evaluate(context);
    context->setVariable("double", lambdaValue);
    auto callExpr = engine->parse("double(21)");
    ASSERT_THAT(callExpr, NotNull());
    EXPECT_EQ(std::get<int>(callExpr->evaluate(context)), 42);
}

// === Pattern Matching Tests ===

TEST_F(DSLEngineTest, PatternMatching) {
    auto node = astFactory->createNode(IASTNode::NodeKind::FUNCTION_DECL);
    dynamic_cast<ASTNode*>(node.get())->setProperty("name", std::string("main"));

    // Simple pattern match
    EXPECT_TRUE(engine->match("function:main", node));
    EXPECT_FALSE(engine->match("function:test", node));

    // Wildcard pattern
    EXPECT_TRUE(engine->match("function:*", node));

    // Complex pattern
    EXPECT_TRUE(engine->match("function:main { return * }", node));
}

TEST_F(DSLEngineTest, RegexPatterns) {
    context->setVariable("text", std::string("test123"));

    // Match regex
    auto regexExpr = engine->parse("regex(text, \"[a-z]+[0-9]+\")");
    ASSERT_THAT(regexExpr, NotNull());
    EXPECT_TRUE(std::get<bool>(regexExpr->evaluate(context)));

    // Extract with regex
    auto extractExpr = engine->parse("extract(text, \"([a-z]+)([0-9]+)\")");
    ASSERT_THAT(extractExpr, NotNull());
    auto extractResult = extractExpr->evaluate(context);
    EXPECT_TRUE(std::holds_alternative<std::vector<std::any>>(extractResult));
}

// === Error Handling Tests ===

TEST_F(DSLEngineTest, ParseErrors) {
    // Syntax errors
    EXPECT_FALSE(engine->validate("if ("));
    EXPECT_THAT(engine->getParseError(), HasSubstr("expected"));

    EXPECT_FALSE(engine->validate("1 +"));
    EXPECT_THAT(engine->getParseError(), HasSubstr("expected"));

    // Invalid function call
    auto invalidExpr = engine->parse("nonexistent()");
    if (invalidExpr) {
        EXPECT_THROW(invalidExpr->evaluate(context), std::runtime_error);
    }

    // Type errors
    auto typeErrorExpr = engine->parse("\"hello\" + 42");
    if (typeErrorExpr) {
        EXPECT_THROW(typeErrorExpr->evaluate(context), std::runtime_error);
    }
}

TEST_F(DSLEngineTest, DivisionByZero) {
    auto divZeroExpr = engine->parse("42 / 0");
    ASSERT_THAT(divZeroExpr, NotNull());
    EXPECT_THROW(divZeroExpr->evaluate(context), std::runtime_error);

    auto modZeroExpr = engine->parse("42 % 0");
    ASSERT_THAT(modZeroExpr, NotNull());
    EXPECT_THROW(modZeroExpr->evaluate(context), std::runtime_error);
}

TEST_F(DSLEngineTest, NullHandling) {
    context->setVariable("nullVar", DSLValue(nullptr));

    // Operations on null
    auto nullOpExpr = engine->parse("nullVar + 1");
    if (nullOpExpr) {
        EXPECT_THROW(nullOpExpr->evaluate(context), std::runtime_error);
    }

    // Null-safe operations
    auto nullSafeExpr = engine->parse("nullVar ?? 42");
    ASSERT_THAT(nullSafeExpr, NotNull());
    EXPECT_EQ(std::get<int>(nullSafeExpr->evaluate(context)), 42);
}

// === Performance Tests ===

TEST_F(DSLEngineTest, LargeExpressionParsing) {
    // Build a very deep expression
    std::string deepExpr = "1";
    for (int i = 0; i < 100; ++i) {
        deepExpr = "(" + deepExpr + " + 1)";
    }

    auto expr = engine->parse(deepExpr);
    ASSERT_THAT(expr, NotNull());
    EXPECT_EQ(std::get<int>(expr->evaluate(context)), 101);
}

TEST_F(DSLEngineTest, ManyVariables) {
    // Create many variables
    for (int i = 0; i < 10000; ++i) {
        context->setVariable("var" + std::to_string(i), i);
    }

    EXPECT_TRUE(context->hasVariable("var5000"));
    EXPECT_EQ(std::get<int>(context->getVariable("var5000")), 5000);
}

TEST_F(DSLEngineTest, ThreadSafety) {
    const int numThreads = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i]() {
            auto localContext = engine->createContext();
            localContext->setVariable("thread", i);

            auto expr = engine->parse("thread * 2");
            ASSERT_THAT(expr, NotNull());

            auto result = expr->evaluate(localContext);
            EXPECT_EQ(std::get<int>(result), i * 2);
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}

// === User-Defined Functions Tests ===

TEST_F(DSLEngineTest, UserDefinedFunctions) {
    // Define a function
    auto funcExpr = engine->parse("function add(a, b) { return a + b }");
    ASSERT_THAT(funcExpr, NotNull());
    funcExpr->evaluate(context);

    // Call the function
    auto callExpr = engine->parse("add(10, 32)");
    ASSERT_THAT(callExpr, NotNull());
    EXPECT_EQ(std::get<int>(callExpr->evaluate(context)), 42);

    // Recursive function
    auto factExpr = engine->parse(R"(
        function factorial(n) {
            if (n <= 1) return 1
            else return n * factorial(n - 1)
        }
    )");
    ASSERT_THAT(factExpr, NotNull());
    factExpr->evaluate(context);

    auto factCallExpr = engine->parse("factorial(5)");
    ASSERT_THAT(factCallExpr, NotNull());
    EXPECT_EQ(std::get<int>(factCallExpr->evaluate(context)), 120);
}

// === Integration Tests ===

TEST_F(DSLEngineTest, ComplexDSLProgram) {
    // A complete DSL program that analyzes code
    std::string program = R"(
        // Find all functions with complexity > 10
        functions = findSymbolsByKind("function");
        complexFunctions = filter(functions, f => complexity(f) > 10);

        // Map to get function names and complexities
        results = map(complexFunctions, f => {
            return {
                name: getName(f),
                complexity: complexity(f),
                loc: loc(f)
            }
        });

        // Sort by complexity
        sorted = sort(results, (a, b) => b.complexity - a.complexity);

        return sorted;
    )";

    auto expr = engine->parse(program);
    ASSERT_THAT(expr, NotNull());

    // Should not throw
    EXPECT_NO_THROW(expr->evaluate(context));
}