#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>

using namespace llvm;
using namespace std;

namespace {
struct BranchBias : public FunctionPass {
  static char ID;
  BranchBias() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    Module* mod = F.getParent();
    LLVMContext &context = mod->getContext();
    Function* updateBranchInfoFunc = cast<Function>(mod->getOrInsertFunction(
        "updateBranchInfo",
        Type::getVoidTy(context),        // return type
        Type::getInt1Ty(context)         // num
    ));

    Function* printOutBranchInfoFunc = cast<Function>(mod->getOrInsertFunction(
        "printOutBranchInfo",
        Type::getVoidTy(context)         // return type
    ));

    for (Function::iterator bb = F.begin(); bb != F.end(); bb++) {
        for (BasicBlock::iterator it = bb->begin(); it != bb->end(); it++) {
            IRBuilder<> builder(&*it);
            BranchInst* brInst = cast<BranchInst>(it);
            if (brInst->isConditional()) {
                // builder.SetInsertPoint(brInst);
                vector<Value*> args;
                args.push_back(brInst->getCondition());
                builder.CreateCall(updateBranchInfoFunc, args);
            }
            
            if((string)it->getOpcodeName() == "ret"){
                // builder.SetInsertPoint(&*it);
                builder.CreateCall(printOutBranchInfoFunc);
            }
        }
    }
    return true;
  }
}; // end of struct BranchBias
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Profile Branch Bias",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);