#include "model/cfg.hpp"
#include "model/modifications/constantfolding.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/insertboolcasts.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/lower_guards.hpp"
#include "model/modifications/markinglooplocations.hpp"
#include "model/modifications/removealloca.hpp"
#include "model/modifications/removephi.hpp"
#include "model/modifications/replacememnondet.hpp"
#include "model/modifications/replacenondetuniform.hpp"
#include "model/modifications/replacesub.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/simplify_cfg.hpp"
#include "model/modifications/splitasserts.hpp"
#include "model/modifications/splitcmps.hpp"

#include "model/checkers/structure.hpp"
#include "model/checkers/typechecker.hpp"

#include "model/controller.hpp"

namespace MiniMC {
  namespace Model {
    bool Controller::typecheck () {
      return MiniMC::Model::Checkers::TypeChecker{}.run (*prgm);
    }
    
    bool Controller::structuralcheck () {
      return MiniMC::Model::Checkers::StructureChecker{}.run (*prgm);
    }

    void Controller::removePhi (){
      MiniMC::Model::Modifications::RemovePhi{}.run (*prgm);
    }

    void Controller::boolCasts (){
      MiniMC::Model::Modifications::InsertBoolCasts{}.run (*prgm);
    }

    void Controller::makeMemNonDet (){
      MiniMC::Model::Modifications::RemoveMemNondet{}.run (*prgm);
    }
    void Controller::createAssertViolateLocations (){
      MiniMC::Model::Modifications::SplitAsserts{}.run (*prgm);
    }
    
    void Controller::inlineFunctions (std::size_t depth, const MiniMC::Model::Function_ptr& func){
      MiniMC::Model::Modifications::InlineFunctions{depth}.runFunction (func);
    }
    void Controller::unrollLoops (std::size_t iterations,  const MiniMC::Model::Function_ptr& func){
      MiniMC::Model::Modifications::UnrollLoops{iterations}.runFunction (func);
    }
    void Controller::splitAtCMP (){
      MiniMC::Model::Modifications::EnsureEdgesOnlyHasOneCompar{}.run (*prgm);
      MiniMC::Model::Modifications::SplitCompares{}.run(*prgm);
    }

    void Controller::lowerGuards (){
      MiniMC::Model::Modifications::LowerGuards{}.run (*prgm) ;
    }
    
    void Controller::expandNonDet (){
      MiniMC::Model::Modifications::ExpandNondet{}.run (*prgm) ;
    
    }
    void Controller::simplifyCFA (){
      MiniMC::Model::Modifications::SimplifyCFG{}.run (*prgm);
    }
    
    void Controller::onlyOneMemAccessPerEdge (){
      MiniMC::Model::Modifications::EnsureEdgesOnlyHasOneMemAccess {}.run (*prgm);
    }
    
    void Controller::markLoopLocations (){
      MiniMC::Model::Modifications::MarkLoopStates{}.run (*prgm);
    }

    void Controller::makeLoopAllLocations (){
      MiniMC::Model::Modifications::MarkAllStates{}.run (*prgm);
    
    }
      
    void Controller::addEntryPoint (std::string& name, std::vector<MiniMC::Model::Value_ptr>&&) {
      auto func = prgm->getFunction(name);
      auto entry = createEntryPoint(prgm, func);
      prgm->addEntryPoint(entry->getName());
    }
    
  }
} // namespace MiniMC