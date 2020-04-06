#include <unordered_map>
#include <boost/program_options.hpp>
#include "algorithms/algorithm.hpp"

#include "plugin.hpp"

namespace {
  auto& getMap () {
	static std::unordered_map<std::string,CommandRegistrar*> map;
	return map;
  }
}

void registerCommand (const std::string& s,CommandRegistrar& r) {
  getMap().insert(std::make_pair(s,&r));
}

bool isCommand (const std::string& s) {
  return getMap().count (s);
}

subcommandfunc getCommand (const std::string& s) {
  return getMap().at(s)->getFunction ();
}

bool parseOptionsAddHelp (boost::program_options::variables_map& vm, boost::program_options::options_description& desc, std::vector<std::string>& params) {
  bool help;
  desc.add_options()
    ("help,c",boost::program_options::bool_switch(&help), "Print Help Message");
  
  try {
    boost::program_options::store(boost::program_options::command_line_parser(params).
			  options(desc) 
			  .run(), vm);
	boost::program_options::notify (vm);
	
  }
  catch(boost::program_options::error& e) {
	if (help) {
	  std::cerr << desc << std::endl;
	}
	else 
	  std::cerr << e.what () << std::endl;
	
	return false;
  }

  if (help) {
	std::cerr << desc << std::endl;
	return false;
  }
  
  

  return true;
  

}


