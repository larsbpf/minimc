#include <unordered_map>

#include "model/types.hpp"

namespace MiniMC {
  namespace Model {
    class IntegerType : public Type {
    public:
      IntegerType(size_t b) : Type(TypeID::Integer),
                              bytes(b) {}
      virtual std::size_t getSize() const { return bytes; }
      virtual std::ostream& output(std::ostream& os) const { return os << "Int" << bytes * 8; }

    protected:
      virtual bool innerEq(const Type& t) {
        return bytes == static_cast<const IntegerType&>(t).bytes;
      }

    private:
      size_t bytes;
    };

    class FloatType : public Type {
    public:
      FloatType() : Type(TypeID::Float) {}
      virtual std::size_t getSize() const { return 4; }
      virtual std::ostream& output(std::ostream& os) const { return os << "Float"; }
      virtual bool innerEq(const Type& t) { return true; }
    };

    class DoubleType : public Type {
    public:
      DoubleType() : Type(TypeID::Double) {}
      virtual std::size_t getSize() const { return 8; }
      virtual std::ostream& output(std::ostream& os) const { return os << "Double"; }
      virtual bool innerEq(const Type& t) { return false; }
    };

    class PointerType : public Type {
    public:
      PointerType() : Type(TypeID::Pointer) {}
      virtual std::size_t getSize() const { return 8; }
      virtual std::ostream& output(std::ostream& os) const { return os << "Pointer"; }
      virtual bool innerEq(const Type& t) { return true; }
    };

    class BoolType : public Type {
    public:
      BoolType() : Type(TypeID::Bool) {}
      virtual std::size_t getSize() const { return 1; }
      virtual std::ostream& output(std::ostream& os) const { return os << "Bool"; }
      virtual bool innerEq(const Type& t) { return true; }
    };

    class VoidType : public Type {
    public:
      VoidType() : Type(TypeID::Void) {}
      virtual std::size_t getSize() const { return 0; }
      virtual std::ostream& output(std::ostream& os) const { return os << "Void"; }
      virtual bool innerEq(const Type& t) { return true; }
    };

    class AggregateType : public Type {
    public:
      AggregateType(TypeID h, size_t size) : Type(h), size(size) {}
      virtual std::size_t getSize() const { return size; }
      virtual std::ostream& output(std::ostream& os) const { return os << "Aggr:" << size; }
      virtual bool innerEq(const Type& t) { return size == static_cast<const AggregateType&>(t).size; }

    private:
      std::size_t size;
    };

    class StructType : public AggregateType {
    public:
      StructType(std::size_t size) : AggregateType(TypeID::Struct, size) {}
    };

    class ArrayType : public AggregateType {
    public:
      ArrayType(std::size_t size) : AggregateType(TypeID::Array, size) {}
    };

    struct TypeFactory64::Inner {
      Inner() : vt(new VoidType()),
                dt(new DoubleType()),
                ft(new FloatType()),
                bt(new BoolType()),
                pt(new PointerType()),
                i8(new IntegerType(1)),
                i16(new IntegerType(2)),
                i32(new IntegerType(4)),
                i64(new IntegerType(8)) {}
      Type_ptr vt;
      Type_ptr dt;
      Type_ptr ft;
      Type_ptr bt;
      Type_ptr pt;
      Type_ptr i8;
      Type_ptr i16;
      Type_ptr i32;
      Type_ptr i64;

      std::unordered_map<size_t, Type_ptr> structs;
      std::unordered_map<size_t, Type_ptr> arrays;
    };

    TypeFactory64::TypeFactory64() {
      impl = std::make_unique<Inner>();
    }
    TypeFactory64::~TypeFactory64() {}

    const Type_ptr TypeFactory64::makeIntegerType(size_t t) {
      if (t <= 8) {
        return impl->i8;
      } else if (t <= 16) {
        return impl->i16;
      } else if (t <= 32) {
        return impl->i32;
      } else if (t <= 64) {
        return impl->i64;
      }

      else
        return nullptr;
    }

    const Type_ptr TypeFactory64::makeFloatType() { return impl->ft; }
    const Type_ptr TypeFactory64::makeBoolType() { return impl->bt; }
    const Type_ptr TypeFactory64::makeDoubleType() { return impl->dt; }
    const Type_ptr TypeFactory64::makePointerType() { return impl->pt; }
    const Type_ptr TypeFactory64::makeVoidType() { return impl->vt; }
    const Type_ptr TypeFactory64::makeArrayType(size_t t) {
      if (!impl->arrays.count(t)) {
        impl->arrays.insert(std::make_pair(t, std::make_shared<ArrayType>(t)));
      }
      return impl->arrays.at(t);
    }

    const Type_ptr TypeFactory64::makeStructType(size_t t) {
      if (!impl->structs.count(t)) {
        impl->structs.insert(std::make_pair(t, std::make_shared<ArrayType>(t)));
      }
      return impl->structs.at(t);
    }

  } // namespace Model
} // namespace MiniMC
