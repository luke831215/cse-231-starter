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
struct CountDynamicInstructions : public FunctionPass {
  static char ID;
  CountDynamicInstructions() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    Module* mod = F.getParent();
    LLVMContext &context = mod->getContext();
    Function* updateInstrInfoFunc = cast<Function>(mod->getOrInsertFunction(
        "updateInstrInfo",  
        Type::getVoidTy(context),        // return type
        Type::getInt32Ty(context),       // num
        Type::getInt32PtrTy(context),    // keys
        Type::getInt32PtrTy(context)     // values
    ));

    Function* printOutInstrInfoFunc = cast<Function>(mod->getOrInsertFunction(
        "printOutInstrInfo",
        Type::getVoidTy(context)         // return type
    ));

    for (Function::iterator bb = F.begin(); bb != F.end(); bb++) {
        std::map<int, int> instMap;
        for (BasicBlock::iterator it = bb->begin(); it != bb->end(); it++) {
            instMap[it->getOpcode()]++;
        }

        int num = instMap.size();
        // insert in the end of each basic block
        IRBuilder<> builder(&*bb);
        builder.SetInsertPoint(bb->getTerminator());

        uint32_t keys[num], values[num];
        int i = 0;
        for (auto it = instMap.begin(); it != instMap.end(); it++, i++){
            // keys.push_back(ConstantInt::get(Type::getInt32Ty(context), it->first));
            // values.push_back(ConstantInt::get(Type::getInt32Ty(context), it->second));
            keys[i] = it->first;
            values[i] = it->second;
        }
        
        ArrayType* arrayTy = ArrayType::get(IntegerType::get(context, 32), num);
        GlobalVariable* keyG = new GlobalVariable(
        *mod, arrayTy, true, GlobalValue::InternalLinkage, 
        ConstantDataArray::get(context, *(new ArrayRef<uint32_t>(keys, num))), 
        "global keys");
        GlobalVariable* valueG = new GlobalVariable(
        *mod, arrayTy, true, GlobalValue::InternalLinkage, 
        ConstantDataArray::get(context, *(new ArrayRef<uint32_t>(values, num))), 
        "global values");
        
        Value* keyArg = builder.CreatePointerCast(keyG, Type::getInt32PtrTy(context));
        Value* valueArg = builder.CreatePointerCast(valueG, Type::getInt32PtrTy(context));
        vector<Value*> args;
        args.push_back(ConstantInt::get(Type::getInt32Ty(context), num));
        args.push_back(keyArg);
        args.push_back(valueArg);

        builder.CreateCall(updateInstrInfoFunc, args);

        for(BasicBlock::iterator it = bb->begin(); it != bb->end(); it++){
          if((string)it->getOpcodeName() == "ret"){
            builder.SetInsertPoint(&*it);
            builder.CreateCall(printOutInstrInfoFunc);
          }
        }
    }

    // IRBuilder<> builder(&(F.back().back()));
    // builder.CreateCall(printOutInstrInfoFunc);
    return true;
  }
}; // end of struct CountDynamicInstructions
}  // end of anonymous namespace

char CountDynamicInstructions::ID = 0;
static RegisterPass<CountDynamicInstructions> X("cse231-cdi", "Count dynamic instructions",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);