#include "loaders/loader.hpp"
#include "model/controller.hpp"
#include "plugin.hpp"
#include "support/localisation.hpp"
#include "support/smt.hpp"
#include <boost/program_options.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC
#include "cpa/pathformula.hpp"
#endif
#include "cpa/compound.hpp"
#include "cpa/concrete.hpp"
#include "options.hpp"

namespace po = boost::program_options;



int main(int argc, char* argv[]) {
  
  std::string input;
  std::string subcommand;
  MiniMC::Support::setMessageSink(MiniMC::Support::MessagerType::Terminal);
  MiniMC::Support::Messager messager;
  
  SetupOptions options;
  bool ok = parseOptions(argc, argv, options);

  if (ok) {
  // Load Program
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64>();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64>();
  MiniMC::Model::Program_ptr prgm = MiniMC::Loaders::loadFromFile<MiniMC::Loaders::Type::LLVM>(options.load.inputname, typename MiniMC::Loaders::OptionsLoad<MiniMC::Loaders::Type::LLVM>::Opt{.tfactory = tfac,
                                                                                                                                                                                               .cfactory = cfac});

  MiniMC::Model::Controller control(*prgm);
  control.boolCasts();
  control.makeLoopAllLocations();
  control.createAssertViolateLocations();
  control.lowerGuards();

  try {
    if (options.load.tasks.size()) {
      for (std::string& s : options.load.tasks) {
	try {
	  control.addEntryPoint(s, {});
	} catch (MiniMC::Support::FunctionDoesNotExist&) {
	  messager.message<MiniMC::Support::Severity::Error> (MiniMC::Support::Localiser{"Function '%1%' specicifed as entry point does not exists. "}.format(s));
	  return -1;
	}
      }
    }
    else {
      messager.message<MiniMC::Support::Severity::Error> ("At least one entry point must be specified");
      return -1;
    }
    if (options.command) {
      return static_cast<int>(options.command->getFunction()(control,options.cpa));
    }
    
    else {
      messager.message<MiniMC::Support::Severity::Error> ("No subcommand selected");
      
      return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError);
    }
  }
  catch (MiniMC::Support::ConfigurationException& e) {
    messager.message<MiniMC::Support::Severity::Error> (e.what ());
    return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError); 
  }
  catch (MiniMC::Support::Exception& e) {
    messager.message<MiniMC::Support::Severity::Error> (e.what ());
    return static_cast<int>(MiniMC::Support::ExitCodes::RuntimeError); 
  }
  }
  
}
