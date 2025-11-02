#include <gtest/gtest.h>
#include "nomic/core/clang_parser.h"
#include <memory>

using namespace nomic::core;

/**
 * @file test_clang_parser.cpp
 * @brief Unit tests for Clang/LLVM integration
 */

class ClangParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<ClangParser>();
    }

    std::unique_ptr<ClangParser> parser;
};

// === Basic Parsing Tests ===

TEST_F(ClangParserTest, ParseEmptyFile) {
    std::string source = "";
    auto result = parser->parseString(source, "empty.c");

    EXPECT_TRUE(result.success);
    EXPECT_NE(result.ast, nullptr);
    EXPECT_NE(result.semanticModel, nullptr);
}

TEST_F(ClangParserTest, ParseSimpleFunction) {
    std::string source = R"(
        void testFunction() {
            return;
        }
    )";

    auto result = parser->parseString(source, "test.c");

    EXPECT_TRUE(result.success);
    EXPECT_NE(result.ast, nullptr);
    EXPECT_NE(result.semanticModel, nullptr);

    // Should have at least one symbol (the function)
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 1);
}

TEST_F(ClangParserTest, ParseFunctionWithParameters) {
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }
    )";

    auto result = parser->parseString(source, "add.c");

    EXPECT_TRUE(result.success);
    EXPECT_NE(result.ast, nullptr);

    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 1); // At least the function symbol
}

TEST_F(ClangParserTest, ParseVariableDeclarations) {
    std::string source = R"(
        int globalVar = 42;
        static int staticVar = 100;

        void func() {
            int localVar = 10;
        }
    )";

    auto result = parser->parseString(source, "vars.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 3); // globalVar, staticVar, func (localVar might not be at top level)
}

// === Type System Tests ===

TEST_F(ClangParserTest, ParseBasicTypes) {
    std::string source = R"(
        void voidFunc(void);
        _Bool boolVar;
        char charVar;
        int intVar;
        long longVar;
        float floatVar;
        double doubleVar;
    )";

    auto result = parser->parseString(source, "types.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 6); // All the variables
}

TEST_F(ClangParserTest, ParsePointerTypes) {
    std::string source = R"(
        int* intPtr;
        char* charPtr;
        void* voidPtr;
        int** doublePtr;
    )";

    auto result = parser->parseString(source, "pointers.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 4);

    // Check that at least one symbol has a pointer type
    bool foundPointer = false;
    for (const auto& sym : symbols) {
        if (sym->getType() && sym->getType()->getKind() == ITypeInfo::TypeKind::POINTER) {
            foundPointer = true;
            break;
        }
    }
    EXPECT_TRUE(foundPointer);
}

TEST_F(ClangParserTest, ParseArrayTypes) {
    std::string source = R"(
        int array[10];
        char buffer[256];
        int matrix[3][3];
    )";

    auto result = parser->parseString(source, "arrays.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 3);
}

TEST_F(ClangParserTest, ParseStructType) {
    std::string source = R"(
        struct Point {
            int x;
            int y;
        };

        struct Point p;
    )";

    auto result = parser->parseString(source, "struct.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 3); // Point, x, y fields, and p variable
}

TEST_F(ClangParserTest, ParseUnionType) {
    std::string source = R"(
        union Data {
            int i;
            float f;
            char c;
        };

        union Data d;
    )";

    auto result = parser->parseString(source, "union.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 4); // Data, fields, and d variable
}

TEST_F(ClangParserTest, ParseEnumType) {
    std::string source = R"(
        enum Color {
            RED,
            GREEN,
            BLUE
        };

        enum Color c;
    )";

    auto result = parser->parseString(source, "enum.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 1); // At least the Color enum
}

TEST_F(ClangParserTest, ParseTypedef) {
    std::string source = R"(
        typedef int MyInt;
        typedef struct {
            int x;
            int y;
        } Point;

        MyInt value;
        Point pt;
    )";

    auto result = parser->parseString(source, "typedef.c");

    EXPECT_TRUE(result.success);
    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 4); // MyInt, Point, value, pt
}

// === Statement Tests ===

TEST_F(ClangParserTest, ParseIfStatement) {
    std::string source = R"(
        void test(int x) {
            if (x > 0) {
                x = 1;
            } else {
                x = -1;
            }
        }
    )";

    auto result = parser->parseString(source, "if.c");
    EXPECT_TRUE(result.success);
}

TEST_F(ClangParserTest, ParseLoops) {
    std::string source = R"(
        void loops(void) {
            int i;

            while (i < 10) {
                i++;
            }

            for (i = 0; i < 10; i++) {
                // body
            }

            do {
                i--;
            } while (i > 0);
        }
    )";

    auto result = parser->parseString(source, "loops.c");
    EXPECT_TRUE(result.success);
}

TEST_F(ClangParserTest, ParseSwitchStatement) {
    std::string source = R"(
        void test(int x) {
            switch (x) {
                case 1:
                    break;
                case 2:
                    break;
                default:
                    break;
            }
        }
    )";

    auto result = parser->parseString(source, "switch.c");
    EXPECT_TRUE(result.success);
}

TEST_F(ClangParserTest, ParseReturnStatement) {
    std::string source = R"(
        int getValue(void) {
            return 42;
        }
    )";

    auto result = parser->parseString(source, "return.c");
    EXPECT_TRUE(result.success);
}

// === Expression Tests ===

TEST_F(ClangParserTest, ParseBinaryOperators) {
    std::string source = R"(
        int ops(int a, int b) {
            int sum = a + b;
            int diff = a - b;
            int prod = a * b;
            int quot = a / b;
            int rem = a % b;
            return sum;
        }
    )";

    auto result = parser->parseString(source, "binops.c");
    EXPECT_TRUE(result.success);
}

TEST_F(ClangParserTest, ParseUnaryOperators) {
    std::string source = R"(
        void unary(int x) {
            int neg = -x;
            int comp = ~x;
            int not = !x;
            x++;
            ++x;
            x--;
            --x;
        }
    )";

    auto result = parser->parseString(source, "unary.c");
    EXPECT_TRUE(result.success);
}

TEST_F(ClangParserTest, ParseFunctionCall) {
    std::string source = R"(
        int helper(int x) { return x; }

        void test(void) {
            int result = helper(42);
        }
    )";

    auto result = parser->parseString(source, "call.c");
    EXPECT_TRUE(result.success);
}

// === Preprocessor Tests ===

TEST_F(ClangParserTest, ParseWithDefines) {
    std::string source = R"(
        #define MAX_SIZE 100

        int buffer[MAX_SIZE];
    )";

    auto result = parser->parseString(source, "define.c");
    EXPECT_TRUE(result.success);
}

TEST_F(ClangParserTest, ParseWithIncludes) {
    std::string source = R"(
        void test(void) {
            int x = 0;
        }
    )";

    ClangParser::ParseOptions options;
    options.parsePreprocessor = true;

    auto result = parser->parseString(source, "include.c", options);
    EXPECT_TRUE(result.success);
}

// === Parse Options Tests ===

TEST_F(ClangParserTest, ParseWithIncludePaths) {
    std::string source = R"(
        void test(void) {}
    )";

    ClangParser::ParseOptions options;
    options.includePaths.push_back("/usr/include");

    auto result = parser->parseString(source, "test.c", options);
    EXPECT_TRUE(result.success);
}

TEST_F(ClangParserTest, ParseWithDefineOptions) {
    std::string source = R"(
        #ifdef MY_DEFINE
        int definedVar;
        #endif
    )";

    ClangParser::ParseOptions options;
    options.defines.push_back("MY_DEFINE");

    auto result = parser->parseString(source, "test.c", options);
    EXPECT_TRUE(result.success);
}

// === Error Handling Tests ===

TEST_F(ClangParserTest, ParseSyntaxError) {
    std::string source = R"(
        void broken {  // Missing parentheses
            return;
        }
    )";

    auto result = parser->parseString(source, "error.c");
    // Clang is forgiving, might still succeed with errors
    EXPECT_NE(result.ast, nullptr);
}

// === Complex Code Tests ===

TEST_F(ClangParserTest, ParseComplexFunction) {
    std::string source = R"(
        struct Node {
            int data;
            struct Node* next;
        };

        int sumList(struct Node* head) {
            int sum = 0;
            struct Node* current = head;

            while (current != 0) {
                sum += current->data;
                current = current->next;
            }

            return sum;
        }
    )";

    auto result = parser->parseString(source, "complex.c");
    EXPECT_TRUE(result.success);
    EXPECT_NE(result.ast, nullptr);

    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 3); // Node struct, sumList function, fields
}

TEST_F(ClangParserTest, ParseMultipleFunctions) {
    std::string source = R"(
        int add(int a, int b) { return a + b; }
        int sub(int a, int b) { return a - b; }
        int mul(int a, int b) { return a * b; }
        int div(int a, int b) { return b ? a / b : 0; }
    )";

    auto result = parser->parseString(source, "math.c");
    EXPECT_TRUE(result.success);

    auto symbols = result.semanticModel->getAllSymbols();
    EXPECT_GE(symbols.size(), 4); // Four functions
}

// === Type Information Tests ===

TEST_F(ClangParserTest, TypeSizeAndAlignment) {
    std::string source = R"(
        int intVar;
        char charVar;
        long longVar;
        double doubleVar;

        struct Packed {
            char c;
            int i;
        };
    )";

    auto result = parser->parseString(source, "sizes.c");
    EXPECT_TRUE(result.success);

    // Check that types have size and alignment set
    for (const auto& symbol : result.semanticModel->getAllSymbols()) {
        if (symbol->getType()) {
            auto type = symbol->getType();
            // Size and alignment should be non-zero for most types
            if (type->getKind() != ITypeInfo::TypeKind::VOID) {
                EXPECT_GT(type->getSize(), 0);
                EXPECT_GT(type->getAlignment(), 0);
            }
        }
    }
}
