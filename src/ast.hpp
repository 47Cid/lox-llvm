#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>

#include "token.hpp"
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

enum class ASTType {
  CallSmtAST,
  PrototypeAST,
  FunctionAST,
  BinaryAST,
  GroupingAST,
  NumberAST,
  VariableASt,
  AssignAST,
  IfAST,
  ReturnAST,
};
class CallSmtAST;
class PrototypeAST;
class FunctionAST;
class BinaryAST;
class GroupingAST;
class NumberAST;
class VariableAST;
class AssignAST;
class IfAST;
class ReturnAST;

class Visitor {
public:
  virtual void visit(CallSmtAST *e) = 0;
  virtual void visit(PrototypeAST *e) = 0;
  virtual void visit(FunctionAST *e) = 0;
  virtual void visit(BinaryAST *e) = 0;
  virtual void visit(GroupingAST *e) = 0;
  virtual void visit(NumberAST *e) = 0;
  virtual void visit(VariableAST *e) = 0;
  virtual void visit(AssignAST *e) = 0;
  virtual void visit(IfAST *e) = 0;
  virtual void visit(ReturnAST *e) = 0;
};

class SmtAST {
public:
  virtual ~SmtAST() = default;
  virtual ASTType getType() = 0;
  virtual void accept(class Visitor &v) = 0;
  virtual llvm::Value *codegen() = 0;
};

class CallSmtAST : public SmtAST {
public:
  std::string Callee;
  std::vector<std::unique_ptr<SmtAST>> Args;
  ASTType getType() override { return ASTType::CallSmtAST; }

  CallSmtAST(const std::string &Callee,
             std::vector<std::unique_ptr<SmtAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}

  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class PrototypeAST : public SmtAST {
public:
  std::string Name;
  std::vector<std::string> Args;
  ASTType getType() override { return ASTType::PrototypeAST; }

  PrototypeAST(const std::string &Name, std::vector<std::string> Args)
      : Name(Name), Args(std::move(Args)) {}

  void accept(Visitor &v) override { v.visit(this); }
  llvm::Function *codegen() override;
  const std::string &getName() const { return Name; }
};

class FunctionAST : public SmtAST {
public:
  std::unique_ptr<PrototypeAST> Proto;
  std::vector<std::unique_ptr<SmtAST>> BodyStmts;
  ASTType getType() override { return ASTType::FunctionAST; }

  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::vector<std::unique_ptr<SmtAST>> BodyStmts)
      : Proto(std::move(Proto)), BodyStmts(std::move(BodyStmts)) {}

  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class BinaryAST : public SmtAST {
public:
  std::unique_ptr<SmtAST> Left;
  Token *Op;
  std::unique_ptr<SmtAST> Right;
  ASTType getType() override { return ASTType::BinaryAST; }

  BinaryAST(std::unique_ptr<SmtAST> Left, Token *Op,
            std::unique_ptr<SmtAST> Right)
      : Left(std::move(Left)), Op(Op), Right(std::move(Right)) {}
  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class GroupingAST : public SmtAST {
public:
  std::unique_ptr<SmtAST> Expr;
  ASTType getType() override { return ASTType::GroupingAST; }

  GroupingAST(std::unique_ptr<SmtAST> Expr) : Expr(std::move(Expr)) {}
  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class NumberAST : public SmtAST {
public:
  double Value;
  NumberAST(double Value) : Value(Value) {}
  ASTType getType() override { return ASTType::NumberAST; }

  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class VariableAST : public SmtAST {
public:
  Token *Name;
  VariableAST(Token *Name) : Name(Name) {}
  ASTType getType() override { return ASTType::VariableASt; }

  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class AssignAST : public SmtAST {
public:
  Token *Name;
  std::unique_ptr<SmtAST> Value;
  bool IsDecl;
  ASTType getType() override { return ASTType::AssignAST; }

  AssignAST(Token *Name, std::unique_ptr<SmtAST> Value, bool IsDecl)
      : Name(Name), Value(std::move(Value)), IsDecl(IsDecl) {}
  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class IfAST : public SmtAST {
public:
  std::unique_ptr<SmtAST> Cond;
  std::vector<std::unique_ptr<SmtAST>> Then, Else;
  ASTType getType() override { return ASTType::IfAST; }

  IfAST(std::unique_ptr<SmtAST> Cond, std::vector<std::unique_ptr<SmtAST>> Then,
        std::vector<std::unique_ptr<SmtAST>> Else)
      : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

class ReturnAST : public SmtAST {
public:
  std::unique_ptr<SmtAST> Value;
  ASTType getType() override { return ASTType::ReturnAST; }

  ReturnAST(std::unique_ptr<SmtAST> Value) : Value(std::move(Value)) {}
  void accept(Visitor &v) override { v.visit(this); }
  llvm::Value *codegen() override;
};

#endif
