#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/ProfileInfo.h"
#include <vector>
#include <map>
using namespace llvm;

struct MLStatic : public ModulePass {
	BranchProbabilityInfo* BI;
	static char ID;
	ProfileInfo* PI;
   	MLStatic() : ModulePass(ID) {}
	bool runOnModule(Module &F);
	//trace all paths starting from entry BB of a function
	void tracePath(BasicBlock *BB);
	//TO DO: Not sure if required. Authors replace function calls in CFG by CFG of called functions. But will decide later. 
	void mergePath(Module &M);
	//Expand basic blocks to get path at instruction level
	void expandPathAll(Module &M);
	void expandPath(std::vector<BasicBlock *>, Function * F);
	//A stack to store BB on current path
	std::vector<BasicBlock *> path;

	//List of all paths for a given function. Ex pathCollecn2[F][0] would give the first path (stored as vector of BB) for function F. 
	std::map<Function *, std::vector<std::vector<BasicBlock *> > >  pathCollecn2;

	std::map<Function *, std::vector<std::vector<double> > >  pathCollecnWeights;
	std::map<Function *, std::vector< double > >  pathCollecnFreq;

	//Not used currently
	std::vector<std::vector<BasicBlock *> >  pathCollecn;
	//Same as pathCollecn2, but Instruction instead of BB
	std::map<Function *, std::vector<std::vector<Instruction *> > > expandedPath;
	DominatorTree * DT;
	
	//this function assign weights to the edges between paths
	void ProfileFunctionPaths(Function* F, BranchProbabilityInfo* BI);
	double GetMinimumExecution(std::vector<double> execCounts);
	void getAnalysisUsage(AnalysisUsage &AU) const;
};

