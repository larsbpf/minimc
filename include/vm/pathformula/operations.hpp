#ifndef _VM_PATHFORMULA_OPERATIONS__
#define _VM_PATHFORMULA_OPERATIONS__

#include "vm/pathformula/value.hpp"

#include <memory>

namespace SMTLib {
  class TermBuilder;
}

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      struct Operations {
	Operations (SMTLib::TermBuilder& builder) : builder(builder) {}

	template<class T>
	T Add (const T&, const T&);

	template<class T>
	T Sub (const T&, const T&);

	template<class T>
	T Mul (const T&, const T&);

	template<class T>
	T UDiv (const T&, const T&);

	template<class T>
	T SDiv (const T&, const T&);

	template<class T>
	T LShl (const T&, const T&);

	template<class T>
	T LShr (const T&, const T&);

	template<class T>
	T AShr (const T&, const T&);

	template<class T>
	T And (const T&, const T&);

	template<class T>
	T Or (const T&, const T&);

	template<class T>
	T Xor (const T&, const T&);

	template<class T>
	Value<ValType::Bool> SGt (const T&, const T&);

	template<class T>
	Value<ValType::Bool> SGe (const T&, const T&);

	template<class T>
	Value<ValType::Bool> SLt (const T&, const T&);

	template<class T>
	Value<ValType::Bool> SLe (const T&, const T&);

	template<class T>
	Value<ValType::Bool> UGt (const T&, const T&);

	template<class T>
	Value<ValType::Bool> UGe (const T&, const T&);

	template<class T>
	Value<ValType::Bool> ULt (const T&, const T&);

	template<class T>
	Value<ValType::Bool> ULe (const T&, const T&);

	template<class T>
	Value<ValType::Bool> Eq (const T&, const T&);

	template<class T>
	Value<ValType::Bool> NEq (const T&, const T&);

	BoolValue PtrEq(const PointerValue&, const PointerValue&) {
	    throw MiniMC::Support::Exception ("Not Implemented");
	}

	BoolValue  BoolNegate(const BoolValue&);
	
        template <class T>
        PointerValue PtrAdd(const PointerValue&, const T&) {
	  throw MiniMC::Support::Exception ("Not Implemented");
	}
	
	template <class T>
        T ExtractBaseValue(const AggregateValue&, const MiniMC::uint64_t);
	AggregateValue ExtractAggregateValue(const AggregateValue&, const MiniMC::uint64_t, std::size_t);

	template <class T>
        AggregateValue InsertBaseValue(const AggregateValue&, const MiniMC::uint64_t, const T&);

	AggregateValue InsertAggregateValue(const AggregateValue&, const MiniMC::uint64_t, const AggregateValue&);
	
      private:
	SMTLib::TermBuilder& builder;
      };

      template <size_t>
      struct RetTyp;

      template <>
      struct RetTyp<1> {
        using type = Value<ValType::I8>;
	using underlying_type = MiniMC::uint8_t;
      };

      template <>
      struct RetTyp<2> {
        using type = Value<ValType::I16>;
	using underlying_type = MiniMC::uint16_t;
      };

      template <>
      struct RetTyp<4> {
        using type = Value<ValType::I32>;
	using underlying_type = MiniMC::uint32_t;
	
      };

      template <>
      struct RetTyp<8> {
        using type = Value<ValType::I64>;
	using underlying_type = MiniMC::uint64_t;
	
      };
      
      struct Casts {
	Casts (SMTLib::TermBuilder& builder) : builder(builder) {}

	template <std::size_t bw>
	typename RetTyp<bw>::type BoolZExt(const Value<ValType::Bool>& val);

	template <std::size_t bw>
	typename RetTyp<bw>::type BoolSExt(const Value<ValType::Bool>& val);

	template <class T>
        Value<ValType::Bool> IntToBool(const T& t);

	template <class T>
        Value<ValType::Pointer> IntToPtr(const T& t);

	template <size_t bw, class T>
        typename RetTyp<bw>::type Trunc(const T& t) const;

	template <size_t bw, typename T>
        typename RetTyp<bw>::type ZExt(const T& t) const;

	template <size_t bw, typename T>
        typename RetTyp<bw>::type SExt(const T& t) const;
      private:
	SMTLib::TermBuilder& builder;
      
      };

      
      
    }
  }
}


#endif