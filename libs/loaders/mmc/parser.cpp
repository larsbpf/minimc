#include "parser.hpp"
#include "model/cfg.hpp"
#include "model/builder.hpp"
#include "parserexception.hpp"

#include "support/feedback.hpp"


namespace MiniMC{
namespace Loaders{

void Parser::ignore_eol() {
  while(lexer->token() == Token::EOL_TOKEN) lexer->advance();
}

void Parser::run() {
  lexer = new Lexer{*in};
  try {
    functions();
    entrypoints();
    heap();
    initialiser();
  } catch (MMCParserException const& e) {
    Support::Messager messager;

        messager.message<MiniMC::Support::Severity::Error>(
            Support::Localiser("\n%4% \n"
                               "Pos: %1% \n"
                               "Line: %2% \n"
                               "With Token Value: '%3%' \n")
            .format(e.getPos(), e.getLine(), e.getValue(), e.getMesg()));
  }
  delete lexer;
}

void Parser::functions() {
  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::FUNCTIONS) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::HASH_SIGN) {
        function();
      }
      return;
    }
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a functions-title on the from '# Functions'");
}

void Parser::entrypoints() {
  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::ENTRYPOINTS) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::HASH_SIGN) {
        prgm->addEntryPoint(identifier());
        lexer->advance();
        ignore_eol();
      }
      return;
    }
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a entrypoints-title on the from '# Entrypoints'");
}

void Parser::heap() {
  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::HEAP) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::HASH_SIGN) {
        if (lexer->token() == Token::DIGIT) {
          lexer->advance();
          if (lexer->token() == Token::COLON) {
            lexer->advance();
            if (lexer->token() == Token::DIGIT) {
              prgm->getHeapLayout().addBlock(stoi(lexer->getValue()));
              lexer->advance();
              ignore_eol();
              return;
            }
          }
        }
      }
    }
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a heap-title on the form '# Heap'");
}

void Parser::initialiser() {
  Model::InstructionStream instructionStream;
  std::vector<MiniMC::Model::Register_ptr> variables;

  if (lexer->token() == Token::HASH_SIGN) {
    lexer->advance();
    if (lexer->token() == Token::INITIALISER) {
      lexer->advance();
      ignore_eol();
      while (lexer->token() != Token::EOF_TOKEN) {
        instruction(&instructionStream, variables);
        lexer->advance();
        ignore_eol();
      }
      prgm->setInitialiser(instructionStream);
      return;
    }
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a initialiser-segment on the form '# Initialiser'");
}
void Parser::function() {
  std::vector<MiniMC::Model::Register_ptr> params;

  if (lexer->token() == Token::HASHHASH_SIGN) {
    lexer->advance();
    std::string name = identifier();
    lexer->advance();
    ignore_eol();
    auto registerDescr = registers(name);

    parameters(&params, registerDescr.get());
    auto ret = returns();
    auto cfg = cfa(name, registerDescr.get());
    prgm->addFunction(name, params, ret,
                      std::move(registerDescr), std::move(cfg));
    return;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a function-declaration beginning with '## {function-name}'");
}

Model::RegisterDescr_uptr Parser::registers(std::string name){
  auto registerDescr = std::make_unique<MiniMC::Model::RegisterDescr> (name);

  if(lexer->token() == Token::REGISTERS){
    lexer->advance();
    ignore_eol();
    while(lexer->token() != Token::PARAMETERS){
      auto v = variable();
      registerDescr->addRegister(v.getName(), v.getType());
      lexer->advance();
      ignore_eol();
    }
    return registerDescr;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a registers-declaration on the form: '.registers'");
}

void Parser::parameters(std::vector<MiniMC::Model::Register_ptr>* params, const MiniMC::Model::RegisterDescr* regs) {
  std::vector<Model::Register_ptr> variables = regs->getRegisters();

  if(lexer->token() == Token::PARAMETERS){
    lexer->advance();
    ignore_eol();
    while(lexer->token() != Token::RETURNS){
      std::for_each(variables.begin(),variables.end(), [params,regs,this](Model::Register_ptr reg){
        if(reg->getName() ==  regs->getPref() + ":" + identifier()){
          params->push_back(reg);
        }
      });
      lexer->advance();
      ignore_eol();
    }
    return;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a parameteres-declaration on the form: '.parameters'");
}

Model::Type_ptr Parser::returns(){
  if(lexer->token() == Token::RETURNS){
    lexer->advance();
    Model::Type_ptr t = type();
    lexer->advance();
    ignore_eol();
    return t;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a returns-declaration on the form: '.returns'");
}

Model::CFA Parser::cfa(std::string name, const MiniMC::Model::RegisterDescr* regs) {
  Model::CFA cfg;
  std::unordered_map<std::string, MiniMC::Model::Location_ptr> locmap;

  if (lexer->token() == Token::CFA) {
    lexer->advance();
    ignore_eol();
    while (lexer->token() != Token::HASHHASH_SIGN && lexer->token() != Token::HASH_SIGN) {
      edge(name, regs, &cfg, &locmap);
    }
    auto edges = cfg.getEdges();
    std::for_each(edges.begin(),edges.end(),[locmap,this](auto e){
      auto location = e->getTo();
      auto to = locmap.at(location->getInfo().getNameWithoutPref());
      e->setTo(to);
    });
    cfg.setInitial(locmap.at("Initial"));
    return cfg;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected a cfa-declaration on the form: '.cfa'");
}

void Parser::edge(std::string name, const MiniMC::Model::RegisterDescr* regs, Model::CFA* cfg, std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap) {
  Model::InstructionStream instructionStream;
  MiniMC::Model::LocationInfoCreator locinfoc(name, regs);
  Model::Location_ptr to;
  auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();

  Model::Location_ptr from = location(cfg,locmap,source_loc,locinfoc);
  lexer->advance();
  ignore_eol();
  if (lexer->token() == Token::L_BRACKET) {
    lexer->advance();
    ignore_eol();
    while (lexer->token() != Token::R_BRACKET) {
      if (lexer->token() == Token::R_ARROW) {
        if (locmap->contains (lexer->getValue())) {
          to = locmap->at(lexer->getValue());
        }
        else {
          auto location = cfg->makeLocation(locinfoc.make(lexer->getValue(), 0, *source_loc));
          (*locmap)[lexer->getValue()] = location;
          to = location;
        }
        auto edge = cfg->makeEdge(from,to);
        edge->getInstructions() = instructionStream;
        instructionStream.clear();
      } else {
        instruction(&instructionStream, regs->getRegisters());
      }
      lexer->advance();
      ignore_eol();
    }
    lexer->advance();
    ignore_eol();
    return;
  }
  throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(),
                           "Expected an edge on the form: \n"
                           "[\n"
                           "{INSTRUCTION1}\n"
                           "{INSTRUCTION2}\n"
                           "{...}\n"
                           "]'");
}

Model::Location_ptr Parser::location(Model::CFA* cfg,std::unordered_map<std::string, MiniMC::Model::Location_ptr>* locmap, std::shared_ptr<MiniMC::Model::SourceInfo> source_loc,MiniMC::Model::LocationInfoCreator locinfoc) {
  try {
    std::string index = identifier();
    lexer->advance();
    std::string name = identifier();

    if (locmap->contains(index)) {
      return locmap->at(index);
    } else {
      auto location = cfg->makeLocation(locinfoc.make(name, 0, *source_loc));
      if (locmap->size() == 0) {
        (*locmap)["Initial"] = location;
      }
      (*locmap)[index] = location;
      return location;
    }
  } catch (MMCParserException const& e){
    Support::Messager messager;
    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a location, which is "
        "on the form: {INDEX_NAME} {LOCATION_NAME}");
  }
}

void Parser::instruction(Model::InstructionStream* instructionStream, std::vector<MiniMC::Model::Register_ptr> variables) {
  try {
    switch (lexer->token()) {
    case Token::INSTR_Skip:
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Skip>({}));
      return;
    case Token::INSTR_Uniform:
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Uniform>({}));
      return;
    case Token::INSTR_RetVoid:
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::RetVoid>({}));
      return;
    case Token::INSTR_Ret:
      lexer->advance();
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Ret>(
              {.value = value(variables)}));
      return;
    case Token::INSTR_PRED_ICMP_SGT:
    case Token::INSTR_PRED_ICMP_UGT:
    case Token::INSTR_PRED_ICMP_SGE:
    case Token::INSTR_PRED_ICMP_UGE:
    case Token::INSTR_PRED_ICMP_SLT:
    case Token::INSTR_PRED_ICMP_ULT:
    case Token::INSTR_PRED_ICMP_SLE:
    case Token::INSTR_PRED_ICMP_ULE:
    case Token::INSTR_PRED_ICMP_EQ:
    case Token::INSTR_PRED_ICMP_NEQ:
      instructionStream->addInstruction(predicates(variables));
      return;
    case Token::INSTR_Assert:
      lexer->advance();
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Assert>(
              {.expr = value(variables)}));
      return;
    case Token::INSTR_Assume:
      lexer->advance();
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Assume>(
              {.expr = value(variables)}));
      return;
    case Token::INSTR_NegAssume:
      lexer->advance();
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::NegAssume>(
              {.expr = value(variables)}));
      return;
    case Token::INSTR_Store: {
      lexer->advance();
      Model::Value_ptr addr = value(variables);
      lexer->advance();
      Model::Value_ptr storee = value(variables);
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Store>(
              {.addr = addr, .storee = storee}));
      return;
    }
    case Token::INSTR_Call: {
      lexer->advance();
      Model::Value_ptr function = value(variables);
      lexer->advance();
      instructionStream->addInstruction(
          Model::createInstruction<Model::InstructionCode::Call>(
              {.res = nullptr,
               .function = function,
               .params = value_list(variables)}));
      return;
    }
    case Token::LESS_THAN:
      instructionStream->addInstruction(instrres(variables));
      return;
    default:
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(),lexer->getValue(),
          "The first token of the instruction is not recognised");
    }
  } catch (MMCParserException const& e) {
    Support::Messager messager;
    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "The parser doesn't recognise this instruction. The instruction "
        "might not be supported. Otherwise you might want to check the "
        "structure of the instruction again");
  }
}

Model::Instruction Parser::instrres(const std::vector<MiniMC::Model::Register_ptr> variables) {
  if (lexer->token() == Token::LESS_THAN) {
    Model::Value_ptr res = value(variables);
    lexer->advance();
    if (lexer->token() == Token::INSTR_PtrAdd) {
      lexer->advance();
      Model::Value_ptr ptr = value(variables);
      lexer->advance();
      Model::Value_ptr skipsize = value(variables);
      lexer->advance();
      Model::Value_ptr nbSkips = value(variables);
      return Model::createInstruction<Model::InstructionCode::PtrAdd>(
          {.res = res, .ptr = ptr, .skipsize = skipsize, .nbSkips = nbSkips});
    } else if (lexer->token() == Token::EQUAL_SIGN) {
      return instreseq(variables, res);
    }
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "The 2nd or 3rd token of the instruction is not recognised. "
      "It is either a variable or 'PtrAdd' followed by '='");
}

Model::Instruction Parser::instreseq(const std::vector<MiniMC::Model::Register_ptr> variables, Model::Value_ptr res) {
 try {
   if (lexer->token() == Token::EQUAL_SIGN) {
     lexer->advance();
     switch (lexer->token()) {
     case Token::INSTR_Add:
     case Token::INSTR_Sub:
     case Token::INSTR_Mul:
     case Token::INSTR_UDiv:
     case Token::INSTR_SDiv:
     case Token::INSTR_Shl:
     case Token::INSTR_LShr:
     case Token::INSTR_AShr:
     case Token::INSTR_And:
     case Token::INSTR_Or:
     case Token::INSTR_Xor:
       return tacops(variables, res);
     case Token::INSTR_ICMP_SGT:
     case Token::INSTR_ICMP_UGT:
     case Token::INSTR_ICMP_SGE:
     case Token::INSTR_ICMP_UGE:
     case Token::INSTR_ICMP_SLT:
     case Token::INSTR_ICMP_ULT:
     case Token::INSTR_ICMP_SLE:
     case Token::INSTR_ICMP_ULE:
     case Token::INSTR_ICMP_EQ:
       return comparison(variables, res);
     case Token::INSTR_ICMP_NEQ:
     case Token::INSTR_Not:
       lexer->advance();
       return Model::createInstruction<Model::InstructionCode::Not>(
           {.res = res, .op1 = value(variables)});
     case Token::INSTR_Trunc:
     case Token::INSTR_ZExt:
     case Token::INSTR_SExt:
     case Token::INSTR_PtrToInt:
     case Token::INSTR_IntToPtr:
     case Token::INSTR_BitCast:
     case Token::INSTR_BoolZExt:
     case Token::INSTR_BoolSExt:
     case Token::INSTR_IntToBool:
       return castops(variables, res);
     case Token::INSTR_NonDet:
       return nondet(variables, res);
     case Token::INSTR_Call: {
       lexer->advance();
       Model::Value_ptr function = value(variables);
       lexer->advance();
       return Model::createInstruction<Model::InstructionCode::Call>(
           {.res = res,
            .function = function,
            .params = value_list(variables)});
     }
     case Token::INSTR_ExtractValue:
       return extract(variables, res);
     case Token::INSTR_InsertValue:
       return insert(variables, res);
     case Token::INSTR_Load:
       return load(variables, res);
     case Token::LESS_THAN:
       return Model::createInstruction<Model::InstructionCode::Assign>(
           {.res = res, .op1 = value(variables)});
     default:
       throw MMCParserException(
           lexer->getLine(), lexer->getPos(), lexer->getValue(),
           "The token does not match any know/supported instruction.");
     }
   }
 } catch (MMCParserException const& e){
   Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "The 3rd< token of the instruction is not recognised.");
 }
}


Model::Instruction Parser::predicates(const std::vector<MiniMC::Model::Register_ptr> variables) {
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    lexer->advance();
    Model::Value_ptr op2 = value(variables);
    switch (token) {
    case Token::INSTR_PRED_ICMP_SGT:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_SGT>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_UGT:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_UGT>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_SGE:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_SGE>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_UGE:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_UGE>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_SLT:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_SLT>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_ULT:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_ULT>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_SLE:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_SLE>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_ULE:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_ULE>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_EQ:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_EQ>(
          {.op1 = op1, .op2 = op2});
    case Token::INSTR_PRED_ICMP_NEQ:
      return Model::createInstruction<Model::InstructionCode::PRED_ICMP_NEQ>(
          {.op1 = op1, .op2 = op2});
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e){
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a predicate instruction on the form: "
        "{PRED_INSTRUCTION} {OP1} {OP2}");
  }
}

Model::Instruction Parser::tacops(const std::vector<MiniMC::Model::Register_ptr> variables, Model::Value_ptr res){
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    lexer->advance();
    Model::Value_ptr op2 = value(variables);

    switch (token) {
    case Token::INSTR_Add:
      return Model::createInstruction<Model::InstructionCode::Add>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_Sub:
      return Model::createInstruction<Model::InstructionCode::Sub>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_Mul:
      return Model::createInstruction<Model::InstructionCode::Mul>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_UDiv:
      return Model::createInstruction<Model::InstructionCode::UDiv>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_SDiv:
      return Model::createInstruction<Model::InstructionCode::SDiv>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_Shl:
      return Model::createInstruction<Model::InstructionCode::Shl>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_LShr:
      return Model::createInstruction<Model::InstructionCode::LShr>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_AShr:
      return Model::createInstruction<Model::InstructionCode::AShr>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_And:
      return Model::createInstruction<Model::InstructionCode::And>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_Or:
      return Model::createInstruction<Model::InstructionCode::Or>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_Xor:
      return Model::createInstruction<Model::InstructionCode::Xor>(
          {.res = res, .op1 = op1, .op2 = op2});
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e){
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a TAC-instruction expression on the form: "
        "{RES} = {INSTRUCTION} {OP1} {OP2}");
  }
}

Model::Instruction Parser::comparison(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res) {
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    lexer->advance();
    Model::Value_ptr op2 = value(variables);
    switch (token) {
    case Token::INSTR_ICMP_SGT:
      return Model::createInstruction<Model::InstructionCode::ICMP_SGT>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_UGT:
      return Model::createInstruction<Model::InstructionCode::ICMP_UGT>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_SGE:
      return Model::createInstruction<Model::InstructionCode::ICMP_SGE>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_UGE:
      return Model::createInstruction<Model::InstructionCode::ICMP_UGE>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_SLT:
      return Model::createInstruction<Model::InstructionCode::ICMP_SLT>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_ULT:
      return Model::createInstruction<Model::InstructionCode::ICMP_ULT>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_SLE:
      return Model::createInstruction<Model::InstructionCode::ICMP_SLE>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_ULE:
      return Model::createInstruction<Model::InstructionCode::ICMP_ULE>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_EQ:
      return Model::createInstruction<Model::InstructionCode::ICMP_EQ>(
          {.res = res, .op1 = op1, .op2 = op2});
    case Token::INSTR_ICMP_NEQ:
      return Model::createInstruction<Model::InstructionCode::ICMP_NEQ>(
          {.res = res, .op1 = op1, .op2 = op2});
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e){
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a comparison instruction on the form: "
        "{RES} = {INSTRUCTION} {OP1} {OP2}");
  }
}

Model::Instruction Parser::castops(const std::vector<MiniMC::Model::Register_ptr> variables, Model::Value_ptr res) {
  try {
    Token token = lexer->token();
    lexer->advance();
    Model::Type_ptr t = type();
    lexer->advance();
    Model::Value_ptr op1 = value(variables);
    switch (token) {
    case Token::INSTR_Trunc:
      return Model::createInstruction<Model::InstructionCode::Trunc>(
          {.res = res, .op1 = op1});
    case Token::INSTR_ZExt:
      return Model::createInstruction<Model::InstructionCode::ZExt>(
          {.res = res, .op1 = op1});
    case Token::INSTR_SExt:
      return Model::createInstruction<Model::InstructionCode::SExt>(
          {.res = res, .op1 = op1});
    case Token::INSTR_PtrToInt:
      return Model::createInstruction<Model::InstructionCode::PtrToInt>(
          {.res = res, .op1 = op1});
    case Token::INSTR_IntToPtr:
      return Model::createInstruction<Model::InstructionCode::IntToPtr>(
          {.res = res, .op1 = op1});
    case Token::INSTR_BitCast:
      return Model::createInstruction<Model::InstructionCode::BitCast>(
          {.res = res, .op1 = op1});
    case Token::INSTR_BoolZExt:
      return Model::createInstruction<Model::InstructionCode::BoolZExt>(
          {.res = res, .op1 = op1});
    case Token::INSTR_BoolSExt:
      return Model::createInstruction<Model::InstructionCode::BoolSExt>(
          {.res = res, .op1 = op1});
    case Token::INSTR_IntToBool:
      return Model::createInstruction<Model::InstructionCode::IntToBool>(
          {.res = res, .op1 = op1});
    default:
      __builtin_unreachable();
    }
  } catch (MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(lexer->getLine(), lexer->getPos(),
                             lexer->getValue(),
                             "Expected a comparison instruction on the form: "
                             "{RES} = {INSTRUCTION} {TYPE} {OP1}");
  }
}

Model::Instruction Parser::nondet(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::INSTR_NonDet) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr min = value(variables);
      lexer->advance();
      Model::Value_ptr max = value(variables);
      return Model::createInstruction<Model::InstructionCode::NonDet>(
          {.res = res, .min = min, .max = max});
    }
    __builtin_unreachable();
  } catch(MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(lexer->getLine(), lexer->getPos(),
                             lexer->getValue(),
                             "Expected a NonDet instruction on the form: "
                             "{RES} = {INSTRUCTION} {TYPE} {MIN} {MAX}");
  }
}

Model::Instruction Parser::extract(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::INSTR_ExtractValue) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr aggregate = value(variables);
      lexer->advance();
      Model::Value_ptr offset = value(variables);
      return Model::createInstruction<Model::InstructionCode::ExtractValue>(
          {.res = res, .aggregate = aggregate, .offset = offset});
    }
    __builtin_unreachable();
  }  catch(MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(),lexer->getValue(),
      "Expected a ExtractValue instruction on the form: "
      "{RES} = ExtractValue {TYPE} {AGGREGATE} {OFFSET}");

}

Model::Instruction Parser::insert(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::INSTR_InsertValue) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr aggregate = value(variables);
      lexer->advance();
      Model::Value_ptr offset = value(variables);
      lexer->advance();
      Model::Value_ptr insertee = value(variables);
      return Model::createInstruction<Model::InstructionCode::InsertValue>(
          {.res = res,
           .aggregate = aggregate,
           .offset = offset,
           .insertee = insertee});
    }
    __builtin_unreachable();
  }  catch(MMCParserException const& e) {
    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a InsertValue instruction on the form: "
        "{RES} = InsertValue {TYPE} {AGGREGATE} {OFFSET} {INSERTEE}");
  }
}

Model::Instruction Parser::load(const std::vector<MiniMC::Model::Register_ptr> variables,Model::Value_ptr res){
  try {
    if (lexer->token() == Token::INSTR_Load) {
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      Model::Value_ptr addr = value(variables);
      return Model::createInstruction<Model::InstructionCode::Load>(
          {.res = res, .addr = addr});
    }
    __builtin_unreachable();
  } catch (MMCParserException const& e){

    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a InsertValue instruction on the form: "
        "{RES} = Load {TYPE} {ADDRESS}");
  }
}


Model::Type_ptr Parser::type(){
  switch(lexer->token()){
  case Token::TYPE_Void:
    return tfac.makeVoidType();
  case Token::TYPE_Bool:
    return tfac.makeBoolType();
  case Token::TYPE_I8:
    return tfac.makeIntegerType(8);
  case Token::TYPE_I16:
    return tfac.makeIntegerType(16);
  case Token::TYPE_I32:
    return tfac.makeIntegerType(32);
  case Token::TYPE_I64:
    return tfac.makeIntegerType(64);
  case Token::TYPE_Float:
    return tfac.makeFloatType();
  case Token::TYPE_Double:
    return tfac.makeDoubleType();
  case Token::TYPE_Pointer:
    return tfac.makePointerType();
  case Token::TYPE_Struct:
    lexer->advance();
    return tfac.makeStructType(integer());
  case Token::TYPE_Array:
    lexer->advance();
    return tfac.makeArrayType(integer());
  default:
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a type.");
  }
}

std::vector<Model::Value_ptr> Parser::value_list(std::vector<MiniMC::Model::Register_ptr> variables){
  std::vector<Model::Value_ptr> list;
  while(lexer->token() != Token::EOL_TOKEN){
    list.push_back(value(variables));
    lexer->advance();
  }
  return list;
}


Model::Value_ptr Parser::value(std::vector<MiniMC::Model::Register_ptr> variables) {
  Model::Value_ptr ret;
  int value;
  std::vector<Model::Constant_ptr> blob;
  Token token;

  if (lexer->token() == Token::LESS_THAN) {
    lexer->advance();
    switch (lexer->token()) {
    case Token::IDENTIFIER: {
      std::string var = identifier();
      lexer->advance();
      Model::Type_ptr t = type();
      lexer->advance();
      if (lexer->token() != Token::GREATER_THAN)
        throw MMCParserException(lexer->getLine(), lexer->getPos(), lexer->getValue(), "");
      std::for_each(
          variables.begin(), variables.end(), [&ret, &t, &var ,this](auto rptr) {
            if (rptr->getName() == rptr->getOwner()->getPref() + ":" + var &&
                t->getTypeID() == rptr->getType()->getTypeID()) {
              if (!ret) {
                ret = rptr;
              } else
                throw MMCParserException(
                    lexer->getLine(), lexer->getPos(), lexer->getValue(),
                    "The identifier describes a NULL-pointer");
            }
          });
      return ret;
    }
    case Token::DOLLAR_SIGN: {
      lexer->advance();
      blob = integer_list();
      lexer->advance();
      if (lexer->token() != Token::DOLLAR_SIGN)
        throw MMCParserException(
            lexer->getLine(), lexer->getPos(), lexer->getValue(),
            "Expected a dollar-sign to enclose the list.");
    }
    case Token::HEAP_POINTER:
    case Token::FUNCTION_POINTER:
      token = lexer->token();
      lexer->advance();
      if (lexer->token() == Token::L_PARA) {
        lexer->advance();
        int base = integer();
        lexer->advance();
        int offset = integer();
        lexer->advance();
        if (lexer->token() != Token::R_PARA)
          throw MMCParserException(
              lexer->getLine(),lexer->getPos(), lexer->getValue(),
              "Expected a right parenthesis.");
        lexer->advance();
        type();
        lexer->advance();
        if (lexer->token() != Token::GREATER_THAN)
          throw MMCParserException(
              lexer->getLine(), lexer->getPos(), lexer->getValue(),
              "Expected a greater than.");
        if (token == Token::HEAP_POINTER)
          return cfac.makeHeapPointer(base);
        if (token == Token::FUNCTION_POINTER)
          return cfac.makeFunctionPointer(base);
      }
      break;
    case Token::DIGIT:
    case Token::HEX:
      value = integer();
      break;
    default:
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(), lexer->getValue(),
          "The token is not recognised as a part of a value. This might be a "
          "result to using a keyword as identifier.");
    }
    lexer->advance();
    Model::Type_ptr t = type();
    lexer->advance();
    if (lexer->token() != Token::GREATER_THAN)
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(), lexer->getValue(),
          "Expected a greater than.");
    switch (t->getTypeID()) {
    case Model::TypeID::Bool:
    case Model::TypeID::I8:
    case Model::TypeID::I16:
    case Model::TypeID::I32:
    case Model::TypeID::I64:
      return cfac.makeIntegerConstant(value, t->getTypeID());
    case Model::TypeID::Struct:
      return cfac.makeAggregateConstant(blob, false);
    case Model::TypeID::Array:
      return cfac.makeAggregateConstant(blob, true);
    default:
      throw MMCParserException(
          lexer->getLine(), lexer->getPos(), lexer->getValue(),
          "The type is not supported in the Parser right now");
    }
  }
  throw MMCParserException(
      lexer->getLine(),lexer->getPos(), lexer->getValue(),
      "Expected a value, which always starts with a less than.");
}

Model::Register Parser::variable(){
  try {
    if (lexer->token() == Token::LESS_THAN) {
      lexer->advance();
      Model::Register var = Model::Register(identifier(), nullptr);
      lexer->advance();
      var.setType(type());
      lexer->advance();
      if (lexer->token() == Token::GREATER_THAN) {
        return var;
      }
    }
  } catch (MMCParserException const& e){

    Support::Messager messager;

    messager.message<MiniMC::Support::Severity::Error>(
        Support::Localiser("\n%1% \n").format(e.getMesg()));
    throw MMCParserException(
        lexer->getLine(), lexer->getPos(), lexer->getValue(),
        "Expected a variable. A variable is of the form: "
        "<{IDENTIFIER} {TYPE}>");
  }
}

std::string Parser::identifier() {
  if(lexer->token() == Token::IDENTIFIER){
    return lexer->getValue();
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected a identifier. This can be caused of using a keyword as "
      "identifier which is not allowed in MiniMC.");
}

int Parser::integer(){
  std::string value = lexer->getValue();

  if(lexer->token() == Token::DIGIT){
    return std::stoi(value);
  } else if(lexer->token() == Token::HEX){
    return std::stoi(value, 0,16);
  }
  throw MMCParserException(
      lexer->getLine(), lexer->getPos(), lexer->getValue(),
      "Expected either a Hex(16) or Decimal(10) value.");
}

std::vector<Model::Constant_ptr> Parser::integer_list(){
  std::vector<Model::Constant_ptr> blob;
  while(lexer->token() == Token::HEX){
    Model::Constant_ptr cptr = std::make_shared<Model::TConstant<BV8>>(static_cast<MiniMC::BV8>(integer()));
    blob.push_back(cptr);
    lexer->advance();
  }
  return blob;
}

} // namespace Loader
} // namespace MiniMC