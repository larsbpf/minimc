#ifndef MINIMC_PARSER_HPP
#define MINIMC_PARSER_HPP

#include "cpa/interface.hpp"
#include "lexer.hpp"
#include "interpreter/statemap.hpp"
#include <string>
namespace MiniMC {
namespace Interpreter {

class Parser {
public:
  Parser(StateMap *statemap,
         CPA::AnalysisTransfer transfer)
      : statemap(statemap), transfer(transfer){};

  std::string operator()(const std::string& s)
  {
    std::string ret;
    std::istringstream ist{s};
    lexer = new Lexer{ist};
    ret = command();
    delete lexer;

    return ret;
  }

private:
  Lexer *lexer;
  StateMap *statemap;
  CPA::AnalysisTransfer transfer;

  std::string print();
  std::string edges();
  std::string jump();
  std::string step();
  std::string bookmark();
  std::string nonrecognizable();
  std::string help();
  std::string get_id();
  int get_nr();

  std::string command();

  MiniMC::Model::Edge* get_edge(int i);
  Model::Edge* haveNoInstructionEdge(CPA::AnalysisState);
};

} // namespace Interpreter
} // namespace MiniMC

#endif // MINIMC_PARSER_HPP
