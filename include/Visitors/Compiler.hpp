#ifndef CYGNI_VISITORS_COMPILER_HPP
#define CYGNI_VISITORS_COMPILER_HPP

#include "Visitors/Visitor.hpp"
#include "Visitors/Scope.hpp"
#include "Visitors/TypeChecker.hpp"
#include "Visitors/NameLocator.hpp"
#include "FlintByteCode/ByteCode.hpp"
#include "Utility/OrderPreservingMap.hpp"

namespace Cygni {
namespace Visitors {

using flint_bytecode::ByteCode;
using Utility::OrderPreservingMap;

class ConstantPool {
private:
  std::vector<flint_bytecode::Constant> constants;

public:
  /* TODO: keep only unique constants */
};

class Compiler
    : public ExpressionVisitor<void, ByteCode &,
                               std::vector<flint_bytecode::Constant> &> {
private:
  TypeChecker &typeChecker;
  NameLocator &nameLocator;
  NamespaceFactory &namespaceFactory;
  std::stack<Namespace *> namespaceStack;
  int mainFunctionIndex;
  OrderPreservingMap<std::u32string, flint_bytecode::NativeLibrary> libraryMap;

public:
  Compiler(TypeChecker &typeChecker, NameLocator &nameLocator,
           NamespaceFactory &namespaceFactory);

  void
  VisitBinary(const BinaryExpression *node, ByteCode &byteCode,
              std::vector<flint_bytecode::Constant> &constantPool) override;
  void VisitUnary(const UnaryExpression *node, ByteCode &byteCode,
                  std::vector<flint_bytecode::Constant> &constantPool) override;
  void
  VisitConstant(const ConstantExpression *node, ByteCode &byteCode,
                std::vector<flint_bytecode::Constant> &constantPool) override;
  void
  VisitParameter(const ParameterExpression *node, ByteCode &byteCode,
                 std::vector<flint_bytecode::Constant> &constantPool) override;
  void VisitBlock(const BlockExpression *node, ByteCode &byteCode,
                  std::vector<flint_bytecode::Constant> &constantPool) override;
  void VisitConditional(
      const ConditionalExpression *node, ByteCode &byteCode,
      std::vector<flint_bytecode::Constant> &constantPool) override;
  void VisitCall(const CallExpression *node, ByteCode &byteCode,
                 std::vector<flint_bytecode::Constant> &constantPool) override;
  void
  VisitLambda(const LambdaExpression *node, ByteCode &byteCode,
              std::vector<flint_bytecode::Constant> &constantPool) override;
  void VisitWhileLoop(const WhileLoopExpression *node, ByteCode &byteCode,
                 std::vector<flint_bytecode::Constant> &constantPool) override;
  void
  VisitDefault(const DefaultExpression *node, ByteCode &byteCode,
               std::vector<flint_bytecode::Constant> &constantPool) override;

  void VisitVariableDeclaration(
      const VariableDeclarationExpression *node, ByteCode &byteCode,
      std::vector<flint_bytecode::Constant> &constantPool) override;
  
  void CompileAssignment(const BinaryExpression* node, ByteCode& byteCode, std::vector<flint_bytecode::Constant>& constantPool);

  flint_bytecode::Function CompileFunction(const std::string &name,
                                           const LambdaExpression *node);
  flint_bytecode::NativeFunction
  CompileNativeFunction(const std::string &name, const LambdaExpression *node);

  std::vector<flint_bytecode::NativeLibrary> GetNativeLibraries();

  void CompileNamespace(
      std::vector<flint_bytecode::Function> &functions,
      std::vector<flint_bytecode::NativeFunction> &nativeFunctions);

  int EntryPoint() const;
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_COMPILER_HPP */