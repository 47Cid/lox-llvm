#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <iostream>
#include <map>

class Environment {
public:
  llvm::Function *Function;
  llvm::BasicBlock *ExitBlock;
  std::unique_ptr<Environment> EnclosingEnv;
  std::map<std::string, llvm::AllocaInst *> Locals;
  Environment(llvm::Function *Function, llvm::BasicBlock *ExitBlock,
              std::unique_ptr<Environment> EnclosingEnv)
      : Function(Function), ExitBlock(ExitBlock),
        EnclosingEnv(std::move(EnclosingEnv)) {}
  llvm::AllocaInst *createEntryBlockAlloca(std::string Identifer);
  llvm::StoreInst *storeLocal(std::string Identifer, llvm::Value *Value);
  llvm::StoreInst *assignLocal(std::string Identifer, llvm::Value *Value);
  llvm::LoadInst *getLocal(std::string Identifer);
  ~Environment();

private:
  llvm::AllocaInst *getAlloca(std::string Identifer);
};

#endif
