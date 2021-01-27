#ifndef _INSTRUCTION_IMPL
#define _INSTRUCTION_IMPL

#include "model/instructions.hpp"


#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/types.hpp"

#include "support/random.hpp"
#include "except.hpp"
#include "tacimpl.hpp"
#include "cmpimpl.hpp"
#include "castimpl.hpp"



namespace MiniMC {
  namespace CPA {
    namespace Concrete {

	  using VariableLookup = MiniMC::Model::VariableMap<MiniMC::Util::Array>;
	  
	  struct GlobalLocalVariableLookup{
		VariableLookup* global;
		VariableLookup* local;
		auto LookUp (const MiniMC::Model::Variable_ptr& v) const  {
		  if (v->isGlobal ()) {
			return global->at (v);
		  }
		  else
			return local->at (v);
		}
		
		void set (const MiniMC::Model::Variable_ptr& v, const MiniMC::Util::Array& arr) {
		  assert (v->getType ()->getSize () == arr.getSize ());
		  if (v->isGlobal ()) {
			(*global)[v] = arr;
		  }
		  else  {
			assert(arr.getSize () == v->getType ()->getSize ());
			
			(*local)[v] = arr;
		  }
		}

		const MiniMC::Util::Array evaluate (const MiniMC::Model::Value_ptr& v) const {
		  if (v->isVariable ()) {
			auto var =  std::static_pointer_cast<MiniMC::Model::Variable> (v);
			auto arr = LookUp (var);
			assert(arr.getSize () == v->getType ()->getSize ());
			return arr;
		  }
		  else {
			auto constant = std::static_pointer_cast<MiniMC::Model::Constant> (v);
			if (!constant->isNonCompileConstant ()) {
			  MiniMC::Util::Array arr(constant->getSize ());
			  arr.set_block (0,constant->getSize (),constant->getData ());
			  return arr;
			}

			throw MiniMC::Support::Exception ("No Evaluation of constants available");
		  }
		}
		
	  };
	  
	  struct VMData {
		GlobalLocalVariableLookup readFrom;
		GlobalLocalVariableLookup writeTo;
		void finalise () {}
	  };
	  
	  struct ExecuteInstruction {

		template<MiniMC::Model::InstructionCode opc>
		static void execute (VMData& data,
							 const MiniMC::Model::Instruction& i)  {
		  MiniMC::Model::InstHelper<opc> helper (i);
		  if constexpr (MiniMC::Model::InstructionData<opc>::isTAC) {
			auto& res = helper.getResult ();
			auto& left = helper.getLeftOp ();
			auto& right = helper.getRightOp ();

			auto lval = data.readFrom.evaluate (left);
			auto rval = data.readFrom.evaluate (right);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Steptacexec<opc> (lval,rval));
			
		  }

		  else if constexpr (MiniMC::Model::InstructionData<opc>::isComparison) {
			auto& res = helper.getResult ();
			auto& left = helper.getLeftOp ();
			auto& right = helper.getRightOp ();
			 
			auto lval = data.readFrom.evaluate (left);
			auto rval = data.readFrom.evaluate (right);
			assert(lval.getSize () == left->getType()->getSize ());
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Stepcmpexec<opc> (lval,rval));
			
		  }

		  else if constexpr (MiniMC::Model::InstructionData<opc>::isPredicate) {
			 auto& left = helper.getLeftOp ();
			 auto& right = helper.getRightOp ();
			 
			 auto lval = data.readFrom.evaluate (left);
			 auto rval = data.readFrom.evaluate (right);
			 Steppredexec<opc> (lval,rval);
			 
		  }
		  
		  else if constexpr (MiniMC::Model::InstructionData<opc>::isCast) {
			auto& res = helper.getResult ();
			auto& left = helper.getCastee ();
			auto lval = data.readFrom.evaluate (left);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Stepcastexec1<opc> (lval,res->getType()->getSize()));
			
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assign) {
			auto& res = helper.getResult ();
			auto& left = helper.getValue ();
			auto lval = data.readFrom.evaluate (left);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), lval);
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Skip) {
			
		  }
		   
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assume) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (!lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssumeViolated ();
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Uniform) {
			auto& res = helper.getResult ();
			auto& min = helper.getMin ();
			auto& max = helper.getMax ();
			
			auto lmin = data.readFrom.evaluate (min);
			auto lmax = data.readFrom.evaluate (max);

			
			auto mod = [&]<typename T> () {
			  MiniMC::Util::Array arrres (sizeof (T));
			  arrres.template set<T> (0,MiniMC::Support::RandomNumber{}.uniform (lmin.template read<T>(), lmax.template read<T>()));;
			  data.writeTo.set (std::static_pointer_cast<MiniMC::Model::Variable> (res),arrres);
			};
			
			switch (res->getType()->getSize ()) {
			case 1:
			  mod.template operator()<MiniMC::uint8_t> ();
			  break;
			case 2:
			  mod.template operator()<MiniMC::uint16_t> ();
			  break;
			case 4:
			  mod.template operator()<MiniMC::uint32_t> ();
			  break;
			case 8:
			  mod.template operator()<MiniMC::uint64_t> ();
			  break;
			}
			
		  }
		  
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assert) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (!lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssertViolated ();
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::NegAssume) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssumeViolated ();
		  }
		  else {
			throw NotImplemented<opc> ();
		  }
		   
		}
	  };

	}
  }
}

namespace std {
  template<>
  struct hash<MiniMC::CPA::Concrete::VariableLookup > {
	auto operator() (const MiniMC::CPA::Concrete::VariableLookup& arr) {
	  return arr.hash (0);
	}
  };
  
}


#endif
