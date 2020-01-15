#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include <map>
#include <string>

using namespace llvm;

namespace {
struct CountStaticInstructions : public FunctionPass {
  static char ID;
  CountStaticInstructions() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    std::map<std::string, int> map;
    // F is a pointer to a Function instance
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction* inst = &(*I);
      map[inst->getOpcodeName()] += 1;
    }
    for (auto& p: map)
      errs() << p.first << " " << p.second << "\n";
  
    return false;
  }
}; // end of struct CountStaticInstructions
}  // end of anonymous namespace

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cse231-csi", "Count static instructions",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);