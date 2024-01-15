#include "LexicalAnalysis/Token.hpp"

#include <magic_enum/magic_enum.hpp>

#include "Utility/UTF32Functions.hpp"

namespace Cygni {
namespace LexicalAnalysis {

TokenTag Token::IdentifyKeyword(TokenTag tag, const std::u32string &text) {
  if (tag == TokenTag::Identifier) {
    if (keywords.find(text) != keywords.end()) {
      return keywords[text];
    } else {
      return tag;
    }
  } else {
    return tag;
  }
}

std::unordered_map<std::u32string, TokenTag> Token::keywords = {
    {U"if", TokenTag::If},
    {U"else", TokenTag::Else},
    {U"var", TokenTag::Var},
    {U"func", TokenTag::Func},
    {U"return", TokenTag::Return},
    {U"true", TokenTag::True},
    {U"false", TokenTag::False},
    {U"and", TokenTag::And},
    {U"or", TokenTag::Or},
    {U"while", TokenTag::While},
    {U"module", TokenTag::Module},
    {U"package", TokenTag::Package},
    {U"import", TokenTag::Import},
    {U"require", TokenTag::Require},
    {U"rename", TokenTag::Rename},
    {U"to", TokenTag::To},
    {U"interface", TokenTag::Interface}};

std::unordered_map<std::u32string, TokenTag> Token::operators = {
    {U"+", TokenTag::Add},
    {U"-", TokenTag::Subtract},
    {U"*", TokenTag::Multiply},
    {U"/", TokenTag::Divide},
    {U"%", TokenTag::Modulo},
    {U">", TokenTag::GreaterThan},
    {U"<", TokenTag::LessThan},
    {U">=", TokenTag::GreaterThanOrEqual},
    {U"<=", TokenTag::LessThanOrEqual},
    {U"==", TokenTag::Equal},
    {U"!=", TokenTag::NotEqual},
    {U"(", TokenTag::LeftParenthesis},
    {U")", TokenTag::RightParenthesis},
    {U"[", TokenTag::LeftBracket},
    {U"]", TokenTag::RightBracket},
    {U"{", TokenTag::LeftBrace},
    {U"}", TokenTag::RightBrace},
    {U":", TokenTag::Colon},
    {U",", TokenTag::Comma},
    {U".", TokenTag::Dot},
    {U";", TokenTag::Semicolon},
    {U"=", TokenTag::Assign},
    {U"=>", TokenTag::GoesTo},
    {U"@", TokenTag::At},
    {U"::", TokenTag::ScopeResolutionOperator}};

Json Token::ToJson() const {
  return {{"line", line},
          {"column", column},
          {"tag", magic_enum::enum_name<TokenTag>(tag)},
          {"text", Utility::UTF32ToUTF8(text)}};
}

}; /* namespace LexicalAnalysis */
}; /* namespace Cygni */