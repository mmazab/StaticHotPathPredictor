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
#include <vector>
#include <map>
using namespace llvm;

struct MLStatic : public ModulePass {
	static char ID;
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
	//Not used currently
	std::vector<std::vector<BasicBlock *> >  pathCollecn;
	//Same as pathCollecn2, but Instruction instead of BB
	std::map<Function *, std::vector<std::vector<Instruction *> > > expandedPath;
	DominatorTree * DT;
	void getAnalysisUsage(AnalysisUsage &AU) const;
};

//Get all possible execution paths for a function. Ignoring the backedges for now
void MLStatic::tracePath(BasicBlock *BB){

	path.push_back(BB);
	int flag=0;
	const TerminatorInst *TInst = BB->getTerminator();
	int succn = TInst->getNumSuccessors();
	for(int i=0,NSucc = TInst->getNumSuccessors(); i < NSucc; ++i){
		BasicBlock *Succ = TInst->getSuccessor(i);
		if(!DT->dominates(Succ,BB)){
			tracePath(Succ);
		}
		else{
			flag=1;
			std::vector<BasicBlock *> temp;
			for(int i=0;i<path.size();++i){
				//DEBUG(dbgs()<<path[i]->getName()<<" ");
				temp.push_back(path[i]);
				
			}
			pathCollecn.push_back(temp);
			pathCollecn2[BB->getParent()].push_back(temp);
			//DEBUG(dbgs()<<"\n");
			flag=0;
		}
		
	}
	if(succn==0){
		std::vector<BasicBlock *> temp;
		for(int i=0;i<path.size();++i){
			//DEBUG(dbgs()<<path[i]->getName()<<" ");
			temp.push_back(path[i]);
			
		}
		pathCollecn.push_back(temp);
		pathCollecn2[BB->getParent()].push_back(temp);
		//DEBUG(dbgs()<<"\n");
	}
	
	
	path.pop_back();
}



void MLStatic::getAnalysisUsage(AnalysisUsage &AU) const{
                 AU.addRequired<DominatorTree>();
                 AU.addRequired<LoopInfo>();
                 AU.setPreservesAll();   
}


bool MLStatic::runOnModule(Module &M) {

	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		if (F->isDeclaration()) continue;
      	   	errs()<<"---Running for Function:"<<F->getName()<<"\n";
	   	DT = &getAnalysis<DominatorTree>(*F);
		BasicBlock *BB = &(F->getEntryBlock());     
		tracePath(BB);
   	 }

	 DEBUG(dbgs()<<"PATH collection Complete\n");
	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
	 for(int i=0;i<pathCollecn2[F].size();++i){
		for(int j=0;j<pathCollecn2[F][i].size();++j){
			DEBUG(dbgs()<<pathCollecn2[F][i][j]->getName()<<" ");
		}
		DEBUG(dbgs()<<"\n");
       	}
	}

	expandPathAll(M);
	 return false;
}

void MLStatic::expandPath(std::vector<BasicBlock *> tpath, Function *F){
		
	std::vector<Instruction *> temp;
	for(int i=0;i<tpath.size();++i){
		BasicBlock * BB = tpath[i];	
		for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i){
			temp.push_back(i);

		}

	}		
	expandedPath[F].push_back(temp);

}
//Expand Basic Blocks to have path in terms of instructions.
void  MLStatic::expandPathAll(Module &M){

	
	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		for(int i=0;i<pathCollecn2[F].size();++i){
			expandPath(pathCollecn2[F][i],F);				

		}	

	}

	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		for(int i=0;i<expandedPath[F].size();++i){
			for(int j=0;j<expandedPath[F][i].size();++j){
				DEBUG(dbgs()<<*expandedPath[F][i][j]<<"->");
			}
			DEBUG(dbgs()<<"\n");
		}	
		
	}
}

//TO DO: Replace call site by actual cfg
void MLStatic::mergePath(Module &M){

	//Not Sure if to replace function call by actual path
	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		for(int i=0;i<expandedPath[F].size();++i){
			for(int j=0;j<expandedPath[F][i].size();++j){
				Instruction * temp = expandedPath[F][i][j];
				if(CallInst * CI= dyn_cast<CallInst>(temp)){
					Function * CalledFun = CI->getCalledFunction();
				}
			}
		}	
		
	}

}
	
char MLStatic::ID = 0;
static RegisterPass<MLStatic> X("mlstatic", "ML Static Pass", false, false);
