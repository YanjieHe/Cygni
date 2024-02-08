#include "Visitors/Compiler.hpp"

#include <bit_converter/bit_converter.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
using std::cout;
using std::endl;

using flint_bytecode::OpCode;
using flint_bytecode::Byte;

namespace Cygni {
namespace Visitors {

Compiler::Compiler(TypeChecker &typeChecker, NameLocator &nameLocator,
                   NamespaceFactory &namespaceFactory)
    : typeChecker{typeChecker}, nameLocator{nameLocator},
      namespaceFactory{namespaceFactory}, mainFunctionIndex{-1} {
  namespaceStack.push(namespaceFactory.GetRoot());
}

void Compiler::VisitBinary(
    const BinaryExpression *node, ByteCode &byteCode,
    std::vector<flint_bytecode::Constant> &constantPool) {
  Visit(node->Left(), byteCode, constantPool);
  Visit(node->Right(), byteCode, constantPool);
  if (node->NodeType() == ExpressionType::Assign) {
    throw std::runtime_error("assignment not supported");
  }
  TypeCode typeCode = typeChecker.GetType(node->Left())->GetTypeCode();
  if (typeCode != typeChecker.GetType(node->Right())->GetTypeCode()) {
    throw std::runtime_error("binary type mismatch");
  }
  switch (typeCode) {
  case TypeCode::Int32: {
    switch (node->NodeType()) {
    case ExpressionType::Add:
      byteCode.AddOp(OpCode::ADD_I32);
      break;
    case ExpressionType::Subtract:
      byteCode.AddOp(OpCode::SUB_I32);
      break;
    case ExpressionType::Multiply:
      byteCode.AddOp(OpCode::MUL_I32);
      break;
    case ExpressionType::Divide:
      byteCode.AddOp(OpCode::DIV_I32);
      break;
    case ExpressionType::Modulo:
      byteCode.AddOp(OpCode::MOD_I32);
      break;
    case ExpressionType::GreaterThan:
      byteCode.AddOp(OpCode::GT_I32);
      break;
    case ExpressionType::GreaterThanOrEqual:
      byteCode.AddOp(OpCode::GE_I32);
      break;
    case ExpressionType::LessThan:
      byteCode.AddOp(OpCode::LT_I32);
      break;
    case ExpressionType::LessThanOrEqual:
      byteCode.AddOp(OpCode::LE_I32);
      break;
    case ExpressionType::Equal:
      byteCode.AddOp(OpCode::EQ_I32);
      break;
    case ExpressionType::NotEqual:
      byteCode.AddOp(OpCode::NE_I32);
      break;
    default:
      throw std::runtime_error(
          "Unsupported binary operator for integer 32 type.");
    }
    break;
  }
  case TypeCode::Int64: {
    switch (node->NodeType()) {
    case ExpressionType::Add:
      byteCode.AddOp(OpCode::ADD_I64);
      break;
    case ExpressionType::Subtract:
      byteCode.AddOp(OpCode::SUB_I64);
      break;
    case ExpressionType::Multiply:
      byteCode.AddOp(OpCode::MUL_I64);
      break;
    case ExpressionType::Divide:
      byteCode.AddOp(OpCode::DIV_I64);
      break;
    case ExpressionType::Modulo:
      byteCode.AddOp(OpCode::MOD_I64);
      break;
    case ExpressionType::GreaterThan:
      byteCode.AddOp(OpCode::GT_I64);
      break;
    case ExpressionType::GreaterThanOrEqual:
      byteCode.AddOp(OpCode::GE_I64);
      break;
    case ExpressionType::LessThan:
      byteCode.AddOp(OpCode::LT_I64);
      break;
    case ExpressionType::LessThanOrEqual:
      byteCode.AddOp(OpCode::LE_I64);
      break;
    case ExpressionType::Equal:
      byteCode.AddOp(OpCode::EQ_I64);
      break;
    case ExpressionType::NotEqual:
      byteCode.AddOp(OpCode::NE_I64);
      break;
    default:
      throw std::runtime_error(
          "Unsupported binary operator for integer 64 type.");
    }
    break;
  }
  case TypeCode::Float32: {
    switch (node->NodeType()) {
    case ExpressionType::Add:
      byteCode.AddOp(OpCode::ADD_F32);
      break;
    case ExpressionType::Subtract:
      byteCode.AddOp(OpCode::SUB_F32);
      break;
    case ExpressionType::Multiply:
      byteCode.AddOp(OpCode::MUL_F32);
      break;
    case ExpressionType::Divide:
      byteCode.AddOp(OpCode::DIV_F32);
      break;
    case ExpressionType::Modulo:
      byteCode.AddOp(OpCode::MOD_F32);
      break;
    case ExpressionType::GreaterThan:
      byteCode.AddOp(OpCode::GT_F32);
      break;
    case ExpressionType::GreaterThanOrEqual:
      byteCode.AddOp(OpCode::GE_F32);
      break;
    case ExpressionType::LessThan:
      byteCode.AddOp(OpCode::LT_F32);
      break;
    case ExpressionType::LessThanOrEqual:
      byteCode.AddOp(OpCode::LE_F32);
      break;
    case ExpressionType::Equal:
      byteCode.AddOp(OpCode::EQ_F32);
      break;
    case ExpressionType::NotEqual:
      byteCode.AddOp(OpCode::NE_F32);
      break;
    default:
      throw std::runtime_error(
          "Unsupported binary operator for float 32 type.");
    }
    break;
  }
  case TypeCode::Float64: {
    switch (node->NodeType()) {
    case ExpressionType::Add:
      byteCode.AddOp(OpCode::ADD_F64);
      break;
    case ExpressionType::Subtract:
      byteCode.AddOp(OpCode::SUB_F64);
      break;
    case ExpressionType::Multiply:
      byteCode.AddOp(OpCode::MUL_F64);
      break;
    case ExpressionType::Divide:
      byteCode.AddOp(OpCode::DIV_F64);
      break;
    case ExpressionType::Modulo:
      byteCode.AddOp(OpCode::MOD_F64);
      break;
    case ExpressionType::Equal:
      byteCode.AddOp(OpCode::EQ_F64);
      break;
    case ExpressionType::NotEqual:
      byteCode.AddOp(OpCode::NE_F64);
      break;
    case ExpressionType::GreaterThan:
      byteCode.AddOp(OpCode::GT_F64);
      break;
    case ExpressionType::GreaterThanOrEqual:
      byteCode.AddOp(OpCode::GE_F64);
      break;
    case ExpressionType::LessThan:
      byteCode.AddOp(OpCode::LT_F64);
      break;
    case ExpressionType::LessThanOrEqual:
      byteCode.AddOp(OpCode::LE_F64);
      break;
    default:
      throw std::runtime_error(
          "Unsupported binary operator for float 64 type.");
    }
    break;
  }
  case TypeCode::Boolean:
  case TypeCode::Char: {
    if (node->NodeType() == ExpressionType::Equal) {
      byteCode.AddOp(OpCode::EQ_I32);
    } else if (node->NodeType() == ExpressionType::NotEqual) {
      byteCode.AddOp(OpCode::NE_I32);
    } else {
      throw std::runtime_error("Unsupported binary expression type");
    }
    break;
  }
  default: { throw std::runtime_error("Unsupported binary expression type"); }
  }
}

void Compiler::VisitUnary(const UnaryExpression *node, ByteCode &byteCode,
                          std::vector<flint_bytecode::Constant> &constantPool) {
  if (node->NodeType() == ExpressionType::Not &&
      typeChecker.GetType(node)->GetTypeCode() == TypeCode::Boolean) {
    throw std::runtime_error("Unsupported unary expression type");
    // Visit(node->Operand(), byteCode, constantPool);
    // byteCode.AddOp(OpCode::BIT_NOT_I32);
  } else if (node->NodeType() == ExpressionType::Halt) {
    Visit(node->Operand(), byteCode, constantPool);
    byteCode.AddOp(OpCode::HALT);
  } else if (node->NodeType() == ExpressionType::Convert) {
    Visit(node->Operand(), byteCode, constantPool);
    switch (typeChecker.GetType(node->Operand())->GetTypeCode()) {
    case TypeCode::Boolean:
    case TypeCode::Int32: {
      switch (node->GetType()->GetTypeCode()) {
      case TypeCode::Int32:
      case TypeCode::Boolean: {
        break;
      }
      case TypeCode::Int64: {
        byteCode.AddOp(OpCode::CAST_I32_TO_I64);
        break;
      }
      case TypeCode::Float32: {
        byteCode.AddOp(OpCode::CAST_I32_TO_F32);
        break;
      }
      case TypeCode::Float64: {
        byteCode.AddOp(OpCode::CAST_I32_TO_F64);
        break;
      }
      default: { throw std::runtime_error("Cannot convert the type."); }
      }
      break;
    }
    case TypeCode::Int64: {
      switch (node->GetType()->GetTypeCode()) {
      case TypeCode::Int32:
      case TypeCode::Boolean: {
        byteCode.AddOp(OpCode::CAST_I64_TO_I32);
        break;
      }
      case TypeCode::Int64: {
        break;
      }
      case TypeCode::Float32: {
        byteCode.AddOp(OpCode::CAST_I64_TO_F32);
        break;
      }
      case TypeCode::Float64: {
        byteCode.AddOp(OpCode::CAST_I64_TO_F64);
        break;
      }
      default: { throw std::runtime_error("Cannot convert the type."); }
      }
      break;
    }
    case TypeCode::Float32: {
      switch (node->GetType()->GetTypeCode()) {
      case TypeCode::Int32: {
        byteCode.AddOp(OpCode::CAST_F32_TO_I32);
        break;
      }
      case TypeCode::Int64: {
        byteCode.AddOp(OpCode::CAST_F32_TO_I64);
        break;
      }
      case TypeCode::Float32: {
        break;
      }
      case TypeCode::Float64: {
        byteCode.AddOp(OpCode::CAST_F32_TO_F64);
        break;
      }
      default: { throw std::runtime_error("Cannot convert the type."); }
      }
      break;
    }
    case TypeCode::Float64: {
      switch (node->GetType()->GetTypeCode()) {
      case TypeCode::Int32: {
        byteCode.AddOp(OpCode::CAST_F64_TO_I32);
        break;
      }
      case TypeCode::Int64: {
        byteCode.AddOp(OpCode::CAST_F64_TO_I64);
        break;
      }
      case TypeCode::Float32: {
        byteCode.AddOp(OpCode::CAST_F64_TO_F32);
        break;
      }
      case TypeCode::Float64: {
        break;
      }
      default: { throw std::runtime_error("Cannot convert the type."); }
      }
      break;
    }
    default: { throw std::runtime_error("Cannot convert the type."); }
    }
  } else {
    throw std::runtime_error("Unsupported unary expression type");
  }
}

void Compiler::VisitConstant(
    const ConstantExpression *node, ByteCode &byteCode,
    std::vector<flint_bytecode::Constant> &constantPool) {
  switch (typeChecker.GetType(node)->GetTypeCode()) {
  case TypeCode::Int32: {
    int32_t value = std::any_cast<int32_t>(node->Value());
    if (value == 0) {
      byteCode.AddOp(OpCode::PUSH_I32_0);
    } else if (value == 1) {
      byteCode.AddOp(OpCode::PUSH_I32_1);
    } else if (std::numeric_limits<int8_t>::min() < value &&
               value < std::numeric_limits<int8_t>::max()) {
      byteCode.AddOp(OpCode::PUSH_I32_1BYTE);
      byteCode.AddByte(static_cast<Byte>(value));
    } else if (std::numeric_limits<int16_t>::min() < value &&
               value < std::numeric_limits<int16_t>::max()) {
      byteCode.AddOp(OpCode::PUSH_I32_1BYTE);
      size_t location = byteCode.GetBytes().size();
      bit_converter::i16_to_bytes(static_cast<int16_t>(value), true,
                                  byteCode.GetBytes().begin() + location);
    } else {
      byteCode.AddOp(OpCode::PUSH_I32);
      byteCode.AddByte(static_cast<Byte>(constantPool.size()));
      constantPool.push_back(flint_bytecode::Constant(
          flint_bytecode::ConstantKind::CONSTANT_KIND_I32, value));
    }
    break;
  }
  case TypeCode::Int64: {
    int64_t value = std::any_cast<int64_t>(node->Value());
    if (value == 0) {
      byteCode.AddOp(OpCode::PUSH_I64_0);
    } else if (value == 1) {
      byteCode.AddOp(OpCode::PUSH_I64_1);
    } else {
      byteCode.AddOp(OpCode::PUSH_I64);
      byteCode.AddByte(static_cast<Byte>(constantPool.size()));
      constantPool.push_back(flint_bytecode::Constant(
          flint_bytecode::ConstantKind::CONSTANT_KIND_I64, value));
    }
    break;
  }
  case TypeCode::Float32: {
    float_t value = std::any_cast<float_t>(node->Value());
    if (value == 0.0) {
      byteCode.AddOp(OpCode::PUSH_F32_0);
    } else if (value == 1.0) {
      byteCode.AddOp(OpCode::PUSH_F32_1);
    } else {
      byteCode.AddOp(OpCode::PUSH_F32);
      byteCode.AddByte(static_cast<Byte>(constantPool.size()));
      constantPool.push_back(flint_bytecode::Constant(
          flint_bytecode::ConstantKind::CONSTANT_KIND_F32, value));
    }
    break;
  }
  case TypeCode::Float64: {
    double_t value = std::any_cast<double_t>(node->Value());
    if (value == 0.0) {
      byteCode.AddOp(OpCode::PUSH_F64_0);
    } else if (value == 1.0) {
      byteCode.AddOp(OpCode::PUSH_F64_1);
    } else {
      byteCode.AddOp(OpCode::PUSH_F64);
      byteCode.AddByte(static_cast<Byte>(constantPool.size()));
      constantPool.push_back(flint_bytecode::Constant(
          flint_bytecode::ConstantKind::CONSTANT_KIND_F64, value));
    }
    break;
  }
  case TypeCode::Boolean: {
    if (std::any_cast<bool>(node->Value())) {
      byteCode.AddOp(OpCode::PUSH_I32_1);
    } else {
      byteCode.AddOp(OpCode::PUSH_I32_0);
    }
    break;
  }
  case TypeCode::String: {
    std::u32string value = std::any_cast<std::u32string>(node->Value());
    byteCode.AddOp(OpCode::PUSH_STRING);
    byteCode.AddByte(static_cast<Byte>(constantPool.size()));
    constantPool.push_back(flint_bytecode::Constant(
        flint_bytecode::ConstantKind::CONSTANT_KIND_STRING,
        Utility::UTF32ToUTF8(value)));

    break;
  }
  default: {
    /* TODO: other constant types */
    break;
  }
  }
}

void Compiler::VisitParameter(
    const ParameterExpression *node, ByteCode &byteCode,
    std::vector<flint_bytecode::Constant> &constantPool) {
  if (nameLocator.ExistsNameInfo(node, LocationKind::FunctionVariable)) {
    const NameInfo &nameInfo =
        nameLocator.GetNameInfo(node, LocationKind::FunctionVariable);
    switch (typeChecker.GetType(node)->GetTypeCode()) {
    case TypeCode::Int32:
    case TypeCode::Boolean:
    case TypeCode::Char: {
      byteCode.AddOp(OpCode::PUSH_LOCAL_I32);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::Int64: {
      byteCode.AddOp(OpCode::PUSH_LOCAL_I64);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::Float32: {
      byteCode.AddOp(OpCode::PUSH_LOCAL_F32);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::Float64: {
      byteCode.AddOp(OpCode::PUSH_LOCAL_F64);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::String: {
      byteCode.AddOp(OpCode::PUSH_LOCAL_OBJECT);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    default: {
      throw std::runtime_error("Unsupported parameter expression type");
    }
    }
  } else if (nameLocator.ExistsNameInfo(node, LocationKind::GlobalVariable)) {
    const NameInfo &nameInfo =
        nameLocator.GetNameInfo(node, LocationKind::GlobalVariable);
    switch (typeChecker.GetType(node)->GetTypeCode()) {
    case TypeCode::Int32:
    case TypeCode::Boolean:
    case TypeCode::Char: {
      byteCode.AddOp(OpCode::PUSH_GLOBAL_I32);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::Int64: {
      byteCode.AddOp(OpCode::PUSH_GLOBAL_I64);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::Float32: {
      byteCode.AddOp(OpCode::PUSH_GLOBAL_F32);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::Float64: {
      byteCode.AddOp(OpCode::PUSH_GLOBAL_F64);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    case TypeCode::String: {
      byteCode.AddOp(OpCode::PUSH_GLOBAL_OBJECT);
      byteCode.AddByte(nameInfo.Number());
      break;
    }
    default: {
      throw std::runtime_error("Unsupported parameter expression type");
    }
    }
  } else {
    throw std::runtime_error("Unsupported parameter expression location kind");
  }
}

void Compiler::VisitBlock(const BlockExpression *node, ByteCode &byteCode,
                          std::vector<flint_bytecode::Constant> &constantPool) {
  for (auto expression : node->Expressions()) {
    Visit(expression, byteCode, constantPool);
  }
}

void Compiler::VisitConditional(
    const ConditionalExpression *node, ByteCode &byteCode,
    std::vector<flint_bytecode::Constant> &constantPool) {
  Visit(node->Test(), byteCode, constantPool);
  byteCode.AddOp(OpCode::JUMP_IF_FALSE);
  int32_t location1 = static_cast<int32_t>(byteCode.GetBytes().size());
  byteCode.Extend(sizeof(int16_t));

  Visit(node->IfTrue(), byteCode, constantPool);
  byteCode.AddOp(OpCode::JUMP);
  int32_t location2 = static_cast<int32_t>(byteCode.GetBytes().size());
  byteCode.Extend(sizeof(int16_t));

  Visit(node->IfFalse(), byteCode, constantPool);
  int32_t location3 = static_cast<int32_t>(byteCode.GetBytes().size());

  spdlog::debug("location 1: {}, location 2: {}, location 3: {}", location1,
                location2, location3);
  bit_converter::i16_to_bytes(static_cast<int16_t>(location2 - location1), true,
                              byteCode.GetBytes().begin() + location1);

  bit_converter::i16_to_bytes(
      static_cast<int16_t>(location3 - (location2 + sizeof(int16_t))), true,
      byteCode.GetBytes().begin() + location2);
}

void Compiler::VisitCall(const CallExpression *node, ByteCode &byteCode,
                         std::vector<flint_bytecode::Constant> &constantPool) {
  for (const Expression *argument : node->Arguments()) {
    Visit(argument, byteCode, constantPool);
  }
  if (node->Function()->NodeType() == ExpressionType::Parameter) {
    if (nameLocator.ExistsNameInfo(node->Function(), LocationKind::Function)) {
      const NameInfo &nameInfo =
          nameLocator.GetNameInfo(node->Function(), LocationKind::Function);
      byteCode.AddOp(OpCode::INVOKE_FUNCTION);
      byteCode.AddByte(constantPool.size());
      constantPool.push_back(flint_bytecode::Constant(
          flint_bytecode::ConstantKind::CONSTANT_KIND_FUNCTION,
          nameInfo.Number()));
    } else if (nameLocator.ExistsNameInfo(node->Function(),
                                          LocationKind::NativeFunction)) {
      const NameInfo &nameInfo = nameLocator.GetNameInfo(
          node->Function(), LocationKind::NativeFunction);
      byteCode.AddOp(OpCode::INVOKE_NATIVE_FUNCTION);
      byteCode.AddByte(constantPool.size());
      constantPool.push_back(flint_bytecode::Constant(
          flint_bytecode::ConstantKind::CONSTANT_KIND_NATIVE_FUNCTION,
          nameInfo.Number()));
    } else {
      throw std::runtime_error("Unsupported call expression location kind");
    }
  } else {
    cout << "node type: " << (int)node->Function()->NodeType() << endl;
    throw std::runtime_error("Unsupported call expression type");
  }
}

void Compiler::VisitLambda(
    const LambdaExpression *node, ByteCode &byteCode,
    std::vector<flint_bytecode::Constant> &constantPool) {
  throw std::runtime_error("This function is not implemented");
}

void Compiler::VisitLoop(const LoopExpression *node, ByteCode &byteCode,
                         std::vector<flint_bytecode::Constant> &constantPool) {
  Visit(node->Initializer(), byteCode, constantPool);
  flint_bytecode::Label label1 = byteCode.CreateLabel(byteCode.Size());

  Visit(node->Condition(), byteCode, constantPool);
  byteCode.AddOp(OpCode::JUMP_IF_FALSE);
  flint_bytecode::Label label2 = byteCode.CreateLabel(byteCode.Size());
  byteCode.AddI16(0);

  Visit(node->Body(), byteCode, constantPool);
  byteCode.AddOp(OpCode::JUMP);
  byteCode.AddI16(byteCode.Size() + sizeof(int16_t) - label1.Location());

  byteCode.Rewrite(label2,
                   byteCode.Size() -
                       static_cast<int>(label1.Location() + sizeof(int16_t)));
}

void Compiler::VisitDefault(
    const DefaultExpression *node, ByteCode &byteCode,
    std::vector<flint_bytecode::Constant> &constantPool) {
  switch (node->GetType()->GetTypeCode()) {
  case TypeCode::Char:
  case TypeCode::Boolean:
  case TypeCode::Int32:
  case TypeCode::Empty: {
    byteCode.AddOp(OpCode::PUSH_I32_0);
    break;
  }
  case TypeCode::Int64: {
    byteCode.AddOp(OpCode::PUSH_I64_0);
    break;
  }
  case TypeCode::Float32: {
    byteCode.AddOp(OpCode::PUSH_F32_0);
    break;
  }
  case TypeCode::Float64: {
    byteCode.AddOp(OpCode::PUSH_F64_0);
    break;
  }
  default: {
    throw TreeException(__FILE__, __LINE__,
                        "This type is not supported by the default expression.",
                        static_cast<const Expression *>(node), nullptr);
  }
  }
}

void Compiler::VisitVariableDeclaration(
    const VariableDeclarationExpression *node, ByteCode &byteCode,
    std::vector<flint_bytecode::Constant> &constantPool) {
  NameInfo nameInfo = nameLocator.GetNameInfo(node, LocationKind::FunctionVariable);
  int offset = nameInfo.Number();
  Visit(node->Initializer(), byteCode, constantPool);
  const Type *type = typeChecker.GetType(node);
  switch (type->GetTypeCode()) {
  case TypeCode::Boolean:
  case TypeCode::Char:
  case TypeCode::Int32: {
    byteCode.AddOp(OpCode::POP_LOCAL_I32);
    byteCode.AddByte(static_cast<Byte>(offset));
    break;
  }
  case TypeCode::Int64: {
    byteCode.AddOp(OpCode::POP_LOCAL_I64);
    byteCode.AddByte(static_cast<Byte>(offset));
    break;
  }
  case TypeCode::Float32: {
    byteCode.AddOp(OpCode::POP_LOCAL_F32);
    byteCode.AddByte(static_cast<Byte>(offset));
    break;
  }
  case TypeCode::Float64: {
    byteCode.AddOp(OpCode::POP_LOCAL_F64);
    byteCode.AddByte(static_cast<Byte>(offset));
    break;
  }
  default: {
    spdlog::error("The variable declaration expression does not support this "
                  "type. Variable name: '{}'.",
                  Utility::UTF32ToUTF8(node->Name()));
    throw TreeException(
        __FILE__, __LINE__,
        "This type is not supported by the variable declaration expression.",
        static_cast<const Expression *>(node), nullptr);
  }
  }
}

flint_bytecode::Function
Compiler::CompileFunction(const std::string &name,
                          const LambdaExpression *node) {
  spdlog::info("Compile function \"{}\".", name);
  ByteCode byteCode;
  std::vector<flint_bytecode::Constant> constantPool;
  Visit(node->Body(), byteCode, constantPool);

  if (name == "Main") {
    byteCode.AddOp(OpCode::HALT);
  } else {
    switch (node->ReturnType()->GetTypeCode()) {
    case TypeCode::Empty: {
      byteCode.AddOp(OpCode::RETURN);
      break;
    }
    case TypeCode::Int32:
    case TypeCode::Boolean:
    case TypeCode::Char: {
      byteCode.AddOp(OpCode::RETURN_I32);
      break;
    }
    case TypeCode::Int64: {
      byteCode.AddOp(OpCode::RETURN_I64);
      break;
    }
    case TypeCode::Float32: {
      byteCode.AddOp(OpCode::RETURN_F32);
      break;
    }
    case TypeCode::Float64: {
      byteCode.AddOp(OpCode::RETURN_F64);
      break;
    }
    default: {
      byteCode.AddOp(OpCode::RETURN_OBJECT);
      break;
    }
    }
  }
  int localVariableCount = nameLocator.GetNameInfo(node, LocationKind::FunctionVariableCount).Number();
  Byte locals = static_cast<Byte>(localVariableCount -
                                  static_cast<int>(node->Parameters().size()));
  spdlog::info("Finish compiling function \"{}\".", name);
  return flint_bytecode::Function(name, 0, locals, node->Parameters().size(),
                                  constantPool, byteCode);
}

flint_bytecode::NativeFunction
Compiler::CompileNativeFunction(const std::string &name,
                                const LambdaExpression *node) {
  int argsSize = static_cast<int>(node->Parameters().size());
  std::u32string libraryName;
  std::u32string entryPoint;
  bool foundLibraryName = false;
  bool foundEntryPoint = false;
  for (const auto &annotation : node->Annotations()) {
    if (annotation.Name() == U"External") {
      for (const auto &arg : annotation.Arguments()) {
        if (arg.Name() == U"Library") {
          if (foundLibraryName) {
            spdlog::error("Library name was defined more than once.");

            throw TreeException(__FILE__, __LINE__,
                                "Library name was defined more than once.",
                                static_cast<const Expression *>(node), nullptr);
          } else {
            foundLibraryName = true;
            libraryName = std::any_cast<std::u32string>(arg.Value());
          }
        } else if (arg.Name() == U"EntryPoint") {
          if (foundEntryPoint) {
            spdlog::error("Entry point was defined more than once.");

            throw TreeException(__FILE__, __LINE__,
                                "Entry point was defined more than once.",
                                static_cast<const Expression *>(node), nullptr);
          } else {
            foundEntryPoint = true;
            entryPoint = std::any_cast<std::u32string>(arg.Value());
          }
        }
      }
    }
  }
  if (foundLibraryName == false) {
    spdlog::error("Can't find library name in the annotations of the native "
                  "function definition.");

    throw TreeException(
        __FILE__, __LINE__,
        "Can't find library name in the annotations of the native "
        "function definition.",
        static_cast<const Expression *>(node), nullptr);
  }
  if (foundEntryPoint == false) {
    spdlog::error("Can't find entry point in the annotations of the native "
                  "function definition.");

    throw TreeException(
        __FILE__, __LINE__,
        "Can't find entry point in the annotations of the native "
        "function definition.",
        static_cast<const Expression *>(node), nullptr);
  }
  if (!libraryMap.ContainsKey(libraryName)) {
    flint_bytecode::NativeLibrary nativeLibrary(
        Utility::UTF32ToUTF8(libraryName));
    libraryMap.AddItem(libraryName, nativeLibrary);
  }

  int nativeLibraryOffset =
      static_cast<int>(libraryMap.GetIndexByKey(libraryName));
  return flint_bytecode::NativeFunction(Utility::UTF32ToUTF8(entryPoint),
                                        argsSize, nativeLibraryOffset);
}

std::vector<flint_bytecode::NativeLibrary> Compiler::GetNativeLibraries() {
  return libraryMap.GetAllItems();
}

void Compiler::CompileNamespace(
    std::vector<flint_bytecode::Function> &functions,
    std::vector<flint_bytecode::NativeFunction> &nativeFunctions) {
  Namespace *top = namespaceStack.top();

  for (const auto &funcDecl : top->Functions().GetAllItems()) {
    if (funcDecl->IsNativeFunction()) {
      auto function = CompileNativeFunction(
          Utility::UTF32ToUTF8(funcDecl->Name()), funcDecl);
      int index = nameLocator.GetNameInfo(funcDecl, LocationKind::NativeFunction).Number();
      nativeFunctions.at(index) = function;
    } else {
      auto function =
          CompileFunction(Utility::UTF32ToUTF8(funcDecl->Name()), funcDecl);
      int index = nameLocator.GetNameInfo(funcDecl, LocationKind::Function).Number();
      functions.at(index) = function;

      if (funcDecl->Name() == U"Main") {
        if (mainFunctionIndex >= 0) {
          throw TreeException(__FILE__, __LINE__,
                              "Multiple 'Main' functions are defined within "
                              "the same module or across different modules.",
                              static_cast<const Expression *>(funcDecl),
                              nullptr);
        } else {
          mainFunctionIndex = index;
        }
      }
    }
  }

  for (const auto &current : top->Children().GetAllItems()) {
    namespaceStack.push(current);
    CompileNamespace(functions, nativeFunctions);
    namespaceStack.pop();
    spdlog::info("Finish compiling namespace \"{}\".",
                 Utility::UTF32ToUTF8(current->Name()));
  }
}

int Compiler::EntryPoint() const {
  if (mainFunctionIndex >= 0) {
    return mainFunctionIndex;
  } else {
    throw TreeException(__FILE__, __LINE__, "'Main' function is not defined.",
                        nullptr, nullptr);
  }
}

}; /* namespace Visitors */
}; /* namespace Cygni */