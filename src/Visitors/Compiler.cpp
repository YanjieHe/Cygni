#include "Visitors/Compiler.hpp"

#include "Utility/Convert.hpp"
#include "Visitors/CompilationException.hpp"
#include <bit_converter/bit_converter.hpp>
#include <spdlog/spdlog.h>

using flint_bytecode::Byte;
using flint_bytecode::OpCode;

namespace Cygni
{
namespace Visitors
{

Compiler::Compiler(TypeChecker &typeChecker, NameLocator &nameLocator, NamespaceFactory &namespaceFactory)
    : typeChecker{typeChecker}, nameLocator{nameLocator}, namespaceFactory{namespaceFactory}, mainFunctionIndex{-1}
{
    namespaceStack.push(namespaceFactory.GetRoot());
}

void Compiler::VisitBinary(const BinaryExpression *node, ByteCode &byteCode,
                           std::vector<flint_bytecode::Constant> &constantPool)
{
    if (node->NodeType() == ExpressionType::Assign)
    {
        CompileAssignment(node, byteCode, constantPool);
    }
    else if (node->NodeType() == ExpressionType::And)
    {
        CompileLogicalAnd(node, byteCode, constantPool);
    }
    else if (node->NodeType() == ExpressionType::Or)
    {
        CompileLogicalOr(node, byteCode, constantPool);
    }
    else
    {
        Visit(node->Left(), byteCode, constantPool);
        Visit(node->Right(), byteCode, constantPool);
        TypeCode typeCode = typeChecker.GetType(node->Left())->GetTypeCode();
        if (typeCode != typeChecker.GetType(node->Right())->GetTypeCode())
        {
            spdlog::error("Binary operation type mismatch.");

            throw CompilationException(__FILE__, __LINE__, "Binary operation type mismatch", node, nullptr);
        }
        switch (typeCode)
        {
        case TypeCode::Int32: {
            switch (node->NodeType())
            {
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
                throw CompilationException(__FILE__, __LINE__, "Unsupported binary operator for integer 32 type.", node,
                                           nullptr);
            }
            break;
        }
        case TypeCode::Int64: {
            switch (node->NodeType())
            {
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
                throw CompilationException(__FILE__, __LINE__, "Unsupported binary operator for integer 64 type.", node,
                                           nullptr);
            }
            break;
        }
        case TypeCode::Float32: {
            switch (node->NodeType())
            {
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
                throw CompilationException(__FILE__, __LINE__, "Unsupported binary operator for float 32 type.", node,
                                           nullptr);
            }
            break;
        }
        case TypeCode::Float64: {
            switch (node->NodeType())
            {
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
                throw CompilationException(__FILE__, __LINE__, "Unsupported binary operator for float 64 type.", node,
                                           nullptr);
            }
            break;
        }
        case TypeCode::Boolean: {
            switch (node->NodeType())
            {
            case ExpressionType::Equal:
                byteCode.AddOp(OpCode::EQ_I32);
                break;
            case ExpressionType::NotEqual:
                byteCode.AddOp(OpCode::NE_I32);
                break;
            default:
                spdlog::error("Unsupported binary expression type for boolean type operands.");

                throw TreeException(__FILE__, __LINE__, "Unsupported binary expression type for boolean type operands.",
                                    node, nullptr);
            }
            break;
        }
        case TypeCode::Char: {
            switch (node->NodeType())
            {
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
            default: {
                spdlog::error("Unsupported binary expression type for character type operands.");

                throw TreeException(__FILE__, __LINE__,
                                    "Unsupported binary expression type for character type operands.", node, nullptr);
            }
            }
            break;
        }
        default: {
            throw CompilationException(__FILE__, __LINE__, "Unsupported binary expression type", node, nullptr);
        }
        }
    }
}

void Compiler::VisitUnary(const UnaryExpression *node, ByteCode &byteCode,
                          std::vector<flint_bytecode::Constant> &constantPool)
{
    if (node->NodeType() == ExpressionType::Not && typeChecker.GetType(node)->GetTypeCode() == TypeCode::Boolean)
    {
        Visit(node->Operand(), byteCode, constantPool);
        byteCode.AddOp(OpCode::LOGICAL_NOT);
    }
    else if (node->NodeType() == ExpressionType::Halt)
    {
        Visit(node->Operand(), byteCode, constantPool);
        byteCode.AddOp(OpCode::HALT);
    }
    else if (node->NodeType() == ExpressionType::Convert)
    {
        Visit(node->Operand(), byteCode, constantPool);
        const Type *targetType = typeChecker.GetType(node);
        switch (typeChecker.GetType(node->Operand())->GetTypeCode())
        {
        case TypeCode::Boolean:
        case TypeCode::Char:
        case TypeCode::Int32: {
            switch (targetType->GetTypeCode())
            {
            case TypeCode::Boolean:
            case TypeCode::Char:
            case TypeCode::Int32: {
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
            default: {
                throw CompilationException(__FILE__, __LINE__, "Cannot convert the type.", node, nullptr);
            }
            }
            break;
        }
        case TypeCode::Int64: {
            switch (targetType->GetTypeCode())
            {
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
            default: {
                throw CompilationException(__FILE__, __LINE__, "Cannot convert the type.", node, nullptr);
            }
            }
            break;
        }
        case TypeCode::Float32: {
            switch (targetType->GetTypeCode())
            {
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
            default: {
                throw CompilationException(__FILE__, __LINE__, "Cannot convert the type.", node, nullptr);
            }
            }
            break;
        }
        case TypeCode::Float64: {
            switch (targetType->GetTypeCode())
            {
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
            default: {
                throw CompilationException(__FILE__, __LINE__, "Cannot convert the type.", node, nullptr);
            }
            }
            break;
        }
        default: {
            throw CompilationException(__FILE__, __LINE__, "Cannot convert the type.", node, nullptr);
        }
        }
    }
    else
    {
        throw CompilationException(__FILE__, __LINE__, "Unsupported unary expression type", node, nullptr);
    }
}

void Compiler::VisitConstant(const ConstantExpression *node, ByteCode &byteCode,
                             std::vector<flint_bytecode::Constant> &constantPool)
{
    switch (typeChecker.GetType(node)->GetTypeCode())
    {
    case TypeCode::Int32: {
        int32_t value = std::get<int32_t>(node->Value());
        if (value == 0)
        {
            byteCode.AddOp(OpCode::PUSH_I32_0);
        }
        else if (value == 1)
        {
            byteCode.AddOp(OpCode::PUSH_I32_1);
        }
        else if (std::numeric_limits<int8_t>::min() < value && value < std::numeric_limits<int8_t>::max())
        {
            byteCode.AddOp(OpCode::PUSH_I32_1BYTE);
            byteCode.AddByte(static_cast<Byte>(value));
        }
        else if (std::numeric_limits<int16_t>::min() < value && value < std::numeric_limits<int16_t>::max())
        {
            byteCode.AddOp(OpCode::PUSH_I32_1BYTE);
            size_t location = byteCode.GetBytes().size();
            bit_converter::i16_to_bytes(static_cast<int16_t>(value), true, byteCode.GetBytes().begin() + location);
        }
        else
        {
            byteCode.AddOp(OpCode::PUSH_I32);
            byteCode.AddByte(static_cast<Byte>(constantPool.size()));
            constantPool.push_back(flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_I32, value));
        }
        break;
    }
    case TypeCode::Int64: {
        int64_t value = std::get<int64_t>(node->Value());
        if (value == 0)
        {
            byteCode.AddOp(OpCode::PUSH_I64_0);
        }
        else if (value == 1)
        {
            byteCode.AddOp(OpCode::PUSH_I64_1);
        }
        else
        {
            byteCode.AddOp(OpCode::PUSH_I64);
            byteCode.AddByte(static_cast<Byte>(constantPool.size()));
            constantPool.push_back(flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_I64, value));
        }
        break;
    }
    case TypeCode::Float32: {
        float_t value = std::get<float_t>(node->Value());
        if (value == 0.0)
        {
            byteCode.AddOp(OpCode::PUSH_F32_0);
        }
        else if (value == 1.0)
        {
            byteCode.AddOp(OpCode::PUSH_F32_1);
        }
        else
        {
            byteCode.AddOp(OpCode::PUSH_F32);
            byteCode.AddByte(static_cast<Byte>(constantPool.size()));
            constantPool.push_back(flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_F32, value));
        }
        break;
    }
    case TypeCode::Float64: {
        double_t value = std::get<double_t>(node->Value());
        if (value == 0.0)
        {
            byteCode.AddOp(OpCode::PUSH_F64_0);
        }
        else if (value == 1.0)
        {
            byteCode.AddOp(OpCode::PUSH_F64_1);
        }
        else
        {
            byteCode.AddOp(OpCode::PUSH_F64);
            byteCode.AddByte(static_cast<Byte>(constantPool.size()));
            constantPool.push_back(flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_F64, value));
        }
        break;
    }
    case TypeCode::Boolean: {
        if (std::get<bool>(node->Value()))
        {
            byteCode.AddOp(OpCode::PUSH_I32_1);
        }
        else
        {
            byteCode.AddOp(OpCode::PUSH_I32_0);
        }
        break;
    }
    case TypeCode::String: {
        std::u32string value = std::get<std::u32string>(node->Value());
        byteCode.AddOp(OpCode::PUSH_STRING);
        byteCode.AddByte(static_cast<Byte>(constantPool.size()));
        constantPool.push_back(
            flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_STRING, Utility::UTF32ToUTF8(value)));

        break;
    }
    default: {
        /* TODO: other constant types */
        break;
    }
    }
}

void Compiler::VisitParameter(const ParameterExpression *node, ByteCode &byteCode,
                              std::vector<flint_bytecode::Constant> &constantPool)
{
    if (nameLocator.ExistsNameInfo(node, LocationKind::FunctionVariable))
    {
        const NameInfo &nameInfo = nameLocator.GetNameInfo(node, LocationKind::FunctionVariable);
        switch (typeChecker.GetType(node)->GetTypeCode())
        {
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
        case TypeCode::String:
        case TypeCode::Structure:
        case TypeCode::Array:
        case TypeCode::Callable: {
            byteCode.AddOp(OpCode::PUSH_LOCAL_OBJECT);
            byteCode.AddByte(nameInfo.Number());
            break;
        }
        default: {
            throw CompilationException(__FILE__, __LINE__, "Unsupported function variable parameter expression type",
                                       node, nullptr);
        }
        }
    }
    else if (nameLocator.ExistsNameInfo(node, LocationKind::GlobalVariable))
    {
        const NameInfo &nameInfo = nameLocator.GetNameInfo(node, LocationKind::GlobalVariable);
        Byte constantPoolIndex = static_cast<Byte>(constantPool.size());
        constantPool.push_back(flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_GLOBAL_VARIABLE,
                                                        static_cast<int32_t>(nameInfo.Number())));
        switch (typeChecker.GetType(node)->GetTypeCode())
        {
        case TypeCode::Int32:
        case TypeCode::Boolean:
        case TypeCode::Char: {
            byteCode.AddOp(OpCode::PUSH_GLOBAL_I32);
            byteCode.AddByte(constantPoolIndex);
            break;
        }
        case TypeCode::Int64: {
            byteCode.AddOp(OpCode::PUSH_GLOBAL_I64);
            byteCode.AddByte(constantPoolIndex);
            break;
        }
        case TypeCode::Float32: {
            byteCode.AddOp(OpCode::PUSH_GLOBAL_F32);
            byteCode.AddByte(constantPoolIndex);
            break;
        }
        case TypeCode::Float64: {
            byteCode.AddOp(OpCode::PUSH_GLOBAL_F64);
            byteCode.AddByte(constantPoolIndex);
            break;
        }
        case TypeCode::String:
        case TypeCode::Structure:
        case TypeCode::Array:
        case TypeCode::Callable: {
            byteCode.AddOp(OpCode::PUSH_GLOBAL_OBJECT);
            byteCode.AddByte(constantPoolIndex);
            break;
        }
        default: {
            throw CompilationException(__FILE__, __LINE__, "Unsupported global variable parameter expression type",
                                       node, nullptr);
        }
        }
    }
    else if (nameLocator.ExistsNameInfo(node, LocationKind::Function))
    {
        spdlog::info("Pass the function '{}' as a reference.", Utility::UTF32ToUTF8(node->Name()));
        const NameInfo &nameInfo = nameLocator.GetNameInfo(node, LocationKind::Function);
        Byte constantPoolIndex = static_cast<Byte>(constantPool.size());
        constantPool.push_back(flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_FUNCTION,
                                                        static_cast<int32_t>(nameInfo.Number())));
        byteCode.AddOp(OpCode::PUSH_NULL);
        byteCode.AddOp(OpCode::NEW_CLOSURE);
        byteCode.AddByte(constantPoolIndex);
    }
    else
    {
        spdlog::error("Unsupported parameter expression location kind. Parameter name: '{}'.",
                      Utility::UTF32ToUTF8(node->Name()));

        throw CompilationException(__FILE__, __LINE__, "Unsupported parameter expression location kind", node, nullptr);
    }
}

void Compiler::VisitBlock(const BlockExpression *node, ByteCode &byteCode,
                          std::vector<flint_bytecode::Constant> &constantPool)
{
    for (auto expression : node->Expressions())
    {
        Visit(expression, byteCode, constantPool);
    }
}

void Compiler::VisitConditional(const ConditionalExpression *node, ByteCode &byteCode,
                                std::vector<flint_bytecode::Constant> &constantPool)
{
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

    int32_t offset1 = location2 - location1;
    int32_t offset2 = location3 - (location2 + sizeof(int16_t));
    if (offset1 < MIN_JUMP_OFFSET || offset1 > MAX_JUMP_OFFSET ||
        offset2 < MIN_JUMP_OFFSET || offset2 > MAX_JUMP_OFFSET)
    {
        throw CompilationException(__FILE__, __LINE__,
                                   "Jump offset exceeds 16-bit limit. Consider splitting the function.", node,
                                   nullptr);
    }

    spdlog::debug("location 1: {}, location 2: {}, location 3: {}", location1, location2, location3);
    bit_converter::i16_to_bytes(static_cast<int16_t>(offset1), true,
                                byteCode.GetBytes().begin() + location1);

    bit_converter::i16_to_bytes(static_cast<int16_t>(offset2), true,
                                byteCode.GetBytes().begin() + location2);
}

void Compiler::VisitCall(const CallExpression *node, ByteCode &byteCode,
                         std::vector<flint_bytecode::Constant> &constantPool)
{
    const Type *functionType = typeChecker.GetType(node->Function());
    if (functionType->GetTypeCode() == TypeCode::Array)
    {
        Visit(node->Function(), byteCode, constantPool);
        for (const Expression *argument : node->Arguments())
        {
            Visit(argument, byteCode, constantPool);
        }
        const ArrayType *arrayType = static_cast<const ArrayType *>(functionType);
        switch (arrayType->ElementType()->GetTypeCode())
        {
        case TypeCode::Boolean:
        case TypeCode::Char:
        case TypeCode::Int32: {
            byteCode.AddOp(OpCode::PUSH_ARRAY_I32);
            break;
        }
        case TypeCode::Int64: {
            byteCode.AddOp(OpCode::PUSH_ARRAY_I64);
            break;
        }
        case TypeCode::Float32: {
            byteCode.AddOp(OpCode::PUSH_ARRAY_F32);
            break;
        }
        case TypeCode::Float64: {
            byteCode.AddOp(OpCode::PUSH_ARRAY_F64);
            break;
        }
        case TypeCode::String:
        case TypeCode::Structure:
        case TypeCode::Callable:
        case TypeCode::Array: {
            byteCode.AddOp(OpCode::PUSH_ARRAY_OBJECT);
            break;
        }
        default: {
            spdlog::error("Unsupported array element type for array access.");

            throw CompilationException(__FILE__, __LINE__, "Unsupported array element type for array access.", node,
                                       nullptr);
        }
        }
    }
    else
    {
        for (const Expression *argument : node->Arguments())
        {
            Visit(argument, byteCode, constantPool);
        }
        if (node->Function()->NodeType() == ExpressionType::Parameter)
        {
            if (nameLocator.ExistsNameInfo(node->Function(), LocationKind::Function))
            {
                const NameInfo &nameInfo = nameLocator.GetNameInfo(node->Function(), LocationKind::Function);
                byteCode.AddOp(OpCode::INVOKE_FUNCTION);
                byteCode.AddByte(constantPool.size());
                constantPool.push_back(
                    flint_bytecode::Constant(flint_bytecode::ConstantKind::CONSTANT_KIND_FUNCTION, nameInfo.Number()));
            }
            else if (nameLocator.ExistsNameInfo(node->Function(), LocationKind::NativeFunction))
            {
                const NameInfo &nameInfo = nameLocator.GetNameInfo(node->Function(), LocationKind::NativeFunction);
                byteCode.AddOp(OpCode::INVOKE_NATIVE_FUNCTION);
                byteCode.AddByte(constantPool.size());
                constantPool.push_back(flint_bytecode::Constant(
                    flint_bytecode::ConstantKind::CONSTANT_KIND_NATIVE_FUNCTION, nameInfo.Number()));
            }
            else if (nameLocator.ExistsNameInfo(node->Function(), LocationKind::FunctionVariable))
            {
                Visit(node->Function(), byteCode, constantPool);
                byteCode.AddOp(OpCode::INVOKE_CLOSURE);
            }
            else
            {
                spdlog::error("Unsupported call expression location kind.");
                throw CompilationException(__FILE__, __LINE__, "Unsupported call expression location kind.", node,
                                           nullptr);
            }
        }
        else
        {
            spdlog::error("Unsupported call expression type. Got node type: {}",
                          Utility::EnumToString(node->Function()->NodeType()));

            throw CompilationException(__FILE__, __LINE__, "Unsupported call expression type", node, nullptr);
        }
    }
}

void Compiler::VisitLambda(const LambdaExpression *node, ByteCode &byteCode,
                           std::vector<flint_bytecode::Constant> &constantPool)
{
    throw CompilationException(__FILE__, __LINE__, "This function is not implemented", node, nullptr);
}

void Compiler::VisitWhileLoop(const WhileLoopExpression *node, ByteCode &byteCode,
                              std::vector<flint_bytecode::Constant> &constantPool)
{
    flint_bytecode::Label label1 = byteCode.CreateLabel(byteCode.Size());

    Visit(node->Condition(), byteCode, constantPool);
    byteCode.AddOp(OpCode::JUMP_IF_FALSE);
    flint_bytecode::Label label2 = byteCode.CreateLabel(byteCode.Size());
    byteCode.AddI16(0);

    Visit(node->Body(), byteCode, constantPool);
    byteCode.AddOp(OpCode::JUMP);
    int32_t backwardOffset = static_cast<int32_t>(label1.Location()) -
                             (byteCode.Size() + static_cast<int32_t>(sizeof(int16_t)));
    int32_t forwardOffset = byteCode.Size() - static_cast<int32_t>(label2.Location() + sizeof(int16_t));
    if (backwardOffset < MIN_JUMP_OFFSET || backwardOffset > MAX_JUMP_OFFSET ||
        forwardOffset < MIN_JUMP_OFFSET || forwardOffset > MAX_JUMP_OFFSET)
    {
        throw CompilationException(__FILE__, __LINE__,
                                   "Jump offset exceeds 16-bit limit. Consider splitting the function.", node,
                                   nullptr);
    }
    byteCode.AddI16(backwardOffset);

    byteCode.Rewrite(label2, forwardOffset);
}

void Compiler::VisitDefault(const DefaultExpression *node, ByteCode &byteCode,
                            std::vector<flint_bytecode::Constant> &constantPool)
{
    switch (typeChecker.GetType(node)->GetTypeCode())
    {
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
        throw TreeException(__FILE__, __LINE__, "This type is not supported by the default expression.",
                            static_cast<const Expression *>(node), nullptr);
    }
    }
}

void Compiler::VisitVariableDeclaration(const VariableDeclarationExpression *node, ByteCode &byteCode,
                                        std::vector<flint_bytecode::Constant> &constantPool)
{
    NameInfo nameInfo = nameLocator.GetNameInfo(node, LocationKind::FunctionVariable);
    int offset = nameInfo.Number();
    Visit(node->Initializer(), byteCode, constantPool);
    const Type *type = typeChecker.GetType(node);
    switch (type->GetTypeCode())
    {
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
    case TypeCode::String:
    case TypeCode::Structure: {
        byteCode.AddOp(OpCode::POP_LOCAL_OBJECT);
        byteCode.AddByte(static_cast<Byte>(offset));
        break;
    }
    default: {
        spdlog::error("The variable declaration expression does not support this "
                      "type. Variable name: '{}'.",
                      Utility::UTF32ToUTF8(node->Name()));

        throw TreeException(__FILE__, __LINE__, "This type is not supported by the variable declaration expression.",
                            static_cast<const Expression *>(node), nullptr);
    }
    }
}

void Compiler::VisitNew(const NewExpression *node, ByteCode &byteCode,
                        std::vector<flint_bytecode::Constant> &constantPool)
{
    const NameInfo &nameInfo = nameLocator.GetNameInfo(node, LocationKind::Structure);
    byteCode.AddOp(OpCode::NEW);
    byteCode.AddByte(AllocateConstant(constantPool, flint_bytecode::ConstantKind::CONSTANT_KIND_STRUCTURE_META_DATA,
                                      nameInfo.Number()));
    const StructureType *structureType = static_cast<const StructureType *>(typeChecker.GetType(node));
    for (const std::u32string &fieldName : node->FieldsInitialization().GetAllKeys())
    {
        size_t index = structureType->Fields().GetIndexByKey(fieldName);
        const Type *fieldType = structureType->Fields().GetItemByKey(fieldName);
        byteCode.AddOp(OpCode::DUPLICATE);
        const Expression *value = node->FieldsInitialization().GetItemByKey(fieldName);
        Visit(value, byteCode, constantPool);

        switch (fieldType->GetTypeCode())
        {
        case TypeCode::Boolean:
        case TypeCode::Char:
        case TypeCode::Int32: {
            byteCode.AddOp(OpCode::POP_FIELD_I32);
            byteCode.AddByte(static_cast<Byte>(index));
            break;
        }
        case TypeCode::Int64: {
            byteCode.AddOp(OpCode::POP_FIELD_I64);
            byteCode.AddByte(static_cast<Byte>(index));
            break;
        }
        case TypeCode::Float32: {
            byteCode.AddOp(OpCode::POP_FIELD_F32);
            byteCode.AddByte(static_cast<Byte>(index));
            break;
        }
        case TypeCode::Float64: {
            byteCode.AddOp(OpCode::POP_FIELD_F64);
            byteCode.AddByte(static_cast<Byte>(index));
            break;
        }
        case TypeCode::String:
        case TypeCode::Structure: {
            byteCode.AddOp(OpCode::POP_FIELD_OBJECT);
            byteCode.AddByte(static_cast<Byte>(index));
            break;
        }
        default: {
            spdlog::error("Unsupported field type: '{}'.", Utility::EnumToString(fieldType->GetTypeCode()));

            throw CompilationException(__FILE__, __LINE__, "Unsupported field type", node, nullptr);
        }
        }
    }
}

void Compiler::VisitMember(const MemberExpression *node, ByteCode &byteCode,
                           std::vector<flint_bytecode::Constant> &constantPool)
{
    Visit(node->GetExpression(), byteCode, constantPool);
    const Type *type = typeChecker.GetType(node->GetExpression());
    if (type->GetTypeCode() == TypeCode::Structure)
    {
        const StructureType *structureType = static_cast<const StructureType *>(type);
        if (structureType->Fields().ContainsKey(node->FieldName()))
        {
            size_t index = structureType->Fields().GetIndexByKey(node->FieldName());
            const Type *fieldType = structureType->Fields().GetItemByIndex(index);
            switch (fieldType->GetTypeCode())
            {
            case TypeCode::Boolean:
            case TypeCode::Char:
            case TypeCode::Int32: {
                byteCode.AddOp(OpCode::PUSH_FIELD_I32);
                byteCode.AddByte(static_cast<Byte>(index));
                break;
            }
            case TypeCode::Int64: {
                byteCode.AddOp(OpCode::PUSH_FIELD_I64);
                byteCode.AddByte(static_cast<Byte>(index));
                break;
            }
            case TypeCode::Float32: {
                byteCode.AddOp(OpCode::PUSH_FIELD_F32);
                byteCode.AddByte(static_cast<Byte>(index));
                break;
            }
            case TypeCode::Float64: {
                byteCode.AddOp(OpCode::PUSH_FIELD_F64);
                byteCode.AddByte(static_cast<Byte>(index));
                break;
            }
            case TypeCode::String:
            case TypeCode::Structure: {
                byteCode.AddOp(OpCode::PUSH_FIELD_OBJECT);
                byteCode.AddByte(static_cast<Byte>(index));
                break;
            }
            default: {
                spdlog::error("Unsupported field type: '{}'.", Utility::EnumToString(fieldType->GetTypeCode()));

                throw CompilationException(__FILE__, __LINE__, "Unsupported field type", node, nullptr);
            }
            }
        }
        else
        {
            spdlog::error("Field '{}' doesn't exist.", Utility::UTF32ToUTF8(node->FieldName()));

            throw CompilationException(__FILE__, __LINE__, "Field doesn't exist.", node, nullptr);
        }
    }
    else
    {
        spdlog::error("Unsupported type for member access: '{}'.", Utility::EnumToString(type->GetTypeCode()));

        throw CompilationException(__FILE__, __LINE__, "Unsupported type for member access.", node, nullptr);
    }
}

void Compiler::CompileAssignment(const BinaryExpression *node, ByteCode &byteCode,
                                 std::vector<flint_bytecode::Constant> &constantPool)
{
    if (node->Left()->NodeType() == ExpressionType::Call)
    {
        auto callExpression = static_cast<const CallExpression *>(node->Left());
        const Type *functionType = typeChecker.GetType(callExpression->Function());
        if (functionType->GetTypeCode() == TypeCode::Array)
        {
            Visit(callExpression->Function(), byteCode, constantPool);
            Visit(callExpression->Arguments().front(), byteCode, constantPool);
            Visit(node->Right(), byteCode, constantPool);

            const ArrayType *arrayType = static_cast<const ArrayType *>(functionType);
            switch (arrayType->ElementType()->GetTypeCode())
            {
            case TypeCode::Boolean:
            case TypeCode::Char:
            case TypeCode::Int32: {
                byteCode.AddOp(OpCode::POP_ARRAY_I32);
                break;
            }
            case TypeCode::Int64: {
                byteCode.AddOp(OpCode::POP_ARRAY_I64);
                break;
            }
            case TypeCode::Float32: {
                byteCode.AddOp(OpCode::POP_ARRAY_F32);
                break;
            }
            case TypeCode::Float64: {
                byteCode.AddOp(OpCode::POP_ARRAY_F64);
                break;
            }
            case TypeCode::String:
            case TypeCode::Structure:
            case TypeCode::Callable:
            case TypeCode::Array: {
                byteCode.AddOp(OpCode::POP_ARRAY_OBJECT);
                break;
            }
            default: {
                spdlog::error("Unsupported array element type for array assignment.");

                throw CompilationException(__FILE__, __LINE__, "Unsupported array element type for array assignment.",
                                           node, nullptr);
            }
            }
            return; // Array assignment handled, exit the function
        }
        else
        {
            throw CompilationException(__FILE__, __LINE__,
                                       "The left-hand side of the assignment is a call expression, but it's not an "
                                       "array access. It is currently not supported.",
                                       node, nullptr);
        }
    }
    else
    {
        TypeCode typeCode = typeChecker.GetType(node->Left())->GetTypeCode();
        if (typeCode != typeChecker.GetType(node->Right())->GetTypeCode())
        {
            spdlog::error("Assignment type mismatch.");

            throw TreeException(__FILE__, __LINE__, "Assignment type mismatch.", static_cast<const Expression *>(node),
                                nullptr);
        }
        else
        {
            Visit(node->Right(), byteCode, constantPool);
            if (node->Left()->NodeType() == ExpressionType::Parameter)
            {
                if (nameLocator.ExistsNameInfo(node->Left(), LocationKind::FunctionVariable))
                {
                    NameInfo nameInfo = nameLocator.GetNameInfo(node->Left(), LocationKind::FunctionVariable);
                    int offset = nameInfo.Number();
                    switch (typeCode)
                    {
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
                    case TypeCode::String:
                    case TypeCode::Structure:
                    case TypeCode::Array:
                    case TypeCode::Callable: {
                        byteCode.AddOp(OpCode::POP_LOCAL_OBJECT);
                        byteCode.AddByte(static_cast<Byte>(offset));
                        break;
                    }
                    default: {
                        spdlog::error("Assignment to the local variable of this type is not supported.");

                        throw CompilationException(__FILE__, __LINE__,
                                                   "Assignment to the local variable of this type is not supported.",
                                                   static_cast<const Expression *>(node), nullptr);
                    }
                    }
                }
                else if (nameLocator.ExistsNameInfo(node->Left(), LocationKind::GlobalVariable))
                {
                    NameInfo nameInfo = nameLocator.GetNameInfo(node->Left(), LocationKind::GlobalVariable);
                    int offset = nameInfo.Number();
                    switch (typeCode)
                    {
                    case TypeCode::Boolean:
                    case TypeCode::Char:
                    case TypeCode::Int32: {
                        byteCode.AddOp(OpCode::POP_GLOBAL_I32);
                        byteCode.AddByte(static_cast<Byte>(offset));
                        break;
                    }
                    case TypeCode::Int64: {
                        byteCode.AddOp(OpCode::POP_GLOBAL_I64);
                        byteCode.AddByte(static_cast<Byte>(offset));
                        break;
                    }
                    case TypeCode::Float32: {
                        byteCode.AddOp(OpCode::POP_GLOBAL_F32);
                        byteCode.AddByte(static_cast<Byte>(offset));
                        break;
                    }
                    case TypeCode::Float64: {
                        byteCode.AddOp(OpCode::POP_GLOBAL_F64);
                        byteCode.AddByte(static_cast<Byte>(offset));
                        break;
                    }
                    case TypeCode::String:
                    case TypeCode::Structure:
                    case TypeCode::Array:
                    case TypeCode::Callable: {
                        byteCode.AddOp(OpCode::POP_GLOBAL_OBJECT);
                        byteCode.AddByte(static_cast<Byte>(offset));
                        break;
                    }
                    default: {
                        spdlog::error("Assignment to the global variable of this type is not supported.");

                        throw CompilationException(__FILE__, __LINE__,
                                                   "Assignment to the global variable of this type is not supported.",
                                                   static_cast<const Expression *>(node), nullptr);
                    }
                    }
                }
                else
                {
                    spdlog::error("Assignments only work for variables and structure fields.");

                    throw CompilationException(
                        __FILE__, __LINE__, "Assignments only work for variables and structure fields.", node, nullptr);
                }
            }
            else if (node->Left()->NodeType() == ExpressionType::MemberAccess)
            {
                const MemberExpression *memberAccess = static_cast<const MemberExpression *>(node->Left());
                Visit(memberAccess->GetExpression(), byteCode, constantPool);
                Visit(node->Right(), byteCode, constantPool);
                const Type *type = typeChecker.GetType(memberAccess->GetExpression());
                if (type->GetTypeCode() == TypeCode::Structure)
                {
                    const StructureType *structureType = static_cast<const StructureType *>(type);
                    if (structureType->Fields().ContainsKey(memberAccess->FieldName()))
                    {
                        size_t index = structureType->Fields().GetIndexByKey(memberAccess->FieldName());
                        const Type *fieldType = structureType->Fields().GetItemByIndex(index);
                        switch (fieldType->GetTypeCode())
                        {
                        case TypeCode::Boolean:
                        case TypeCode::Char:
                        case TypeCode::Int32: {
                            byteCode.AddOp(OpCode::POP_FIELD_I32);
                            byteCode.AddByte(static_cast<Byte>(index));
                            break;
                        }
                        case TypeCode::Int64: {
                            byteCode.AddOp(OpCode::POP_FIELD_I64);
                            byteCode.AddByte(static_cast<Byte>(index));
                            break;
                        }
                        case TypeCode::Float32: {
                            byteCode.AddOp(OpCode::POP_FIELD_F32);
                            byteCode.AddByte(static_cast<Byte>(index));
                            break;
                        }
                        case TypeCode::Float64: {
                            byteCode.AddOp(OpCode::POP_FIELD_F64);
                            byteCode.AddByte(static_cast<Byte>(index));
                            break;
                        }
                        case TypeCode::String:
                        case TypeCode::Structure: {
                            byteCode.AddOp(OpCode::POP_FIELD_OBJECT);
                            byteCode.AddByte(static_cast<Byte>(index));
                            break;
                        }
                        default: {
                            spdlog::error("Unsupported field type: '{}'.",
                                          Utility::EnumToString(fieldType->GetTypeCode()));

                            throw CompilationException(__FILE__, __LINE__, "Unsupported field type", node, nullptr);
                        }
                        }
                    }
                    else
                    {
                        spdlog::error("Field '{}' doesn't exist.", Utility::UTF32ToUTF8(memberAccess->FieldName()));

                        throw CompilationException(__FILE__, __LINE__, "Field doesn't exist.", node, nullptr);
                    }
                }
                else
                {
                    spdlog::error("Assignments only work for variables and structure fields.");

                    throw CompilationException(
                        __FILE__, __LINE__, "Assignments only work for variables and structure fields.", node, nullptr);
                }
            }
        }
    }
}

flint_bytecode::Function Compiler::CompileFunction(const std::string &name, const LambdaExpression *node)
{
    spdlog::info("Compile function \"{}\".", name);
    ByteCode byteCode;
    std::vector<flint_bytecode::Constant> constantPool;
    Visit(node->Body(), byteCode, constantPool);

    if (name == "Main")
    {
        byteCode.AddOp(OpCode::HALT);
    }
    else
    {
        const auto *callableType = static_cast<const CallableType *>(typeChecker.GetType(node));
        const Type *returnType = callableType->GetReturnType();
        switch (returnType->GetTypeCode())
        {
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
    Byte locals = static_cast<Byte>(localVariableCount - static_cast<int>(node->Parameters().size()));
    spdlog::info("Finish compiling function \"{}\".", name);
    return flint_bytecode::Function(name, 0, locals, node->Parameters().size(), constantPool, byteCode);
}

flint_bytecode::NativeFunction Compiler::CompileNativeFunction(const std::string &name, const LambdaExpression *node)
{
    int argsSize = static_cast<int>(node->Parameters().size());
    std::u32string libraryName;
    std::u32string entryPoint;
    bool foundLibraryName = false;
    bool foundEntryPoint = false;
    for (const auto &annotation : node->Annotations())
    {
        if (annotation.Name() == U"External")
        {
            for (const auto &arg : annotation.Arguments())
            {
                if (arg.Name() == U"Library")
                {
                    if (foundLibraryName)
                    {
                        spdlog::error("Library name was defined more than once.");

                        throw TreeException(__FILE__, __LINE__, "Library name was defined more than once.",
                                            static_cast<const Expression *>(node), nullptr);
                    }
                    else
                    {
                        foundLibraryName = true;
                        libraryName = std::get<std::u32string>(arg.Value());
                    }
                }
                else if (arg.Name() == U"EntryPoint")
                {
                    if (foundEntryPoint)
                    {
                        spdlog::error("Entry point was defined more than once.");

                        throw TreeException(__FILE__, __LINE__, "Entry point was defined more than once.",
                                            static_cast<const Expression *>(node), nullptr);
                    }
                    else
                    {
                        foundEntryPoint = true;
                        entryPoint = std::get<std::u32string>(arg.Value());
                    }
                }
            }
        }
    }
    if (foundLibraryName == false)
    {
        spdlog::error("Can't find library name in the annotations of the native "
                      "function definition.");

        throw TreeException(__FILE__, __LINE__,
                            "Can't find library name in the annotations of the native "
                            "function definition.",
                            static_cast<const Expression *>(node), nullptr);
    }
    if (foundEntryPoint == false)
    {
        spdlog::error("Can't find entry point in the annotations of the native "
                      "function definition.");

        throw TreeException(__FILE__, __LINE__,
                            "Can't find entry point in the annotations of the native "
                            "function definition.",
                            static_cast<const Expression *>(node), nullptr);
    }
    if (!libraryMap.ContainsKey(libraryName))
    {
        flint_bytecode::NativeLibrary nativeLibrary(Utility::UTF32ToUTF8(libraryName));
        libraryMap.AddItem(libraryName, nativeLibrary);
    }

    int nativeLibraryOffset = static_cast<int>(libraryMap.GetIndexByKey(libraryName));
    return flint_bytecode::NativeFunction(Utility::UTF32ToUTF8(entryPoint), argsSize, nativeLibraryOffset);
}

std::vector<flint_bytecode::NativeLibrary> Compiler::GetNativeLibraries()
{
    return libraryMap.GetAllItems();
}

void Compiler::CompileNamespace(std::vector<flint_bytecode::GlobalVariable> &globalVariables,
                                std::vector<flint_bytecode::Function> &functions,
                                std::vector<flint_bytecode::NativeFunction> &nativeFunctions)
{
    Namespace *top = namespaceStack.top();

    for (const auto &varDecl : top->GlobalVariables().GetAllItems())
    {
        std::u32string name = varDecl->Name() + U"#Initializer";
        if (top->Functions().ContainsKey(name))
        {
            int globalVariableIndex = nameLocator.GetNameInfo(varDecl, LocationKind::GlobalVariable).Number();
            LambdaExpression *initializer = top->Functions().GetItemByKey(name);
            int initializerIndex = nameLocator.GetNameInfo(initializer, LocationKind::Function).Number();
            globalVariables.at(globalVariableIndex) =
                flint_bytecode::GlobalVariable(Utility::UTF32ToUTF8(varDecl->Name()), initializerIndex);
        }
        else
        {
            throw TreeException(__FILE__, __LINE__, "Global variable initializer is missing.",
                                static_cast<const Expression *>(varDecl), nullptr);
        }
    }

    for (const auto &funcDecl : top->Functions().GetAllItems())
    {
        if (funcDecl->IsNativeFunction())
        {
            auto function = CompileNativeFunction(Utility::UTF32ToUTF8(funcDecl->Name()), funcDecl);
            int index = nameLocator.GetNameInfo(funcDecl, LocationKind::NativeFunction).Number();
            nativeFunctions.at(index) = function;
        }
        else
        {
            auto function = CompileFunction(Utility::UTF32ToUTF8(funcDecl->Name()), funcDecl);
            int index = nameLocator.GetNameInfo(funcDecl, LocationKind::Function).Number();
            functions.at(index) = function;

            if (funcDecl->Name() == U"Main")
            {
                if (mainFunctionIndex >= 0)
                {
                    throw TreeException(__FILE__, __LINE__,
                                        "Multiple 'Main' functions are defined within "
                                        "the same module or across different modules.",
                                        static_cast<const Expression *>(funcDecl), nullptr);
                }
                else
                {
                    mainFunctionIndex = index;
                }
            }
        }
    }

    for (const auto &current : top->Children().GetAllItems())
    {
        namespaceStack.push(current);
        CompileNamespace(globalVariables, functions, nativeFunctions);
        namespaceStack.pop();
        spdlog::info("Finish compiling namespace \"{}\".", Utility::UTF32ToUTF8(current->Name()));
    }
}

int Compiler::EntryPoint() const
{
    if (mainFunctionIndex >= 0)
    {
        return mainFunctionIndex;
    }
    else
    {
        throw TreeException(__FILE__, __LINE__, "'Main' function is not defined.", nullptr, nullptr);
    }
}

Byte Compiler::AllocateConstant(std::vector<flint_bytecode::Constant> &constantPool,
                                flint_bytecode::ConstantKind constantKind, std::any value)
{
    if (constantPool.size() >= MAX_CONSTANT_POOL_SIZE)
    {
        spdlog::error("Constant pool size exceeds upper limit.");

        throw TreeException(__FILE__, __LINE__, "Constant pool size exceeds upper limit.", nullptr, nullptr);
    }
    else
    {
        Byte size = static_cast<Byte>(constantPool.size());
        constantPool.push_back(flint_bytecode::Constant(constantKind, value));

        return size;
    }
}

void Compiler::CompileLogicalAnd(const BinaryExpression *node, ByteCode &byteCode,
                                 std::vector<flint_bytecode::Constant> &constantPool)
{
    Visit(node->Left(), byteCode, constantPool);
    byteCode.AddOp(OpCode::JUMP_IF_FALSE);
    int32_t location1 = static_cast<int32_t>(byteCode.GetBytes().size());
    byteCode.Extend(sizeof(int16_t));

    Visit(node->Right(), byteCode, constantPool);
    byteCode.AddOp(OpCode::JUMP);
    int32_t location2 = static_cast<int32_t>(byteCode.GetBytes().size());
    byteCode.Extend(sizeof(int16_t));

    int32_t location3 = static_cast<int32_t>(byteCode.GetBytes().size());
    byteCode.AddOp(OpCode::PUSH_I32_0);
    int32_t location4 = static_cast<int32_t>(byteCode.GetBytes().size());

    int32_t offset1 = location3 - (location1 + sizeof(int16_t));
    int32_t offset2 = location4 - (location2 + sizeof(int16_t));
    if (offset1 < MIN_JUMP_OFFSET || offset1 > MAX_JUMP_OFFSET ||
        offset2 < MIN_JUMP_OFFSET || offset2 > MAX_JUMP_OFFSET)
    {
        throw CompilationException(__FILE__, __LINE__,
                                   "Jump offset exceeds 16-bit limit. Consider splitting the expression.", node,
                                   nullptr);
    }

    spdlog::info("logical and: location 1: {}, location 2: {}, location 3: {}, location 4: {}", location1, location2,
                 location3, location4);
    bit_converter::i16_to_bytes(static_cast<int16_t>(offset1), true,
                                byteCode.GetBytes().begin() + location1);

    bit_converter::i16_to_bytes(static_cast<int16_t>(offset2), true,
                                byteCode.GetBytes().begin() + location2);
}
void Visitors::Compiler::CompileLogicalOr(const BinaryExpression *node, ByteCode &byteCode,
                                          std::vector<flint_bytecode::Constant> &constantPool)
{
    Visit(node->Left(), byteCode, constantPool);
    byteCode.AddOp(OpCode::JUMP_IF_TRUE);
    int32_t location1 = static_cast<int32_t>(byteCode.GetBytes().size());
    byteCode.Extend(sizeof(int16_t));

    Visit(node->Right(), byteCode, constantPool);
    byteCode.AddOp(OpCode::JUMP);
    int32_t location2 = static_cast<int32_t>(byteCode.GetBytes().size());
    byteCode.Extend(sizeof(int16_t));

    int32_t location3 = static_cast<int32_t>(byteCode.GetBytes().size());
    byteCode.AddOp(OpCode::PUSH_I32_1);
    int32_t location4 = static_cast<int32_t>(byteCode.GetBytes().size());

    int32_t offset1 = location3 - (location1 + sizeof(int16_t));
    int32_t offset2 = location4 - (location2 + sizeof(int16_t));
    if (offset1 < MIN_JUMP_OFFSET || offset1 > MAX_JUMP_OFFSET ||
        offset2 < MIN_JUMP_OFFSET || offset2 > MAX_JUMP_OFFSET)
    {
        throw CompilationException(__FILE__, __LINE__,
                                   "Jump offset exceeds 16-bit limit. Consider splitting the expression.", node,
                                   nullptr);
    }

    spdlog::info("logical or: location 1: {}, location 2: {}, location 3: {}, location 4: {}", location1, location2,
                 location3, location4);
    bit_converter::i16_to_bytes(static_cast<int16_t>(offset1), true,
                                byteCode.GetBytes().begin() + location1);

    bit_converter::i16_to_bytes(static_cast<int16_t>(offset2), true,
                                byteCode.GetBytes().begin() + location2);
}
}; /* namespace Visitors */
}; // namespace Cygni