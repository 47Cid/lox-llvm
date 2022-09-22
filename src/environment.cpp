#include "environment.hpp"

#include "codegen.hpp"

llvm::AllocaInst* Environment::CreateEntryBlockAlloca(std::string Identifer) {
    llvm::IRBuilder<> TmpB(&Function->getEntryBlock(), Function->getEntryBlock().begin());
    llvm::AllocaInst* Alloca = TmpB.CreateAlloca(llvm::Type::getDoubleTy(*CodeGen::TheContext), 0, Identifer.c_str());
    Locals[Identifer] = Alloca;
    return Alloca;
}

llvm::StoreInst* Environment::storeLocal(std::string Identifer, llvm::Value* Value) {
    // Check if identifier is already declared in the environment
    if (Locals.find(Identifer) != Locals.end()) {
        CodeGen::error("Variable is already defined once");
        return nullptr;
    }

    // Store the initial value into the alloca.
    llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(Identifer);
    llvm::StoreInst* Store = CodeGen::Builder->CreateStore(Value, Alloca);
    Locals[Identifer] = Alloca;
    return Store;
}

llvm::AllocaInst* Environment::getAlloca(std::string Identifer) {
    // Check the current environment
    llvm::AllocaInst* Alloca = Locals[Identifer];
    if (Alloca) {
        return Alloca;
    }

    // Check the enclosing environment
    if (!EnclosingEnv) {
        CodeGen::error("Allocation was not found");
        return nullptr;
    }
    return EnclosingEnv->getAlloca(Identifer);
}

llvm::StoreInst* Environment::assignLocal(std::string Identifer, llvm::Value* Value) {
    llvm::AllocaInst* Alloca = getAlloca(Identifer);
    if (Alloca) {
        return CodeGen::Builder->CreateStore(Value, Alloca);
    }
    return nullptr;
}

llvm::LoadInst* Environment::getLocal(std::string Identifer) {
    llvm::AllocaInst* Alloca = getAlloca(Identifer);
    if (Alloca) {
        return CodeGen::Builder->CreateLoad(Alloca, Identifer);
    }
    return nullptr;
}

Environment::~Environment() {
    Locals.clear();
}