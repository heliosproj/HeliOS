#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "nomic/core/interfaces.h"
#include "nomic/core/type_info.h"
#include "nomic/core/ast_factory.h"
#include <limits>
#include <memory>

using namespace nomic::core;
using ::testing::Return;
using ::testing::NotNull;
using ::testing::IsNull;
using ::testing::IsEmpty;

/**
 * TDD Unit Tests for TypeInfo Implementation
 * Testing edge cases and boundary conditions per Claude Code instructions
 */

class TypeInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        factory = std::make_unique<ASTFactory>();
    }

    std::unique_ptr<IASTFactory> factory;
};

// Test: Create types with all possible TypeKind values
TEST_F(TypeInfoTest, CreateAllTypeKinds) {
    std::vector<ITypeInfo::TypeKind> allKinds = {
        ITypeInfo::TypeKind::VOID,
        ITypeInfo::TypeKind::BOOL,
        ITypeInfo::TypeKind::CHAR,
        ITypeInfo::TypeKind::INT,
        ITypeInfo::TypeKind::FLOAT,
        ITypeInfo::TypeKind::DOUBLE,
        ITypeInfo::TypeKind::POINTER,
        ITypeInfo::TypeKind::ARRAY,
        ITypeInfo::TypeKind::FUNCTION,
        ITypeInfo::TypeKind::STRUCT,
        ITypeInfo::TypeKind::UNION,
        ITypeInfo::TypeKind::ENUM,
        ITypeInfo::TypeKind::TYPEDEF,
        ITypeInfo::TypeKind::QUALIFIED,
        ITypeInfo::TypeKind::UNKNOWN
    };

    for (auto kind : allKinds) {
        auto type = factory->createType(kind);
        ASSERT_THAT(type, NotNull()) << "Failed to create type of kind: " << static_cast<int>(kind);
        EXPECT_EQ(type->getKind(), kind);
        EXPECT_FALSE(type->getTypeName().empty()) << "Type name should not be empty";
    }
}

// Test: Invalid type kind (out of enum range)
TEST_F(TypeInfoTest, CreateInvalidTypeKind) {
    auto invalidKind = static_cast<ITypeInfo::TypeKind>(999);
    auto type = factory->createType(invalidKind);
    if (type) {
        EXPECT_EQ(type->getKind(), ITypeInfo::TypeKind::UNKNOWN);
    }
}

// Test: Type size boundary values
TEST_F(TypeInfoTest, TypeSizeBoundaries) {
    // Test primitive types with known sizes
    auto charType = factory->createType(ITypeInfo::TypeKind::CHAR);
    ASSERT_THAT(charType, NotNull());
    EXPECT_EQ(charType->getSize(), sizeof(char));

    auto intType = factory->createType(ITypeInfo::TypeKind::INT);
    ASSERT_THAT(intType, NotNull());
    EXPECT_EQ(intType->getSize(), sizeof(int));

    auto doubleType = factory->createType(ITypeInfo::TypeKind::DOUBLE);
    ASSERT_THAT(doubleType, NotNull());
    EXPECT_EQ(doubleType->getSize(), sizeof(double));

    // Test void type (special case - size 0 or 1)
    auto voidType = factory->createType(ITypeInfo::TypeKind::VOID);
    ASSERT_THAT(voidType, NotNull());
    EXPECT_LE(voidType->getSize(), 1);

    // Test maximum size type (array with max elements)
    auto arrayType = factory->createType(ITypeInfo::TypeKind::ARRAY);
    ASSERT_THAT(arrayType, NotNull());
    // Size should be reasonable, not overflow
    EXPECT_LE(arrayType->getSize(), std::numeric_limits<size_t>::max());
}

// Test: Type alignment boundary values
TEST_F(TypeInfoTest, TypeAlignmentBoundaries) {
    auto charType = factory->createType(ITypeInfo::TypeKind::CHAR);
    ASSERT_THAT(charType, NotNull());
    EXPECT_EQ(charType->getAlignment(), alignof(char));

    auto intType = factory->createType(ITypeInfo::TypeKind::INT);
    ASSERT_THAT(intType, NotNull());
    EXPECT_EQ(intType->getAlignment(), alignof(int));

    auto doubleType = factory->createType(ITypeInfo::TypeKind::DOUBLE);
    ASSERT_THAT(doubleType, NotNull());
    EXPECT_EQ(doubleType->getAlignment(), alignof(double));

    // Alignment should be power of 2 and reasonable
    auto structType = factory->createType(ITypeInfo::TypeKind::STRUCT);
    ASSERT_THAT(structType, NotNull());
    size_t alignment = structType->getAlignment();
    EXPECT_TRUE((alignment & (alignment - 1)) == 0) << "Alignment should be power of 2";
    EXPECT_LE(alignment, 128) << "Alignment should be reasonable";
}

// Test: Const and volatile qualifiers
TEST_F(TypeInfoTest, TypeQualifiers) {
    auto basicType = factory->createType(ITypeInfo::TypeKind::INT);
    ASSERT_THAT(basicType, NotNull());

    // Basic type should not be qualified by default
    EXPECT_FALSE(basicType->isConst());
    EXPECT_FALSE(basicType->isVolatile());

    // Create qualified type
    auto qualifiedType = factory->createType(ITypeInfo::TypeKind::QUALIFIED);
    ASSERT_THAT(qualifiedType, NotNull());

    // Test all combinations
    dynamic_cast<TypeInfo*>(qualifiedType.get())->setConst(true);
    dynamic_cast<TypeInfo*>(qualifiedType.get())->setVolatile(false);
    EXPECT_TRUE(qualifiedType->isConst());
    EXPECT_FALSE(qualifiedType->isVolatile());

    dynamic_cast<TypeInfo*>(qualifiedType.get())->setConst(false);
    dynamic_cast<TypeInfo*>(qualifiedType.get())->setVolatile(true);
    EXPECT_FALSE(qualifiedType->isConst());
    EXPECT_TRUE(qualifiedType->isVolatile());

    dynamic_cast<TypeInfo*>(qualifiedType.get())->setConst(true);
    dynamic_cast<TypeInfo*>(qualifiedType.get())->setVolatile(true);
    EXPECT_TRUE(qualifiedType->isConst());
    EXPECT_TRUE(qualifiedType->isVolatile());
}

// Test: Pointer type edge cases
TEST_F(TypeInfoTest, PointerTypeEdgeCases) {
    auto ptrType = factory->createType(ITypeInfo::TypeKind::POINTER);
    ASSERT_THAT(ptrType, NotNull());
    EXPECT_TRUE(ptrType->isPointer());
    EXPECT_FALSE(ptrType->isArray());
    EXPECT_FALSE(ptrType->isFunction());

    // Pointer to void
    auto voidType = factory->createType(ITypeInfo::TypeKind::VOID);
    dynamic_cast<TypeInfo*>(ptrType.get())->setPointeeType(voidType);
    auto pointee = ptrType->getPointeeType();
    ASSERT_THAT(pointee, NotNull());
    EXPECT_EQ(pointee->getKind(), ITypeInfo::TypeKind::VOID);

    // Multi-level pointer (pointer to pointer)
    auto ptrToPtrType = factory->createType(ITypeInfo::TypeKind::POINTER);
    dynamic_cast<TypeInfo*>(ptrToPtrType.get())->setPointeeType(ptrType);
    EXPECT_TRUE(ptrToPtrType->isPointer());
    auto level1 = ptrToPtrType->getPointeeType();
    ASSERT_THAT(level1, NotNull());
    EXPECT_TRUE(level1->isPointer());
    auto level2 = level1->getPointeeType();
    ASSERT_THAT(level2, NotNull());

    // Null pointer type (edge case)
    auto nullPtrType = factory->createType(ITypeInfo::TypeKind::POINTER);
    auto nullPointee = nullPtrType->getPointeeType();
    // Should handle gracefully - either null or default type
    if (nullPointee) {
        EXPECT_EQ(nullPointee->getKind(), ITypeInfo::TypeKind::UNKNOWN);
    } else {
        EXPECT_THAT(nullPointee, IsNull());
    }
}

// Test: Array type edge cases
TEST_F(TypeInfoTest, ArrayTypeEdgeCases) {
    auto arrayType = factory->createType(ITypeInfo::TypeKind::ARRAY);
    ASSERT_THAT(arrayType, NotNull());
    EXPECT_TRUE(arrayType->isArray());
    EXPECT_FALSE(arrayType->isPointer());
    EXPECT_FALSE(arrayType->isFunction());

    // Zero-sized array
    dynamic_cast<TypeInfo*>(arrayType.get())->setArraySize(0);
    EXPECT_EQ(dynamic_cast<TypeInfo*>(arrayType.get())->getArraySize(), 0);

    // Maximum sized array
    dynamic_cast<TypeInfo*>(arrayType.get())->setArraySize(std::numeric_limits<size_t>::max());
    EXPECT_EQ(dynamic_cast<TypeInfo*>(arrayType.get())->getArraySize(), std::numeric_limits<size_t>::max());

    // Variable-length array (size = -1 or special marker)
    dynamic_cast<TypeInfo*>(arrayType.get())->setArraySize(static_cast<size_t>(-1));
    auto size = dynamic_cast<TypeInfo*>(arrayType.get())->getArraySize();
    EXPECT_TRUE(size == static_cast<size_t>(-1) || size == 0);
}

// Test: Function type edge cases
TEST_F(TypeInfoTest, FunctionTypeEdgeCases) {
    auto funcType = factory->createType(ITypeInfo::TypeKind::FUNCTION);
    ASSERT_THAT(funcType, NotNull());
    EXPECT_TRUE(funcType->isFunction());
    EXPECT_FALSE(funcType->isPointer());
    EXPECT_FALSE(funcType->isArray());

    // Function with no parameters
    auto params = funcType->getParameterTypes();
    EXPECT_THAT(params, IsEmpty());

    // Function returning void
    auto voidType = factory->createType(ITypeInfo::TypeKind::VOID);
    dynamic_cast<TypeInfo*>(funcType.get())->setReturnType(voidType);
    auto returnType = funcType->getReturnType();
    ASSERT_THAT(returnType, NotNull());
    EXPECT_EQ(returnType->getKind(), ITypeInfo::TypeKind::VOID);

    // Function with many parameters (stress test)
    std::vector<TypeInfoPtr> manyParams;
    for (int i = 0; i < 100; ++i) {
        manyParams.push_back(factory->createType(ITypeInfo::TypeKind::INT));
    }
    dynamic_cast<TypeInfo*>(funcType.get())->setParameterTypes(manyParams);
    auto retrievedParams = funcType->getParameterTypes();
    EXPECT_EQ(retrievedParams.size(), 100);

    // Variadic function (special case)
    dynamic_cast<TypeInfo*>(funcType.get())->setVariadic(true);
    EXPECT_TRUE(dynamic_cast<TypeInfo*>(funcType.get())->isVariadic());
}

// Test: Incomplete types
TEST_F(TypeInfoTest, IncompleteTypes) {
    // Forward declared struct
    auto structType = factory->createType(ITypeInfo::TypeKind::STRUCT);
    ASSERT_THAT(structType, NotNull());

    // Initially incomplete
    dynamic_cast<TypeInfo*>(structType.get())->setComplete(false);
    EXPECT_FALSE(structType->isComplete());
    EXPECT_EQ(structType->getSize(), 0); // Incomplete types have size 0

    // Make complete
    dynamic_cast<TypeInfo*>(structType.get())->setComplete(true);
    dynamic_cast<TypeInfo*>(structType.get())->setSize(24);
    EXPECT_TRUE(structType->isComplete());
    EXPECT_EQ(structType->getSize(), 24);

    // Array of incomplete type (should be incomplete)
    auto arrayType = factory->createType(ITypeInfo::TypeKind::ARRAY);
    auto incompleteElement = factory->createType(ITypeInfo::TypeKind::STRUCT);
    dynamic_cast<TypeInfo*>(incompleteElement.get())->setComplete(false);
    dynamic_cast<TypeInfo*>(arrayType.get())->setElementType(incompleteElement);
    EXPECT_FALSE(arrayType->isComplete());
}

// Test: Canonical type handling
TEST_F(TypeInfoTest, CanonicalTypes) {
    // Typedef should have different canonical type
    auto typedefType = factory->createType(ITypeInfo::TypeKind::TYPEDEF);
    auto intType = factory->createType(ITypeInfo::TypeKind::INT);

    ASSERT_THAT(typedefType, NotNull());
    ASSERT_THAT(intType, NotNull());

    dynamic_cast<TypeInfo*>(typedefType.get())->setCanonicalType(intType);
    auto canonical = typedefType->getCanonicalType();
    ASSERT_THAT(canonical, NotNull());
    EXPECT_EQ(canonical->getKind(), ITypeInfo::TypeKind::INT);

    // Canonical of canonical should be self
    auto canonicalOfCanonical = canonical->getCanonicalType();
    EXPECT_EQ(canonicalOfCanonical, canonical);

    // Qualified type canonical
    auto qualifiedType = factory->createType(ITypeInfo::TypeKind::QUALIFIED);
    dynamic_cast<TypeInfo*>(qualifiedType.get())->setCanonicalType(intType);
    dynamic_cast<TypeInfo*>(qualifiedType.get())->setConst(true);
    auto qualCanonical = qualifiedType->getCanonicalType();
    EXPECT_TRUE(qualifiedType->isConst());
    EXPECT_FALSE(qualCanonical->isConst()); // Canonical should be unqualified
}

// Test: Type name edge cases
TEST_F(TypeInfoTest, TypeNameEdgeCases) {
    auto type = factory->createType(ITypeInfo::TypeKind::STRUCT);
    ASSERT_THAT(type, NotNull());

    // Empty name
    dynamic_cast<TypeInfo*>(type.get())->setTypeName("");
    EXPECT_EQ(type->getTypeName(), "");

    // Very long name
    std::string longName(10000, 'a');
    dynamic_cast<TypeInfo*>(type.get())->setTypeName(longName);
    EXPECT_EQ(type->getTypeName(), longName);

    // Name with special characters
    std::string specialName = "type<int, float>::nested_t*[10]";
    dynamic_cast<TypeInfo*>(type.get())->setTypeName(specialName);
    EXPECT_EQ(type->getTypeName(), specialName);

    // Unicode name
    std::string unicodeName = "型別_类型_τύπος";
    dynamic_cast<TypeInfo*>(type.get())->setTypeName(unicodeName);
    EXPECT_EQ(type->getTypeName(), unicodeName);

    // Name with null character
    std::string nullName = "type\0hidden";
    dynamic_cast<TypeInfo*>(type.get())->setTypeName(nullName);
    EXPECT_EQ(type->getTypeName(), nullName);
}

// Test: Circular type references
TEST_F(TypeInfoTest, CircularTypeReferences) {
    // Struct containing pointer to itself
    auto structType = factory->createType(ITypeInfo::TypeKind::STRUCT);
    auto ptrType = factory->createType(ITypeInfo::TypeKind::POINTER);

    ASSERT_THAT(structType, NotNull());
    ASSERT_THAT(ptrType, NotNull());

    dynamic_cast<TypeInfo*>(ptrType.get())->setPointeeType(structType);
    dynamic_cast<TypeInfo*>(structType.get())->addField("next", ptrType);

    // Should not cause infinite loop
    auto fieldType = dynamic_cast<TypeInfo*>(structType.get())->getFieldType("next");
    ASSERT_THAT(fieldType, NotNull());
    EXPECT_TRUE(fieldType->isPointer());
    auto pointee = fieldType->getPointeeType();
    EXPECT_EQ(pointee, structType);
}

// Test: Anonymous types
TEST_F(TypeInfoTest, AnonymousTypes) {
    // Anonymous struct
    auto anonStruct = factory->createType(ITypeInfo::TypeKind::STRUCT);
    ASSERT_THAT(anonStruct, NotNull());

    dynamic_cast<TypeInfo*>(anonStruct.get())->setTypeName("");
    EXPECT_EQ(anonStruct->getTypeName(), "");
    EXPECT_TRUE(anonStruct->getTypeName().empty());

    // Anonymous union
    auto anonUnion = factory->createType(ITypeInfo::TypeKind::UNION);
    ASSERT_THAT(anonUnion, NotNull());

    dynamic_cast<TypeInfo*>(anonUnion.get())->setTypeName("");
    EXPECT_EQ(anonUnion->getTypeName(), "");

    // Anonymous enum
    auto anonEnum = factory->createType(ITypeInfo::TypeKind::ENUM);
    ASSERT_THAT(anonEnum, NotNull());

    dynamic_cast<TypeInfo*>(anonEnum.get())->setTypeName("");
    EXPECT_EQ(anonEnum->getTypeName(), "");
}