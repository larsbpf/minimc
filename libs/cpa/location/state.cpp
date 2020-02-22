#include "model/cfg.hpp"
#include "hash/hashing.hpp"
#include "cpa/interface.hpp"
#include "cpa/location.hpp"
#include "support/pointer.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Location {

      struct LocationState {
	void push (gsl::not_null<MiniMC::Model::Location*> l) {
	  
	  stack.push_back(l.get());}
	void pop () {
	  if(stack.size()>1)
	    stack.pop_back();
	  assert(stack.back());
	}
	auto& cur () {assert (stack.size()); return stack.back();}
	auto& cur () const {assert (stack.size()); return stack.back();}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  return MiniMC::Hash::Hash<MiniMC::Model::Location*> (stack.data(),stack.size(),seed);
	}

	
	std::vector<MiniMC::Model::Location*> stack;
	
      };
      
      class State : public MiniMC::CPA::State  {
      public:
	State (const std::vector<LocationState>& locations ) : locations(locations) {
	  
	}
	virtual std::ostream& output (std::ostream& os) const {
	  os << "[ ";
	  for (auto l : locations) {
	    assert(l.cur());
	    os << l.cur()->getName() << ", ";
	  }
	  return os << "]";
	}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  MiniMC::Hash::seed_t s = seed;
	  for (auto& t: locations)
	    s = t.hash(s);
	  return s;
	  //return MiniMC::Hash::Hash<MiniMC::Model::Location*> (locations.data(),locations.size(),seed);
	}
	virtual std::shared_ptr<MiniMC::CPA::Location::State> lcopy () const {return std::make_shared<State> (locations);}
	virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return lcopy();}

	
	size_t nbOfProcesses () const {return locations.size();}
	MiniMC::Model::Location_ptr getLocation (size_t i) const  {return locations[i].cur()->shared_from_this();}
	void setLocation (size_t i,MiniMC::Model::Location* l)   {
	  locations[i].cur() = l;
	}
	void pushLocation (size_t i, MiniMC::Model::Location* l) {locations[i].push (l);}
	void popLocation (size_t i) {locations[i].pop ();}
	bool isPotentialLoop () const {
	  for (auto& locState : locations) {
	    if (locState.cur()->template is<MiniMC::Model::Location::Attributes::LoopEntry> ())
	      return true;
	  }
	  return false;
	}
      private:
	std::vector<LocationState> locations;
      };

      MiniMC::CPA::State_ptr MiniMC::CPA::Location::Transferer::doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr& edge,proc_id id) {
	auto state = static_cast<const State*> (s.get ());
	assert(id < state->nbOfProcesses());
	if (edge->getFrom() == state->getLocation (id)) {
	  auto nstate = state->lcopy ();
	  nstate->setLocation (id,edge->getTo().get());

	  if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
	    auto& inst =  edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ().last(); 
	    if (inst.getOpcode () == MiniMC::Model::InstructionCode::Call) {
	      MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Call> helper (inst);
	      if (helper.getFunctionPtr ()->isConstant()) {
		auto constant = std::static_pointer_cast<MiniMC::Model::IntegerConstant> (helper.getFunctionPtr ());
		auto ptr = MiniMC::Support::CastToPtr (constant->getValue());
		auto func = edge->getProgram()->getFunction(MiniMC::Support::getFunctionId (ptr));
		nstate->pushLocation (id,func->getCFG()->getInitialLocation().get().get());
	      }
	      else
		return nullptr;
	    }
	    
	    
	  }
	  
	  if (!nstate->getLocation (id)->getEdges().size()) {
	    nstate->popLocation (id);
	  }
	 
	  
	  return nstate;
	}

	
	
	return nullptr;
      }
      
      State_ptr MiniMC::CPA::Location::StateQuery::makeInitialState (const MiniMC::Model::Program& p) {
	std::vector<LocationState> locs;
	for (auto& f : p.getEntryPoints()) {
	  locs.emplace_back();
	  locs.back().push(f->getCFG()->getInitialLocation().get().get());
	}
	return std::make_shared<State> (locs);
      }
      
      size_t MiniMC::CPA::Location::StateQuery::nbOfProcesses (const State_ptr& s) {
	auto state = static_cast<const State*> (s.get ());
	return state->nbOfProcesses ();
      }

      MiniMC::Model::Location_ptr MiniMC::CPA::Location::StateQuery::getLocation (const State_ptr& s, proc_id id) {
	auto state = static_cast<const State*> (s.get ());
	return state->getLocation (id);
      }
      
    }
  }
}
