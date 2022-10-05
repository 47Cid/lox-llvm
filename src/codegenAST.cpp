#ifndef codegenAST_h
#define codegenAST_h

#include "ast.hpp"
#include "codegen.hpp"

llvm::Value *CallSmtAST::codegen() {
  // Look up the name in the global module table.
  llvm::Function *CalleeF = CodeGen::TheModule->getFunction(Callee);
  if (!CalleeF)
    return CodeGen::error(
        "Error calling function: Function not found the module");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return CodeGen::error("Arguments do not match");

  std::vector<llvm::Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }
  return CodeGen::Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
  std::vector<llvm::Type *> Doubles(
      Args.size(), llvm::Type::getDoubleTy(*CodeGen::TheContext));
  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*CodeGen::TheContext), Doubles, false);
  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, Name, CodeGen::TheModule.get());

  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

llvm::Value *FunctionAST::codegen() {
  llvm::Function *TheFunction =
      CodeGen::TheModule->getFunction(Proto->getName());

  if (!TheFunction)
    TheFunction = Proto->codegen();

  if (!TheFunction)
    return CodeGen::error("Error generating function code: Function not found");

  llvm::BasicBlock *EntryBlock =
      llvm::BasicBlock::Create(*CodeGen::TheContext, "entry", TheFunction);
  llvm::BasicBlock *ExitBlock =
      llvm::BasicBlock::Create(*CodeGen::TheContext, "exit");

  // Insert following IR into this block
  CodeGen::Builder->SetInsertPoint(EntryBlock);

  // Create a new environment
  CodeGen::CurrEnv = std::make_unique<Environment>(TheFunction, ExitBlock,
                                                   std::move(CodeGen::CurrEnv));

  // Store all the function arguments in the environment
  for (auto &Arg : TheFunction->args())
    CodeGen::CurrEnv->storeLocal(std::string(Arg.getName()), &Arg);
  // Allocate the return value in the environment
  CodeGen::CurrEnv->createEntryBlockAlloca("retvalue");

  // Generate IR for the function body
  for (auto &Smt : BodyStmts) {
    Smt->codegen();
  }

  // Unconditional break
  CodeGen::Builder->CreateBr(ExitBlock);
  // Insert the function exit block
  TheFunction->getBasicBlockList().push_back(ExitBlock);
  CodeGen::Builder->SetInsertPoint(ExitBlock);
  // Load the return value
  llvm::LoadInst *Ret = CodeGen::CurrEnv->getLocal("retvalue");
  CodeGen::Builder->CreateRet(Ret);

  // Reset the environment
  CodeGen::CurrEnv = std::move(CodeGen::CurrEnv->EnclosingEnv);

  std::string error;
  llvm::raw_string_ostream errorStream(error);

  // Remove the function if it is not valid
  if (verifyFunction(*TheFunction, &errorStream)) {
    std::cout << error;
    TheFunction->eraseFromParent();
    return CodeGen::error("Function CodeGen Error: ");
  }

  return TheFunction;
}

llvm::Value *BinaryAST ::codegen() {
  llvm::Value *L = Left->codegen();
  llvm::Value *R = Right->codegen();

  if (!L || !R)
    return CodeGen::error("Error generating code for binary instruction");

  switch (Op->Type) {
  case TOK_PLUS:
    return CodeGen::Builder->CreateFAdd(L, R, "addtmp");
  case TOK_MINUS:
    return CodeGen::Builder->CreateFSub(L, R, "subtmp");
  case TOK_STAR:
    return CodeGen::Builder->CreateFMul(L, R, "multmp");
  case TOK_SLASH:
    return CodeGen::Builder->CreateFDiv(L, R, "divtmp");
  case TOK_LESS:
    L = CodeGen::Builder->CreateFCmpULT(L, R, "cmptmp");
    return CodeGen::Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*CodeGen::TheContext), "booltmp");
  case TOK_GREATER:
    L = CodeGen::Builder->CreateFCmpUGT(L, R, "cmptmp");
    return CodeGen::Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*CodeGen::TheContext), "booltmp");
  case TOK_LESS_EQUAL:
    L = CodeGen::Builder->CreateFCmpULE(L, R, "cmptmp");
    return CodeGen::Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*CodeGen::TheContext), "booltmp");
  case TOK_GREATER_EQUAL:
    L = CodeGen::Builder->CreateFCmpUGE(L, R, "cmptmp");
    return CodeGen::Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*CodeGen::TheContext), "booltmp");
  case TOK_EQUAL_EQUAL:
    L = CodeGen::Builder->CreateFCmpOEQ(L, R, "cmptmp");
    return CodeGen::Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*CodeGen::TheContext), "booltmp");
  case TOK_BANG_EQUAL:
    L = CodeGen::Builder->CreateFCmpONE(L, R, "cmptmp");
    return CodeGen::Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*CodeGen::TheContext), "booltmp");
  default:
    return nullptr;
  }
}

llvm::Value *GroupingAST ::codegen() { return Expr->codegen(); }

llvm::Value *VariableAST ::codegen() {
  // Look this variable up in the function.
  llvm::Value *V = CodeGen::CurrEnv->getLocal(Name->Lexeme);
  return V;
}

llvm::Value *NumberAST ::codegen() {
  return llvm::ConstantFP::get(*CodeGen::TheContext, llvm::APFloat(Value));
}

llvm::Value *StringAST ::codegen() {
  return CodeGen::Builder->CreateGlobalString(Value);
}

llvm::Value *AssignAST ::codegen() {
  switch (Type) {
  case TOK_TYPE_NUM:
    if (IsDecl) {
      return CodeGen::CurrEnv->storeLocal(Name->Lexeme, Value->codegen());
    }
    return CodeGen::CurrEnv->assignLocal(Name->Lexeme, Value->codegen());
    break;
  case TOK_TYPE_STRING:
    return Value->codegen();
  default:
    return nullptr;
  }
}

llvm::Value *IfAST ::codegen() {
  llvm::Value *CondV = Cond->codegen();
  if (!CondV)
    return nullptr;

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = CodeGen::Builder->CreateFCmpONE(
      CondV, llvm::ConstantFP::get(*CodeGen::TheContext, llvm::APFloat(0.0)),
      "ifcond");

  llvm::Function *TheFunction = CodeGen::CurrEnv->Function;
  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  llvm::BasicBlock *ThenBB =
      llvm::BasicBlock::Create(*CodeGen::TheContext, "then", TheFunction);
  llvm::BasicBlock *ElseBB =
      llvm::BasicBlock::Create(*CodeGen::TheContext, "else");
  llvm::BasicBlock *MergeBB =
      llvm::BasicBlock::Create(*CodeGen::TheContext, "ifcont");
  // Merge directly with exit block
  llvm::BasicBlock *ExitBlock = CodeGen::CurrEnv->ExitBlock;
  CodeGen::Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  CodeGen::CurrEnv = std::make_unique<Environment>(TheFunction, ExitBlock,
                                                   std::move(CodeGen::CurrEnv));
  // Emit then value.
  CodeGen::Builder->SetInsertPoint(ThenBB);
  // Generate code for all the 'then' statements
  std::vector<llvm::Value *> ThenVs;
  bool ThenExit = false;
  for (auto &Smt : Then) {
    // Reset the basic block before adding new instruction
    CodeGen::Builder->SetInsertPoint(ThenBB);
    ThenVs.push_back(Smt->codegen());
    // Check if parsed statement was 'Return'
    if (Smt->getType() == ASTType::ReturnAST)
      ThenExit = true;
  }
  // Jump to a block depending on whether or not the current block had a return
  // statement
  if (ThenExit)
    CodeGen::Builder->CreateBr(ExitBlock);
  else
    CodeGen::Builder->CreateBr(MergeBB);

  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  CodeGen::Builder->SetInsertPoint(ElseBB);
  bool ElseExit = false;
  // Generate code for all the 'else' statements
  std::vector<llvm::Value *> ElseVs;
  for (auto &Smt : Else) {
    // Reset the basic block before adding new instruction
    CodeGen::Builder->SetInsertPoint(ElseBB);
    ElseVs.push_back(Smt->codegen());
    // Check if parsed statement was 'Return'
    if (Smt->getType() == ASTType::ReturnAST)
      ElseExit = true;
  }
  // Jump to a block depending on whether or not the current block had a return
  // statement
  if (ElseExit)
    CodeGen::Builder->CreateBr(ExitBlock);
  else
    CodeGen::Builder->CreateBr(MergeBB);

  // Emit merge block. Rest of the function code gets inserted into this block
  TheFunction->getBasicBlockList().push_back(MergeBB);
  CodeGen::Builder->SetInsertPoint(MergeBB);

  CodeGen::CurrEnv = std::move(CodeGen::CurrEnv->EnclosingEnv);

  return nullptr;
}

llvm::Value *ReturnAST ::codegen() {
  // store return value
  return CodeGen::CurrEnv->assignLocal("retvalue", Value->codegen());
}

#endif
