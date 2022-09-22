#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "codegen.hpp"
#include "parser.hpp"
#include "printVisitor.hpp"
#include "scanner.hpp"

std::string openFile(const char* file_path) {
    std::fstream f;
    std::stringstream strStream;
    f.open(file_path);
    strStream << f.rdbuf();
    f.close();
    std::string code;
    code = strStream.str();
    return code;
}

void run(const std::string&& file) {
    std::unique_ptr<Scanner> scanner = std::make_unique<Scanner>(std::move(file));
    scanner->scanTokens();
    if (scanner->HadError) {
        exit(1);
    }
#ifdef DEBUG_LIST_TOKENS
    scanner->printTokens();
#endif

    std::unique_ptr<Parser> parser = std::make_unique<Parser>(scanner->Tokens);
    parser->parseStmts();
    if (parser->HadError) {
        exit(1);
    }

#ifdef DEBUG_PRINT_AST
    parser->printAST();
#endif

    for (auto& smt : parser->ParsedStmts) {
        smt->codegen();
    }
    if (CodeGen::HadError) {
        exit(1);
    }

#ifdef DEBUG_PRINT_MODULE
    CodeGen::TheModule->print(llvm::errs(), nullptr);
#endif
}

int main(int argc, char* argv[]) {
    std::string code = openFile(argv[1]);
    CodeGen::insertExtFunctions();
    run(std::move(code));
    CodeGen::generateIR();
}
