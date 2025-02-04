#ifndef _COMPOUND__
#define _COMPOUND__

#include "cpa/interface.hpp"
#include "hash/hashing.hpp"
#include <initializer_list>
#include <set>
#include <vector>

namespace MiniMC {
  namespace CPA {
    namespace Compounds {
      class State : public MiniMC::CPA::State {
      public:
        State(std::initializer_list<MiniMC::CPA::State_ptr> l) {
          std::copy(l.begin(), l.end(), std::back_inserter(states));
        }

        State(std::vector<MiniMC::CPA::State_ptr>& l) {
          std::copy(l.begin(), l.end(), std::back_inserter(states));
        }

        virtual MiniMC::Hash::hash_t hash(MiniMC::Hash::seed_t seed = 0) const override {
          MiniMC::Hash::hash_t hash = seed;

          for (auto& state : states) {
            MiniMC::Hash::hash_combine(hash, *state);
          }
          return hash;
        }

        bool need2Store() const {
          for (auto& state : states) {
            if (state->need2Store()) {
              return true;
            }
          }
          return false;
        }

        virtual bool assertViolated() const {
          for (auto& state : states) {
            if (state->assertViolated()) {
              return true;
            }
          }
          return false;
        }

        virtual bool hasLocationAttribute(MiniMC::Model::AttrType tt) const {
          for (auto& state : states) {
            if (state->hasLocationAttribute(tt)) {
              return true;
            }
          }
          return false;
        }

        bool ready2explore() const override {
          for (auto& state : states) {
            if (!state->ready2explore()) {
              return false;
            }
          }
          return true;
        }

        virtual std::ostream& output(std::ostream& os) const {
          for (auto& state : states) {
            state->output(os);
            os << "\n________________\n";
          }
          return os;
        }

        const State_ptr& get(size_t i) const { return states[i]; }

        virtual const Concretizer_ptr getConcretizer() const override {

          return this->get(1)->getConcretizer();
          //return Concretizer_ptr (this);
        }

        virtual std::shared_ptr<MiniMC::CPA::State> copy() const {
          std::vector<MiniMC::CPA::State_ptr> copies;
          std::for_each(states.begin(), states.end(), [&](auto& s) { copies.push_back(s->copy()); });
          return std::make_shared<State>(copies);
        }

        virtual MiniMC::Model::Location_ptr getLocation(proc_id id) const override {
          return this->get(0)->getLocation(id);
        }

        size_t nbOfProcesses() const override {
          return this->get(0)->nbOfProcesses();
        }

      private:
        std::vector<MiniMC::CPA::State_ptr> states;
      };

      /* template<unsigned int N, class Head, class... Tail>
      struct GetNthTemplateArgument : GetNthTemplateArgument<N-1,Tail...>
      {
      };


      template<class Head, class... Tail>
      struct GetNthTemplateArgument<0,Head,Tail...>
      {
	using Temp = Head;;
      };
      */
      struct StateQuery : public MiniMC::CPA::StateQuery {
        StateQuery(std::vector<MiniMC::CPA::StateQuery_ptr> pts) : states(pts) {}
        State_ptr makeInitialState(const MiniMC::Model::Program& prgm) {
          //std::initializer_list<MiniMC::CPA::State_ptr> init ( {(args::Query::makeInitialState (prgm)) ...});
          //return std::make_shared<State<sizeof... (args)>> (init);
          std::vector<MiniMC::CPA::State_ptr> statees;
          auto inserter = std::back_inserter(statees);
          std::for_each(states.begin(), states.end(), [&inserter, &prgm](auto& it) { inserter = (it->makeInitialState(prgm)); });
          return std::make_shared<State>(statees);
        }

      private:
        std::vector<MiniMC::CPA::StateQuery_ptr> states;
      };

      /*template<size_t i, size_t statesize, class A, class ... args>
      struct BuildVector {
	static void doIt (std::vector<MiniMC::CPA::State_ptr>& vec,const State<statesize>& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	  vec.push_back (A::Transfer::doTransfer (s.template get<i> (),e,id));
	  BuildVector<i+1,statesize,args...>::doIt (vec,s,e,id);
	}
      };

      template<size_t i, size_t statesize, class A>
      struct BuildVector<i,statesize,A> {
	static void doIt (std::vector<MiniMC::CPA::State_ptr>& vec,const State<statesize>& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	  vec.push_back (A::Transfer::doTransfer (s.template get<i> (),e,id));
	}
	};*/

      struct Transferer : public MiniMC::CPA::Transferer {
        Transferer(std::vector<MiniMC::CPA::Transferer_ptr> pts) : transfers(pts) {}
        /*template<size_t i, class Inserter>
	  static bool buildVector (Inserter& ins, const State<sizeof...(args)>& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	  static_assert(i <= sizeof... (args));
	  if constexpr (i == sizeof... (args)) {
	    return true; //We are done
	  }
	  else {
	    using A = typename GetNthTemplateArgument<i,args...>::Temp;
	    auto res = A::Transfer::doTransfer (s.template get<i> (),e,id);
	    if (res) {
	      ins = res;
	      return buildVector<i+1,Inserter> (ins,s,e,id);
	    }
	    else
	      return false;
	  }
		
	}
	*/
        State_ptr doTransfer(const State_ptr& a, const MiniMC::Model::Edge_ptr& e, proc_id id) {
          auto s = static_cast<State&>(*a);
          auto n = transfers.size();
          std::vector<MiniMC::CPA::State_ptr> vec;
          for (size_t i = 0; i < n; i++) {
            auto res = transfers[i]->doTransfer(s.get(i), e, id);
            if (!res) {
              return nullptr;
            }
            vec.push_back(res);
          }
          return std::make_shared<State>(vec);
        }

        std::vector<MiniMC::CPA::Transferer_ptr> transfers;
      };

      struct Joiner : public MiniMC::CPA::Joiner {
        Joiner(std::vector<MiniMC::CPA::Joiner_ptr> pts) : joiners(pts) {}

        State_ptr doJoin(const State_ptr& l, const State_ptr& r) {
          auto left = static_cast<State&>(*l);
          auto right = static_cast<State&>(*r);
          auto n = joiners.size();
          std::vector<MiniMC::CPA::State_ptr> vec;
          for (size_t i = 0; i < n; i++) {
            auto res = joiners[i]->doJoin(left.get(i), right.get(i));
            if (!res) {
              return nullptr;
            }
            vec.push_back(res);
          }
          return std::make_shared<State>(vec);
        }

        /*template<size_t i>
	static bool checkCovers (const State<sizeof...(Args)>& l, const State<sizeof...(Args)>& r) {
	  static_assert(i <= sizeof... (Args));
	  if constexpr (i == sizeof... (Args)) {
	    return true; //We are done
	  }
	  else {
	    using A = typename GetNthTemplateArgument<i,Args...>::Temp;
	    auto left = l.template get<i> ();
	    auto right = r.template get<i> ();
	    
			
	    if (A::Join::covers (left,right)) {
	      return checkCovers<i+1> (l,r);
	    }
	    else {
	      return false;
	    }
	  }
	  }*/

        bool covers(const State_ptr& l, const State_ptr& r) {
          /*auto left = static_cast<State<sizeof... (Args)>&> (*l);
	  auto right = static_cast<State<sizeof... (Args)>&> (*r);
	  return checkCovers<0> (left,right);*/
          auto left = static_cast<State&>(*l);
          auto right = static_cast<State&>(*r);
          auto n = joiners.size();
          for (size_t i = 0; i < n; i++) {
            auto res = joiners[i]->covers(left.get(i), right.get(i));
            if (!res) {
              return false;
            }
          }
          return true;
        }

      private:
        std::vector<MiniMC::CPA::Joiner_ptr> joiners;
      };

      struct PreValidateSetup : public MiniMC::CPA::PrevalidateSetup {

        bool validate(const MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {
          /*A::PreValidate::validate (prgm,mess);
	    return PreValidateSetup<CPAs...>::validate (prgm,mess);*/
          return true;
        }
      };

      /*template<size_t ask,class... CPAs>
      struct CPADef {
	using Query = StateQuery<ask,CPAs...>;
	using Transfer = Transferer<CPAs...>;
	using Join = Joiner<CPAs...>;
	using Storage = Storer<Join>;
	using PreValidate = PreValidateSetup<CPAs...>;
	};*/

      struct CPA : public MiniMC::CPA::ICPA {
        CPA(std::vector<MiniMC::CPA::CPA_ptr>& p) : cpas(p) {
        }

        CPA(std::initializer_list<MiniMC::CPA::CPA_ptr> p) {
          std::copy(p.begin(), p.end(), std::back_inserter(cpas));
        }
        virtual MiniMC::CPA::StateQuery_ptr makeQuery() const {
          std::vector<StateQuery_ptr> queries;
          std::for_each(cpas.begin(), cpas.end(), [&queries](auto& it) { queries.push_back(it->makeQuery()); });
          return std::make_shared<StateQuery>(queries);
        }
        virtual MiniMC::CPA::Transferer_ptr makeTransfer() const {
          std::vector<Transferer_ptr> transfers;
          std::for_each(cpas.begin(), cpas.end(), [&transfers](auto& it) { transfers.push_back(it->makeTransfer()); });
          return std::make_shared<Transferer>(transfers);
        }
        virtual MiniMC::CPA::Joiner_ptr makeJoin() const {
          std::vector<Joiner_ptr> joiners;
          std::for_each(cpas.begin(), cpas.end(), [&joiners](auto& it) { joiners.push_back(it->makeJoin()); });
          return std::make_shared<Joiner>(joiners);
        }

        virtual MiniMC::CPA::Storer_ptr makeStore() const {
          return std::make_shared<MiniMC::CPA::Storer>(makeJoin());
        }

        virtual MiniMC::CPA::PrevalidateSetup_ptr makeValidate() const {
          return std::make_shared<PreValidateSetup>();
        }

      private:
        std::vector<MiniMC::CPA::CPA_ptr> cpas;
      };

    } // namespace Compounds
  }   // namespace CPA
} // namespace MiniMC

#endif
