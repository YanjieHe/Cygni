#ifndef FLINT_BYTE_CODE_BYTE_CODE_HPP
#define FLINT_BYTE_CODE_BYTE_CODE_HPP

#include <string>
#include <vector>
#include <any>
#include <stdexcept>
#include <cstdint>
#include <cmath>
#include "OpCode.hpp"

namespace flint_bytecode {
using Byte = uint8_t;

class Label {
private:
  size_t id;
  size_t location;

public:
  Label() : id{0}, location{0} {}
  Label(size_t id, size_t location) : id{id}, location{location} {}

  size_t ID() const { return id; }
  size_t Location() const { return location; }
};

class ByteCode {
private:
  std::vector<Byte> bytes;
  std::vector<Label> labels;

public:
  ByteCode() = default;

  void AddOp(OpCode op);
  void AddByte(Byte b);
  void AddI32(int32_t value);
  void AddI64(int64_t value);
  void AddF32(float_t value);
  void AddF64(double_t value);
  void AddU16(uint16_t value);
  void AddI16(int16_t value);
  void AddString(const std::string &str);
  int32_t Size() const;
  const std::vector<Byte> &GetBytes() const { return bytes; }
  std::vector<Byte> &GetBytes() { return bytes; }
  void Extend(size_t size);
  const Label &CreateLabel(size_t location);
  void Rewrite(const Label &label, int16_t value);
  bool OutputToFile(const std::string &fileName);
};

class ICompilable {
  virtual void Compile(ByteCode &byteCode) = 0;
};

enum class ConstantKind {
  CONSTANT_KIND_I32 = 0,
  CONSTANT_KIND_I64 = 1,
  CONSTANT_KIND_F32 = 2,
  CONSTANT_KIND_F64 = 3,
  CONSTANT_KIND_STRING = 4,
  CONSTANT_KIND_FUNCTION = 5,
  CONSTANT_KIND_STRUCTURE_META_DATA = 6,
  CONSTANT_KIND_GLOBAL_VARIABLE = 7,
  CONSTANT_KIND_NATIVE_FUNCTION = 8
};

class Constant : public ICompilable {
public:
  Constant(ConstantKind constantKind, std::any value)
      : constantKind{constantKind}, value{value} {}

  void Compile(ByteCode &byteCode) override;

  ConstantKind GetConstantKind() const { return constantKind; }
  const std::any &GetValue() const { return value; }

private:
  ConstantKind constantKind;
  std::any value;
};

class GlobalVariable : public ICompilable {
public:
  GlobalVariable(std::string name, int initializerOffset)
      : name{name}, initializerOffset{initializerOffset} {}

  void Compile(ByteCode &byteCode) override;

private:
  std::string name;
  int initializerOffset;
};

class StructureMeta : public ICompilable {
public:
  StructureMeta(std::string name, std::vector<std::string> fieldNames)
      : name{name}, fieldNames{fieldNames} {}

  void Compile(ByteCode &byteCode) override;

private:
  std::string name;
  std::vector<std::string> fieldNames;
};

class Function : public ICompilable {
public:
  Function(std::string name, Byte stack, Byte locals, Byte argsSize,
           std::vector<Constant> constantPool, ByteCode code)
      : name{name}, stack{stack}, locals{locals}, argsSize{argsSize},
        constantPool{constantPool}, code{code} {}

  void Compile(ByteCode &byteCode) override;

private:
  std::string name;
  Byte stack;
  Byte locals;
  Byte argsSize;
  std::vector<Constant> constantPool;
  ByteCode code;
};

class NativeLibrary : public ICompilable {
public:
  NativeLibrary(std::string libraryPath) { this->libraryPath = libraryPath; }
  void Compile(ByteCode &byteCode) override;

private:
  std::string libraryPath;
};

class NativeFunction : public ICompilable {
public:
  NativeFunction(std::string functionName, uint16_t argsSize,
                 int32_t nativeLibraryOffset)
      : functionName{functionName}, argsSize{argsSize},
        nativeLibraryOffset{nativeLibraryOffset} {}
  void Compile(ByteCode &byteCode) override;

private:
  std::string functionName;
  uint16_t argsSize;
  int32_t nativeLibraryOffset;
};

class ByteCodeProgram : public ICompilable {
public:
  ByteCodeProgram(std::vector<GlobalVariable> globalVariables,
                  std::vector<StructureMeta> structures,
                  std::vector<Function> functions,
                  std::vector<NativeLibrary> nativeLibraries,
                  std::vector<NativeFunction> nativeFunctions, int entryPoint)
      : globalVariables{globalVariables}, structures{structures},
        functions{functions}, nativeLibraries{nativeLibraries},
        nativeFunctions{nativeFunctions}, entryPoint{entryPoint} {}
  void Compile(ByteCode &byte_code);

private:
  std::vector<GlobalVariable> globalVariables;
  std::vector<StructureMeta> structures;
  std::vector<Function> functions;
  std::vector<NativeLibrary> nativeLibraries;
  std::vector<NativeFunction> nativeFunctions;
  int entryPoint;
};

}; /* namespace flint_bytecode */

#endif /* FLINT_BYTE_CODE_BYTE_CODE_HPP */