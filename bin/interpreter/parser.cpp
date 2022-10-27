#include "parser.hpp"
#include "lexer.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include <stdlib.h>

namespace MiniMC {
namespace Interpreter {

std::string Parser::command() {
  switch (lexer->token()) {
  case Token::PRINT:
    return print();
  case Token::EDGES:
    return edges();
  case Token::JUMP:
    return jump();
  case Token::STEP:
    return step();
  case Token::BOOKMARK:
    return bookmark();
  case Token::HELP:
    return help();
  default:
    return nonrecognizable();
  }
}

std::string Parser::print() {
  std::string value = get_id();
  std::stringstream ss;
  if(value == ""){
    value = "current";
  }
  ss << (*statemap)[value];
  return ss.str();
}

std::string Parser::edges(){
  MiniMC::CPA::AnalysisState state = statemap->get("current");
  int n = 0;
  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  std::stringstream ss;

  // Print outgoing edges
  if (enumerator.getNext(res)) {
    n++;
    ss << n << ". " << std::endl;
    ss << *res.edge;
    while (enumerator.getNext(res)) {
      n++;
      ss << n << ". " << std::endl;
      ss << *res.edge;
    }
  }
  return ss.str();
}

std::string Parser::jump() {
  std::string value = get_id();

  if(value == ""){
    value = "bookmark";
  }
  if (statemap->contains(value)) {
    statemap->set("current", value);
  }
  return "";
}


std::string Parser::step() {
  CPA::AnalysisState newstate;
  MiniMC::proc_t proc{0};
  statemap->set("prev","current");

  std::stringstream ss("");

  if (auto edge = get_edge(get_nr())) {
    if (transfer.Transfer(statemap->get("current"), edge, proc, newstate)) {
      statemap->set("current",newstate);
    };
    if (auto noinsedge = haveNoInstructionEdge(statemap->get("current"))) {
      if (transfer.Transfer(statemap->get("current"), noinsedge, proc, newstate)) {
        statemap->set("current",newstate);
      };
    }
  } else {
    ss << "Either does the current location have no outgoing edges "
                 << "or the chosen index is invalid. Show valid edges by"
                 << "entering: 'edges'" << std::endl;
  }
  return ss.str();
}

std::string Parser::bookmark() {
  std::string value = get_id();

  if(value == ""){
    value = "current";
  }
  statemap->set("bookmark",value);
  return "";
}

std::string Parser::help(){
  std::stringstream ss;
  std::for_each(lexer->commandMap.begin(), lexer->commandMap.end(), [&ss](auto i){ss << i.first << std::endl;});

  return ss.str();
}

std::string Parser::nonrecognizable() {
  return "The command is not recognised. Try typing 'help', inorder to get a list of recognised commands\n";
}

std::string Parser::get_id() {
  lexer->advance();
  switch (lexer->token()) {
  case Token::ID:
    return lexer->getTokenAsText();
  default:
    return "";
  }
}

int Parser::get_nr(){
  lexer->advance();
  switch (lexer->token()) {
  case Token::NUMBER:
    return stoi(lexer->getTokenAsText());
  default:
    return -1;
  }
}

Model::Edge* Parser::haveNoInstructionEdge(CPA::AnalysisState state) {
  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  while (enumerator.getNext(res)) {
    if(!res.edge->getInstructions()) return res.edge;
  }
  return nullptr;
}

Model::Edge *Parser::get_edge(int i) {
  if (i == -1){
    return nullptr;
  }

  std::vector<Model::Edge *> edges;
  CPA::AnalysisState state = statemap->get("current");

  Algorithms::EdgeEnumerator enumerator{state};
  Algorithms::EnumResult res;

  // Print outgoing edges
  while (enumerator.getNext(res)) {
    edges.push_back(res.edge);
  }

  if (edges.size() == 0) {
    return nullptr;
  }

 if (edges[i - 1]) {
   return edges[i - 1];
 }
  return nullptr;
}

}// namespace Interpreter
} // namespace MiniMC
