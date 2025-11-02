#include "nomic/core/type_info.h"
#include <algorithm>

namespace nomic {
namespace core {

TypeInfo::TypeInfo(TypeKind kind)
    : kind_(kind),
      size_(0),
      alignment_(1),
      isConst_(false),
      isVolatile_(false),
      isComplete_(true),  // Most types are complete by default
      isVariadic_(false),
      arraySize_(0) {

    initializeTypeProperties();
}

void TypeInfo::initializeTypeProperties() {
    // Set default properties based on type kind
    switch (kind_) {
        case TypeKind::VOID:
            typeName_ = "void";
            size_ = 1;  // GCC extension: sizeof(void) = 1
            alignment_ = 1;
            break;

        case TypeKind::BOOL:
            typeName_ = "_Bool";
            size_ = sizeof(bool);
            alignment_ = alignof(bool);
            break;

        case TypeKind::CHAR:
            typeName_ = "char";
            size_ = sizeof(char);
            alignment_ = alignof(char);
            break;

        case TypeKind::INT:
            typeName_ = "int";
            size_ = sizeof(int);
            alignment_ = alignof(int);
            break;

        case TypeKind::FLOAT:
            typeName_ = "float";
            size_ = sizeof(float);
            alignment_ = alignof(float);
            break;

        case TypeKind::DOUBLE:
            typeName_ = "double";
            size_ = sizeof(double);
            alignment_ = alignof(double);
            break;

        case TypeKind::POINTER:
            typeName_ = "pointer";
            size_ = sizeof(void*);
            alignment_ = alignof(void*);
            break;

        case TypeKind::ARRAY:
            typeName_ = "array";
            // Size depends on element type and array size
            isComplete_ = false;  // Arrays need element info to be complete
            break;

        case TypeKind::FUNCTION:
            typeName_ = "function";
            // Functions don't have a size
            size_ = 0;
            alignment_ = 1;
            break;

        case TypeKind::STRUCT:
            typeName_ = "struct";
            isComplete_ = false;  // Structs start as incomplete (forward declaration)
            alignment_ = 1;
            break;

        case TypeKind::UNION:
            typeName_ = "union";
            isComplete_ = false;  // Unions start as incomplete
            alignment_ = 1;
            break;

        case TypeKind::ENUM:
            typeName_ = "enum";
            size_ = sizeof(int);  // Enums are typically int-sized
            alignment_ = alignof(int);
            break;

        case TypeKind::TYPEDEF:
            typeName_ = "typedef";
            // Size depends on underlying type
            break;

        case TypeKind::QUALIFIED:
            typeName_ = "qualified";
            // Size depends on underlying type
            break;

        case TypeKind::UNKNOWN:
        default:
            typeName_ = "unknown";
            size_ = 0;
            alignment_ = 1;
            isComplete_ = false;
            break;
    }

    // Calculate default size and alignment if needed
    if (size_ == 0 && isComplete_) {
        size_ = calculateDefaultSize();
    }
    if (alignment_ == 1 && isComplete_) {
        alignment_ = calculateDefaultAlignment();
    }
}

size_t TypeInfo::calculateDefaultSize() const {
    switch (kind_) {
        case TypeKind::ARRAY:
            if (elementType_ && arraySize_ > 0) {
                return elementType_->getSize() * arraySize_;
            }
            break;

        case TypeKind::STRUCT:
        case TypeKind::UNION:
            {
                size_t totalSize = 0;
                size_t maxSize = 0;
                for (const auto& [name, fieldType] : fields_) {
                    if (fieldType) {
                        size_t fieldSize = fieldType->getSize();
                        if (kind_ == TypeKind::STRUCT) {
                            // Structs: sum of all fields (simplified, ignoring padding)
                            totalSize += fieldSize;
                        } else {
                            // Unions: maximum of all fields
                            maxSize = std::max(maxSize, fieldSize);
                        }
                    }
                }
                return (kind_ == TypeKind::STRUCT) ? totalSize : maxSize;
            }

        case TypeKind::TYPEDEF:
        case TypeKind::QUALIFIED:
            if (canonicalType_) {
                return canonicalType_->getSize();
            }
            break;

        default:
            break;
    }

    return size_;
}

size_t TypeInfo::calculateDefaultAlignment() const {
    switch (kind_) {
        case TypeKind::ARRAY:
            if (elementType_) {
                return elementType_->getAlignment();
            }
            break;

        case TypeKind::STRUCT:
        case TypeKind::UNION:
            {
                size_t maxAlign = 1;
                for (const auto& [name, fieldType] : fields_) {
                    if (fieldType) {
                        maxAlign = std::max(maxAlign, fieldType->getAlignment());
                    }
                }
                return maxAlign;
            }

        case TypeKind::TYPEDEF:
        case TypeKind::QUALIFIED:
            if (canonicalType_) {
                return canonicalType_->getAlignment();
            }
            break;

        default:
            break;
    }

    // Ensure alignment is power of 2
    size_t align = alignment_;
    if (align > 0 && (align & (align - 1)) != 0) {
        // Round up to next power of 2
        align = 1;
        while (align < alignment_) {
            align <<= 1;
        }
    }

    return align;
}

} // namespace core
} // namespace nomic