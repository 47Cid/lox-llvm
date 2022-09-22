#include "codegen.hpp"

#include <fstream>
#include <iostream>

bool CodeGen::HadError = false;
std::unique_ptr<llvm::LLVMContext> CodeGen::TheContext =
    std::make_unique<llvm::LLVMContext>();
std::unique_ptr<llvm::Module> CodeGen::TheModule =
    std::make_unique<llvm::Module>("My Module", *TheContext);
std::unique_ptr<llvm::IRBuilder<>> CodeGen::Builder =
    std::make_unique<llvm::IRBuilder<>>(*TheContext);
std::map<std::string, llvm::Value *> CodeGen::NamedValues;
std::unique_ptr<Environment> CodeGen::CurrEnv =
    std::make_unique<Environment>(nullptr, nullptr, nullptr);

llvm::Value *CodeGen::error(std::string Message) {
  HadError = true;
  std::cerr << Message << std::endl;
  return nullptr;
}

llvm::Value *CodeGen::insertExtFunctions() {
  std::vector<llvm::Type *> Doubles(
      1, llvm::Type::getDoubleTy(*CodeGen::TheContext));
  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*CodeGen::TheContext), Doubles, false);
  TheModule->getOrInsertFunction("printd", FT);
  return nullptr;
}

int CodeGen::generateIR() {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << *TheModule;
  OS.flush();
  // Write IR to a file
  std::ofstream file("IR/module.ll");
  file << Str;
  file.close();
  return 0;
}
