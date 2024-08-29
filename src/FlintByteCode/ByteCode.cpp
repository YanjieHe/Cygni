#include "ByteCode.hpp"
#include <iostream>
#include <fstream>
#include <bit_converter/bit_converter.hpp>

namespace flint_bytecode {
void ByteCode::AddOp(OpCode op) { bytes.push_back(static_cast<Byte>(op)); }

void ByteCode::AddByte(Byte b) { bytes.push_back(b); }

void ByteCode::AddI32(int32_t value) {
  Extend(sizeof(int32_t));
  bit_converter::i32_to_bytes(value, true, bytes.end() - sizeof(int32_t));
}

void ByteCode::AddI64(int64_t value) {
  Extend(sizeof(int64_t));
  bit_converter::i64_to_bytes(value, true, bytes.end() - sizeof(int64_t));
}

void ByteCode::AddF32(float_t value) {
  Extend(sizeof(float_t));
  bit_converter::f32_to_bytes(value, true, bytes.end() - sizeof(float_t));
}

void ByteCode::AddF64(double_t value) {
  Extend(sizeof(double_t));
  bit_converter::f64_to_bytes(value, true, bytes.end() - sizeof(double_t));
}

void ByteCode::AddU16(uint16_t value) {
  Extend(sizeof(uint16_t));
  bit_converter::u16_to_bytes(value, true, bytes.end() - sizeof(uint16_t));
}

void ByteCode::AddI16(int16_t value) {
  Extend(sizeof(int16_t));
  bit_converter::i16_to_bytes(value, true, bytes.end() - sizeof(int16_t));
}

void ByteCode::AddString(const std::string &str) {
  if (str.size() < std::numeric_limits<uint16_t>::max()) {
    AddU16(static_cast<uint16_t>(str.size()));
    for (char c : str) {
      bytes.push_back(static_cast<Byte>(c));
    }
  } else {
    throw std::invalid_argument("The string literal is too long.");
  }
}

int32_t ByteCode::Size() const { return static_cast<int32_t>(bytes.size()); }

void ByteCode::Extend(size_t size) {
  for (size_t i = 0; i < size; i++) {
    bytes.push_back(0);
  }
}

const Label &ByteCode::CreateLabel(size_t location) {
  labels.push_back(Label(labels.size(), location));
  return labels.back();
}

void ByteCode::Rewrite(const Label &label, int16_t value) {
  bit_converter::i16_to_bytes(value, true, bytes.begin() + label.Location());
}

bool ByteCode::OutputToFile(const std::string &fileName) {
  std::ofstream stream(fileName, std::ios::binary);

  if (stream) {
    stream.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
    stream.close();

    return true;
  } else {

    return false;
  }
}

void Constant::Compile(ByteCode &byteCode) {
  byteCode.AddByte(static_cast<Byte>(this->constantKind));
  switch (this->constantKind) {
  case ConstantKind::CONSTANT_KIND_I32:
  case ConstantKind::CONSTANT_KIND_FUNCTION:
  case ConstantKind::CONSTANT_KIND_GLOBAL_VARIABLE:
  case ConstantKind::CONSTANT_KIND_STRUCTURE_META_DATA:
  case ConstantKind::CONSTANT_KIND_NATIVE_FUNCTION: {
    byteCode.AddI32(std::any_cast<int32_t>(this->value));
    break;
  }
  case ConstantKind::CONSTANT_KIND_I64: {
    byteCode.AddI64(std::any_cast<int64_t>(this->value));
    break;
  }
  case ConstantKind::CONSTANT_KIND_F32: {
    byteCode.AddF32(std::any_cast<float_t>(this->value));
    break;
  }
  case ConstantKind::CONSTANT_KIND_F64: {
    byteCode.AddF64(std::any_cast<double_t>(this->value));
    break;
  }
  case ConstantKind::CONSTANT_KIND_STRING: {
    byteCode.AddString(std::any_cast<std::string>(this->value));
    break;
  }
  default: { throw std::invalid_argument("constant kind"); }
  }
}

void GlobalVariable::Compile(ByteCode &byteCode) {
  byteCode.AddString(this->name);
  byteCode.AddI32(this->initializerOffset);
}

void StructureMeta::Compile(ByteCode &byteCode) {
  byteCode.AddString(this->name);
  byteCode.AddU16(this->fieldNames.size());
  for (const auto &fieldName : this->fieldNames) {
    byteCode.AddString(fieldName);
  }
}

void Function::Compile(ByteCode &byteCode) {
  byteCode.AddString(this->name);
  byteCode.AddByte(this->stack);
  byteCode.AddByte(this->locals);
  byteCode.AddByte(this->argsSize);
  byteCode.AddByte(static_cast<Byte>(this->constantPool.size()));
  for (auto &constant : this->constantPool) {
    constant.Compile(byteCode);
  }
  byteCode.AddU16(static_cast<uint16_t>(this->code.Size()));
  for (const auto &b : this->code.GetBytes()) {
    byteCode.AddByte(b);
  }
}

void NativeLibrary::Compile(ByteCode &byteCode) {
  byteCode.AddString(this->libraryPath);
}

void NativeFunction::Compile(ByteCode &byteCode) {
  byteCode.AddString(this->functionName);
  byteCode.AddU16(this->argsSize);
  byteCode.AddI32(this->nativeLibraryOffset);
}

void ByteCodeProgram::Compile(ByteCode &byte_code) {
  byte_code.AddI32(this->globalVariables.size());
  byte_code.AddI32(this->structures.size());
  byte_code.AddI32(this->functions.size());
  byte_code.AddI32(this->nativeLibraries.size());
  byte_code.AddI32(this->nativeFunctions.size());
  byte_code.AddI32(this->entryPoint);

  for (auto &globalVariable : this->globalVariables) {
    globalVariable.Compile(byte_code);
  }

  for (auto &structureMeta : this->structures) {
    structureMeta.Compile(byte_code);
  }

  for (auto &function : this->functions) {
    function.Compile(byte_code);
  }

  for (auto &nativeLib : this->nativeLibraries) {
    nativeLib.Compile(byte_code);
  }

  for (auto &nativeFunction : this->nativeFunctions) {
    nativeFunction.Compile(byte_code);
  }
}

}; /* namespace flint_bytecode */
