#ifndef MINIMC_LEXER_HPP
#define MINIMC_LEXER_HPP
#include <string>
#include <istream>
#include <map>

namespace MiniMC {
namespace Loaders {

enum class Token {
  EOF_TOKEN,
  COLON,
  DIGIT,
  EQUAL_SIGN,
  FUNCTIONS,
  REGISTERS,
  PARAMETERS,
  RETURNS,
  CFA,
  IDENTIFIER,
  L_BRACKET,
  R_BRACKET,
  L_PARA,
  R_PARA,
  R_ARROW,
  GREATER_THAN,
  LESS_THAN,
  ENTRYPOINTS,
  HEAP,
  INITIALISER,
  HASH_SIGN,
  HASHHASH_SIGN,
  DOLLAR_SIGN,
  PLUS_SIGN,
  EOL_TOKEN,
  HEX,
  TYPE_Void,
  TYPE_Bool,
  TYPE_I8,
  TYPE_I16,
  TYPE_I32,
  TYPE_I64,
  TYPE_Float,
  TYPE_Double,
  TYPE_Pointer,
  FUNCTION_POINTER,
  HEAP_POINTER,
  TYPE_Struct,
  TYPE_Array,
  INSTR_Add,
  INSTR_Sub,
  INSTR_Mul,
  INSTR_UDiv,
  INSTR_SDiv,
  INSTR_Shl,
  INSTR_LShr,
  INSTR_AShr,
  INSTR_And,
  INSTR_Or,
  INSTR_Xor,
  INSTR_Not,
  INSTR_ICMP_SGT,
  INSTR_ICMP_UGT,
  INSTR_ICMP_SGE,
  INSTR_ICMP_UGE,
  INSTR_ICMP_SLT,
  INSTR_ICMP_ULT,
  INSTR_ICMP_SLE,
  INSTR_ICMP_ULE,
  INSTR_ICMP_EQ,
  INSTR_ICMP_NEQ,
  INSTR_PtrAdd,
  INSTR_PtrEq,
  INSTR_ExtractValue,
  INSTR_InsertValue,
  INSTR_Trunc,
  INSTR_ZExt,
  INSTR_SExt,
  INSTR_PtrToInt,
  INSTR_IntToPtr,
  INSTR_BitCast,
  INSTR_BoolZExt,
  INSTR_BoolSExt,
  INSTR_IntToBool,
  INSTR_Store,
  INSTR_Load,
  INSTR_Skip,
  INSTR_Call,
  INSTR_Assign,
  INSTR_Ret,
  INSTR_RetVoid,
  INSTR_NonDet,
  INSTR_Uniform,
  INSTR_Assert,
  INSTR_Assume,
  INSTR_NegAssume,
  INSTR_PRED_ICMP_SGT,
  INSTR_PRED_ICMP_UGT,
  INSTR_PRED_ICMP_SGE,
  INSTR_PRED_ICMP_UGE,
  INSTR_PRED_ICMP_SLT,
  INSTR_PRED_ICMP_ULT,
  INSTR_PRED_ICMP_SLE,
  INSTR_PRED_ICMP_ULE,
  INSTR_PRED_ICMP_EQ,
  INSTR_PRED_ICMP_NEQ,
};

class Lexer {
public:
  Lexer(std::istream &is) : in{&is} {
    advance();
  }

  Token token() const {return cur_token; }
  void advance() {cur_token = get_token(); }
  std::string getValue(){ return buffer;}

  std::map<std::string,Token> keywordsMap = {
      {"$", Token::DOLLAR_SIGN},
      {"=", Token::EQUAL_SIGN},
      {":", Token::COLON},
      {"Functions", Token::FUNCTIONS},
      {"0x", Token::HEX},
      {".registers", Token::REGISTERS},
      {".parameters", Token::PARAMETERS},
      {".returns", Token::RETURNS},
      {".cfa", Token::CFA},
      {"[", Token::L_BRACKET},
      {"]", Token::R_BRACKET},
      {"->", Token::R_ARROW},
      {"<", Token::LESS_THAN},
      {">", Token::GREATER_THAN},
      {"Entrypoints", Token::ENTRYPOINTS},
      {"Initialiser", Token::INITIALISER},
      {"Heap", Token::HEAP},
      {"#", Token::HASH_SIGN},
      {"##", Token::HASHHASH_SIGN},
      {"Void",Token::TYPE_Void},
      {"Bool",Token::TYPE_Bool},
      {"I8",Token::TYPE_I8},
      {"I16",Token::TYPE_I16},
      {"I32",Token::TYPE_I32},
      {"I64",Token::TYPE_I64},
      {"Int8",Token::TYPE_I8},
      {"Int16",Token::TYPE_I16},
      {"Int32",Token::TYPE_I32},
      {"Int64",Token::TYPE_I64},
      {"Float",Token::TYPE_Float},
      {"Double",Token::TYPE_Double},
      {"Pointer",Token::TYPE_Pointer},
      {"Struct",Token::TYPE_Struct},
      {"Array",Token::TYPE_Array},
      {"Add",Token::INSTR_Add},
      {"Sub", Token::INSTR_Sub},
      {"Mul", Token::INSTR_Mul},
      {"UDiv", Token::INSTR_UDiv},
      {"SDiv", Token::INSTR_SDiv},
      {"Shl", Token::INSTR_Shl},
      {"LShr", Token::INSTR_LShr},
      {"AShr", Token::INSTR_AShr},
      {"And", Token::INSTR_And},
      {"Or", Token::INSTR_Or},
      {"Xor", Token::INSTR_Xor},
      {"Not", Token::INSTR_Not},
      {"ICMP_SGT", Token::INSTR_ICMP_SGT},
      {"ICMP_UGT", Token::INSTR_ICMP_UGT},
      {"ICMP_SGE", Token::INSTR_ICMP_SGE},
      {"ICMP_UGE", Token::INSTR_ICMP_UGE},
      {"ICMP_SLT", Token::INSTR_ICMP_SLT},
      {"ICMP_ULT", Token::INSTR_ICMP_ULT},
      {"ICMP_SLE", Token::INSTR_ICMP_SLE},
      {"ICMP_ULE", Token::INSTR_ICMP_ULE},
      {"ICMP_EQ", Token::INSTR_ICMP_EQ},
      {"ICMP_NEQ",Token::INSTR_ICMP_NEQ},
      {"PtrAdd", Token::INSTR_PtrAdd},
      {"PtrEq", Token::INSTR_PtrEq},
      {"ExtractValue", Token::INSTR_ExtractValue},
      {"InsertValue", Token::INSTR_InsertValue},
      {"Trunc", Token::INSTR_Trunc},
      {"ZExt", Token::INSTR_ZExt},
      {"SExt", Token::INSTR_SExt},
      {"PtrToInt", Token::INSTR_PtrToInt},
      {"IntToPtr", Token::INSTR_IntToPtr},
      {"BitCast", Token::INSTR_BitCast},
      {"BoolZExt", Token::INSTR_BoolZExt},
      {"BoolSExt", Token::INSTR_BoolSExt},
      {"IntToBool", Token::INSTR_IntToBool},
      {"Store", Token::INSTR_Store},
      {"Load", Token::INSTR_Load},
      {"Skip", Token::INSTR_Skip},
      {"Call", Token::INSTR_Call},
      {"Assign", Token::INSTR_Assign},
      {"Ret", Token::INSTR_Ret},
      {"RetVoid", Token::INSTR_RetVoid},
      {"NonDet", Token::INSTR_NonDet},
      {"Uniform", Token::INSTR_Uniform},
      {"Assert", Token::INSTR_Assert},
      {"Assume", Token::INSTR_Assume},
      {"NegAssume", Token::INSTR_NegAssume},
      {"PRED_ICMP_SGT", Token::INSTR_PRED_ICMP_SGT},
      {"PRED_ICMP_UGT", Token::INSTR_PRED_ICMP_UGT},
      {"PRED_ICMP_SGE", Token::INSTR_PRED_ICMP_SGE},
      {"PRED_ICMP_UGE", Token::INSTR_PRED_ICMP_UGE},
      {"PRED_ICMP_SLT", Token::INSTR_PRED_ICMP_SLT},
      {"PRED_ICMP_ULT", Token::INSTR_PRED_ICMP_ULT},
      {"PRED_ICMP_SLE", Token::INSTR_PRED_ICMP_SLE},
      {"PRED_ICMP_ULE", Token::INSTR_PRED_ICMP_ULE},
      {"PRED_ICMP_EQ", Token::INSTR_PRED_ICMP_EQ},
      {"PRED_ICMP_NEQ", Token::INSTR_PRED_ICMP_NEQ}
  };

private:
  std::istream* in;
  Token cur_token;
  std::string buffer;

  Token get_token();
};

} // namespace Loader
} // namespace MiniMC

#endif // MINIMC_LEXER_HPP
