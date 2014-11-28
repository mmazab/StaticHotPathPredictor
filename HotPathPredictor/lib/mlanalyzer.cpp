#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/Support/Debug.h"
#include "mlanalyzer.h"
#include <vector>
#include <map>
using namespace llvm;



void MLStatic::ProfileFunctionPaths(Function* F, BranchProbabilityInfo* BI){
	//profiling analysis information
	PI = &getAnalysis<ProfileInfo>();
	//iterator over the paths
	std::map<Function *, std::vector<std::vector<BasicBlock *> > >::iterator fiter = pathCollecn2.find(F);
	{
		//list of all the function paths
		std::vector<std::vector<BasicBlock *> > paths = fiter->second;
		errs()<<"PathCollecn2 ---> " << pathCollecn2[F].size() <<"\n";
		//function paths weights
		std::vector<std::vector<double> > pathsWeights;
		std::vector<double> pathFreqs;
		for(size_t i=0; i < paths.size(); i++){
			//errs()  <<"\nPath " << i << ":\n";
			std::vector<BasicBlock *> path = paths[i];
			//path weights
			std::vector<double> pathWeights;
			double pathFreq = 0;
			double edgesWeights =1;
			for (size_t j=0; j< path.size(); j++){
				if(path.size() > 0 && j != path.size()-1){
					BasicBlock* bb = path[j];
					BasicBlock* succBB = path[j+1];
					//execution count of the basic block
					int bbExecCount = (int)( PI->getExecutionCount(bb));
					//the probability that the branch between the basicblock and its successor is taken
					BranchProbability bProp = BI->getEdgeProbability(bb, succBB);
					double weight=  ((double)bProp.getNumerator() / bProp.getDenominator());
					edgesWeights = edgesWeights * weight;
					//execution count of the successor
					double bbExec = (weight * bbExecCount);
					pathWeights.push_back(bbExec);
					//errs() << bb->getName() << "\t";
				}
			}
			if(path.size() > 0){
				//the percentage that the path got executed
				pathFreq = edgesWeights * (int)( PI->getExecutionCount(path[0])) * 100;
			}
			//errs()<<"\nFrequency:" <<pathFreq<<"\n";
			pathFreqs.push_back(pathFreq);
			pathsWeights.push_back(pathWeights);
		}
		pathCollecnFreq.insert(
			std::pair<Function *, std::vector<double> > (F, pathFreqs)
		);
		//list of the edge weights for each path within each function
		pathCollecnWeights.insert(
			std::pair<Function *, std::vector<std::vector<double> > > (F, pathsWeights)
		);
	}
}

//get the execution count of the path
double MLStatic::GetMinimumExecution(std::vector<double> execCounts){
	double tmp = -1;	
	for(size_t i=0; i < execCounts.size(); i++){
		if(tmp == -1) tmp = execCounts[i];
		else if( execCounts[i] < tmp) {
			tmp=execCounts[i];
		}
	}
	return tmp;
}

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
			for(size_t i=0;i<path.size();++i){
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
		for(size_t i=0;i<path.size();++i){
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
				AU.addRequired<ProfileInfo>();
                AU.addRequired<DominatorTree>();
                AU.addRequired<LoopInfo>();
				AU.addRequired<BranchProbabilityInfo>();
                AU.setPreservesAll();
}


bool MLStatic::runOnModule(Module &M) {
	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		if (F->isDeclaration()) continue;
      	   	errs()<<"---Running for Function: "<<F->getName()<<"\n";
	   	DT = &getAnalysis<DominatorTree>(*F);
		BasicBlock *BB = &(F->getEntryBlock());     
		tracePath(BB);
		BI = &getAnalysis<BranchProbabilityInfo>(*F);
		ProfileFunctionPaths(F, BI);
   	 }
	expandPathAll(M);

	DEBUG(dbgs()<<"PATH collection Complete\n");
	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		std::map<Function *, std::vector<double> >::iterator fiter= pathCollecnFreq.find(F);
		if(fiter != pathCollecnFreq.end()){
			std::vector<double> freqs = fiter->second;
			//errs()<<"Function Name ------------- " << fiter->first->getName()<<"\n";
			//for(size_t i=0; i<freqs.size(); i++){
			//	errs()<<"Path " <<i<<": " << freqs[i] <<"\n";
			//}
		}
		for(size_t i=0;i<pathCollecn2[F].size();++i){
			for(size_t j=0;j<pathCollecn2[F][i].size();++j){
			//DEBUG(dbgs()<<pathCollecn2[F][i][j]->getName()<<" ");
			//errs()<<pathCollecn2[F][i][j]->getName()<<" ";
			}
			DEBUG(dbgs()<<F->getName()<<" "<<pathCollecn2[F].size()<<" "<<expandedPath[F].size()<<"\n");
       	}
	}	 
	


	
	return false;
}



void MLStatic::expandPath(std::vector<BasicBlock *> tpath, Function *F){
	std::vector<Instruction *> temp;
	for(size_t i=0;i<tpath.size();++i){
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
		for(size_t i=0;i<pathCollecn2[F].size();++i){
			expandPath(pathCollecn2[F][i],F);				
		}	
	}
	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		

		for(size_t i=0;i<expandedPath[F].size();++i){
			for(size_t j=0;j<expandedPath[F][i].size();++j){
				//DEBUG(dbgs()<<*expandedPath[F][i][j]<<"->");
			}
			//DEBUG(dbgs()<<"\n");
		}	
	}
		
}

//TO DO: Replace call site by actual cfg
void MLStatic::mergePath(Module &M){
	//Not Sure if to replace function call by actual path
	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		for(size_t i=0;i<expandedPath[F].size();++i){
			for(size_t j=0;j<expandedPath[F][i].size();++j){
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
