#ifndef NOMIC_CORE_TYPE_INFO_H
#define NOMIC_CORE_TYPE_INFO_H

#include "nomic/core/interfaces.h"
#include <unordered_map>

namespace nomic {
namespace core {

/**
 * @brief Concrete implementation of ITypeInfo interface
 *
 * Provides comprehensive type information with support for
 * qualifiers, composite types, and type relationships.
 */
class TypeInfo : public ITypeInfo, public std::enable_shared_from_this<TypeInfo> {
private:
    TypeKind kind_;
    std::string typeName_;
    size_t size_;
    size_t alignment_;
    bool isConst_;
    bool isVolatile_;
    bool isComplete_;
    bool isVariadic_;

    TypeInfoPtr canonicalType_;
    TypeInfoPtr pointeeType_;
    TypeInfoPtr elementType_;  // For arrays
    TypeInfoPtr returnType_;    // For functions
    std::vector<TypeInfoPtr> parameterTypes_;  // For functions
    size_t arraySize_;  // For arrays

    std::unordered_map<std::string, TypeInfoPtr> fields_;  // For structs/unions

public:
    // Constructor
    explicit TypeInfo(TypeKind kind);
    virtual ~TypeInfo() = default;

    // ITypeInfo interface implementation
    TypeKind getKind() const override { return kind_; }
    std::string getTypeName() const override { return typeName_; }
    size_t getSize() const override { return isComplete_ ? size_ : 0; }
    size_t getAlignment() const override { return alignment_; }
    bool isConst() const override { return isConst_; }
    bool isVolatile() const override { return isVolatile_; }
    bool isPointer() const override { return kind_ == TypeKind::POINTER; }
    bool isArray() const override { return kind_ == TypeKind::ARRAY; }
    bool isFunction() const override { return kind_ == TypeKind::FUNCTION; }
    bool isComplete() const override { return isComplete_; }

    TypeInfoPtr getCanonicalType() const override {
        if (canonicalType_) {
            return canonicalType_;
        }
        // Return self if no canonical type is set
        return std::const_pointer_cast<TypeInfo>(shared_from_this());
    }

    TypeInfoPtr getPointeeType() const override { return pointeeType_; }
    TypeInfoPtr getReturnType() const override { return returnType_; }
    std::vector<TypeInfoPtr> getParameterTypes() const override { return parameterTypes_; }

    // Setters for building type information
    void setTypeName(const std::string& name) { typeName_ = name; }
    void setSize(size_t size) { size_ = size; }
    void setAlignment(size_t align) { alignment_ = align; }
    void setConst(bool isConst) { isConst_ = isConst; }
    void setVolatile(bool isVolatile) { isVolatile_ = isVolatile; }
    void setComplete(bool complete) { isComplete_ = complete; }
    void setVariadic(bool variadic) { isVariadic_ = variadic; }
    void setCanonicalType(TypeInfoPtr type) { canonicalType_ = type; }
    void setPointeeType(TypeInfoPtr type) { pointeeType_ = type; }
    void setElementType(TypeInfoPtr type) { elementType_ = type; }
    void setReturnType(TypeInfoPtr type) { returnType_ = type; }
    void setParameterTypes(const std::vector<TypeInfoPtr>& types) { parameterTypes_ = types; }
    void setArraySize(size_t size) { arraySize_ = size; }

    // Additional methods for struct/union fields
    void addField(const std::string& name, TypeInfoPtr type) {
        fields_[name] = type;
    }

    TypeInfoPtr getFieldType(const std::string& name) const {
        auto it = fields_.find(name);
        return (it != fields_.end()) ? it->second : nullptr;
    }

    // Extended interface for additional functionality
    bool isVariadic() const { return isVariadic_; }
    size_t getArraySize() const { return arraySize_; }
    TypeInfoPtr getElementType() const { return elementType_; }

private:
    void initializeTypeProperties();
    size_t calculateDefaultSize() const;
    size_t calculateDefaultAlignment() const;
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_TYPE_INFO_H