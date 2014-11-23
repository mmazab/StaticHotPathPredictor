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
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "./staticpath.h"
#include <vector>
#include <map>
using namespace llvm;

struct features{

	int instcount;
	int condcount;
	int varassign;
	int varstore;
	int uniqload;
	int uniqstore;
	int globalvar;
	int localvar;
	int sameloadtot;
	int samestoretot;
	int sameloadvar;
	int samestorevar;	
	float avgsameload;
	float avgsamestore;
	int functioncalls;
	float avgFuncParam;
	int recursiveCall;	
	int pointerAlloc;
	float varUsed;
	float varAssign;	
	int maxCallDepth;
	int minCallDepth;
	float avgCallDepth;
	int arrayVars;
	int arrayStore;
	int arrayLoad;
	long maxArraySize;
	long minArraySize;
	int floatTy;
	int intTy;
	int floatTyArith;
	int intTyArith;
	int intTyLog;
	int basicBlocks;
	int nullComp;
	int loopCount;
	float avgLoopDepth;
	float avgLoopExits;
	long loopHighInstCnt;
	float avgBBLoops;
	int condTrueLoops;
	int eqCmp;
	int tryBlock;
	int catchBlock;	
	float avgPred;
	float avgSucc;
	int sideEnt;
};


struct MLFeatures : public ModulePass {
        static char ID; 
        MLFeatures() : ModulePass(ID) {}
        bool runOnModule(Module &M);
	std::map<Function *,int> recFun;
	DominatorTree *DT;
	LoopInfo *LI;
	MLStatic * MS;
	ScalarEvolution * SE;
	std::map<Function *,int> uniqVars;
	std::map<Function *, std::vector< std::vector< struct features > > > featureVector;
	std::map<Loop *,int> instCount;
	std::map<Loop *, int> bbCount;
	std::map<Loop *, int> exitsCount;
	std::map<Loop *,int> depthCount;
	std::map<Loop *,long> iterCount;
	std::map<Loop *, int> condPresent;
	void getAnalysisUsage(AnalysisUsage &AU) const;
	int countCond(std::vector<Instruction *> ,int *);
	void countVarAssignments(std::vector<Instruction *>, int *load, int *store,int *uniql, int *uniqs);
	void getGlobalCount(std::vector<Instruction *>, int * local, int *global);
	void getSameLoadStoreCount(std::vector<Instruction *>,int *sameloadtot, int *samestoretot, int *sameloadvar, int *samestorevar );
	void getFunctionFeatures(std::vector<Instruction *>, int * calls, float *avgpara, int  *reccount,int *maxcalldepth,int *mincalldepth,float  *avgcalldepth);
	void getPointerFeatures(std::vector<Instruction *>, int *);
	int getCallNestDepth(Function *F);
	void getArrayFeatures(std::vector<Instruction *>,int * arrayVar,int *arrayStore, int * arrayLoad,long *max, long *min);
	void getInstTypeFeatures(std::vector<Instruction *>,int *,int *,int *,int *,int *,int *);
	void getBlockLevelFeatures(std::vector<BasicBlock *>,float *,float *,int *,int * tryBB, int *catchBB);
	void collectLoopInfo();
	void collectLoopFeatures(std::vector<BasicBlock *>, int * loopCount, float  * avgLoopDepth, float * avgLoopExits,int *loopHighInstCnt, float  *avgBBLoops,int * trueCondLoops);
	int countInstAndBB(Loop *L,int *bbCount,int *insCount);
};


void MLFeatures::getAnalysisUsage(AnalysisUsage &AU) const{
        AU.addRequired<DominatorTree>();
        AU.addRequired<LoopInfo>();
        AU.addRequired<MLStatic>();
	AU.addRequired<ScalarEvolution>();
	AU.setPreservesAll();   
}

int MLFeatures::countCond(std::vector<Instruction *> path, int * eqCmp ){
	int cond=0;
	for(int i=0;i<path.size();++i){
		if(ICmpInst * icmp = dyn_cast<ICmpInst>(path[i])){
			++cond;
			if(icmp->isEquality())
				++(*eqCmp);
		}
	}	
	return cond;
}

void  MLFeatures::countVarAssignments(std::vector<Instruction *> path,int *load,int *store,int * uniql, int *uniqs){
	int varl=0,vars=0,ul=0,us=0;
	std::map<Value *,int> present1,present2;
	for(int i=0;i<path.size();++i){
		if(isa<LoadInst>(path[i])){
			if(present1[path[i]->getOperand(0)]!=1){
				++ul;
				present1[path[i]->getOperand(0)]=1;
			}				

			++varl;
		}
		if(isa<StoreInst>(path[i])){
			
			if(present2[path[i]->getOperand(1)]!=1){
				++us;
				present2[path[i]->getOperand(1)]=1;
			}				
			++vars;
		}		
	}	
	*load=varl;
	*store=vars;
	*uniql=ul;
	*uniqs=us;
}

void MLFeatures::getGlobalCount(std::vector<Instruction *> path, int * local, int *global){
	int l=0,g=0;
	for(int i=0;i<path.size();++i){
		if(isa<LoadInst>(path[i])){
			Value * v = path[i]->getOperand(0);
			if(isa<GlobalVariable>(v))
				++g;
			else
				++l;

			
		}

	}
	*local=l;
	*global=g;

}

void MLFeatures::getSameLoadStoreCount(std::vector<Instruction *> path,int *sameloadtot, int *samestoretot, int *sameloadvar, int *samestorevar){
	int sameload=0,samestore=0,varl=0,vars=0;
	std::map<Value *,int> present,present2;
	for(int i=0;i<path.size();++i){
		if(isa<LoadInst>(path[i])){
			
			if(present[path[i]->getOperand(0)]!=1){
				int f=0;
				present[path[i]->getOperand(0)]=1;
				for(int j=i+1;j<path.size();++j){
					if(isa<LoadInst>(path[j]) && (path[j]->getOperand(0) == path[i]->getOperand(0)) ){
						f=1;
						sameload++;
					}
					if(f)
						varl++;
				}	

			}
		}
		if(isa<StoreInst>(path[i])){
			
			if(present2[path[i]->getOperand(1)]!=1){
				int f=0;
				present2[path[i]->getOperand(1)]=1;
				for(int j=i+1;j<path.size();++j){
					if(isa<StoreInst>(path[j]) && (path[j]->getOperand(1) == path[i]->getOperand(1)) ){
						f=1;
						samestore++;
					}
					if(f)
						vars++;
				}	

			}
		}

	}
	*sameloadtot=sameload;
	*samestoretot=samestore;
	*sameloadvar=varl;
	*samestorevar=vars;

}

long max(long a,long b){
	return a>b?a:b;
}

long min(long a,long b){
	return a<b?a:b;
}
void MLFeatures::getFunctionFeatures(std::vector<Instruction *> path, int * calls, float *avgpara, int * recCall,int *maxNest,int  *minNest,float *avgNest){
	int c=0,p=0,rc=0,maxN=-1,minN=999,totNest=0;
	for(int i=0;i<path.size();++i){
		if(CallInst *I = dyn_cast<CallInst>(path[i])){
			++c;
			p+=(I->getNumArgOperands());
			Function *F = I->getCalledFunction();
			if(recFun[F]==1)
				++rc;		
			int depth=getCallNestDepth(F);
			maxN=max(depth+1,maxN);
			minN=min(depth+1,minN);
			totNest+=depth+1;	
		}
	}
	*calls=c;
	if(c>0)
		*avgpara= p/(float)c;
	*recCall=rc;
	*maxNest=maxN;
	*minNest=minN;
	if(c>0)
		*avgNest=totNest/(float)c;

}

void MLFeatures::getPointerFeatures(std::vector<Instruction *>path, int * allocs){
	int alloc=0;
	for(int i=0;i<path.size();++i){
		if(isa<AllocaInst>(path[i]))
			++alloc;
	}
	*allocs=alloc;

}


int MLFeatures::getCallNestDepth(Function *F){
	int depth=0;
	
        for(int i=0;i<MS->expandedPath[F].size();++i){
               for(int j=0;j<MS->expandedPath[F][i].size();++j){
			if(CallInst * I = dyn_cast<CallInst>(MS->expandedPath[F][i][j])){
				Function * FF = I->getCalledFunction();
			        if(recFun[FF]!=1)	
					depth = max(1 + getCallNestDepth(FF),depth);
			}
		}
	}
	return depth;

}

void MLFeatures::getArrayFeatures(std::vector<Instruction *> path,int *arrayVar,int * arrayStore, int *arrayLoad,long *maxSize, long * minSize){
	int arraysV=0,arrayL=0,arrayS=0;
	long minArraySize=1e10,maxArraySize=-1;
	for(int i=0;i<path.size();++i){
		if(GetElementPtrInst * gep = dyn_cast<GetElementPtrInst>(path[i])){
			++arraysV;
			for(Value::use_iterator it = gep->use_begin(), ie = gep->use_end(); it!=ie; ++it){
				Instruction * I = (Instruction *)(*it);
				if(isa<LoadInst>(I))
					++arrayL;
				if(isa<StoreInst>(I))
					++arrayS;
			}
		}
		if(AllocaInst * AI = dyn_cast<AllocaInst>(path[i])){
			if(AI->getAllocatedType()->isArrayTy()){
				Value *  sizeC= AI->getArraySize();
				long size =0;
				if(ConstantInt* CI = dyn_cast<llvm::ConstantInt>(sizeC))
					size = CI->getZExtValue();
				
				minArraySize=min(minArraySize,size);
				maxArraySize=max(maxArraySize,size);
			}
		}
	}
	*arrayVar=arraysV;
	*arrayStore=arrayS;
	*arrayLoad=arrayL;
	*maxSize=maxArraySize;
	*minSize=minArraySize;
}

void MLFeatures::getInstTypeFeatures(std::vector<Instruction *> path,int *floatTy, int *intTy,int  *floatTyArith,int * intTyArith, int * intTyLog ,int * nulC){
	int floatty=0,intty=0,farith=0,iarith=0,ilog=0;	
	int nul=0;
	for(int i=0;i<path.size();++i){
		Value * v = (Value *)path[i];
		if(v->getType()->isFloatTy() || v->getType()->isDoubleTy()){
			++floatty;
		}
		if(v->getType()->isIntegerTy())
			++intty;
		int opCode = path[i]->getOpcode();	

		 if(opCode == ( Instruction::Add ) || opCode==Instruction::Sub || opCode==Instruction::Mul || opCode==Instruction::UDiv || opCode==Instruction::SDiv || opCode==Instruction::URem || opCode==Instruction::SRem)
			++iarith;

		if(opCode== Instruction::Shl || opCode== Instruction::AShr ||opCode== Instruction::LShr  ||opCode== Instruction::ICmp || opCode==Instruction::And || opCode==Instruction::Or || opCode == Instruction::Xor)
                        ++ilog;               
                                  

                if(opCode== (Instruction::FAdd) || opCode== Instruction::FSub || opCode==Instruction::FMul || opCode==Instruction::FDiv || opCode==Instruction::FRem || opCode==Instruction::FCmp)
                        ++farith;         

		if(ICmpInst * icmp = dyn_cast<ICmpInst>(path[i])){
			Value * op1 = icmp->getOperand(0);
			Value * op2 = icmp->getOperand(1);
			if(  ConstantPointerNull * val = dyn_cast<ConstantPointerNull>(op2)){
				++nul;		
			}
			if(  ConstantPointerNull * val = dyn_cast<ConstantPointerNull>(op1)){
				++nul;		
			}
			
		}

	}

	*floatTy=floatty;
	*intTy=intty;
	*floatTyArith=farith;
	*intTyArith=iarith;
	*intTyLog=ilog;
	*nulC=nul;
} 


void MLFeatures::getBlockLevelFeatures(std::vector<BasicBlock *> path,float  *avgSucc,float *avgPred,int *sideEnt,int *tryBB, int *catchBB ){
	int succ=0,pred=0,sent=0,sexit=0;
	int tryB=0,catchB=0;
	for(int i=0;i<path.size();++i){
		BasicBlock *BB = path[i];
		std::string s1 = BB->getName();
		if(s1.find("catch") != std::string::npos && s1.find("dispatch")==std::string::npos)
			++catchB;
		if(s1.find("try") != std::string::npos){
			++tryB;
		}

		int predC=0;
		for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
			++predC;
		}
		pred+=predC;
		if(predC)
			sent+=(predC-1);
		TerminatorInst * TI = BB->getTerminator();
		int succT = TI->getNumSuccessors();
		succ+=succT;
	}
	*avgPred=pred/(float)path.size();
	*avgSucc=succ/(float)path.size();
	*sideEnt=sent;
	*tryBB=tryB;
	*catchBB=catchB;
}

int nestLoop(Loop *L){
	int maxDepth =L->getLoopDepth();
	std::vector<Loop *> subLoops;
	for(int i = 0;i<subLoops.size();++i){
		if(subLoops[i]->getLoopDepth()>maxDepth)
			maxDepth=subLoops[i]->getLoopDepth();
	}
	return maxDepth-L->getLoopDepth()+1;

}

int MLFeatures::countInstAndBB(Loop *L,int *bbCount,int *insCount){
	int bb=0,ins=0,cond=0;
	for(Loop::block_iterator block = L->block_begin(), y = L->block_end(); block!= y; ++block){
		++bb;
		std::string s1=(*block)->getName().str();
		if(s1.find("cond") !=std::string::npos)
			cond=1;
		for (BasicBlock::iterator i = (*block)->begin(), ie = (*block)->end(); i != ie; ++i){
			++ins;
		}
	
	}  
	if(cond)
		condPresent[L]=1;
	else
		condPresent[L]=0;
	*bbCount=bb;
	*insCount=ins;

}

void MLFeatures::collectLoopInfo(){
	for (LoopInfo::iterator i = LI->begin(), e =LI->end(); i != e; ++i){
		Loop *L =*i;
		depthCount[L]=nestLoop(L);
		int insC,bbC;
		countInstAndBB(L,&insC,&bbC);
		instCount[L]=insC;
		bbCount[L]=bbC;
		if (SE->hasLoopInvariantBackedgeTakenCount(L)) {
    			const SCEV *upperBound = SE->getBackedgeTakenCount(L);
    			if (isa<SCEVConstant>(upperBound)) {
      				const SCEVConstant *constUpperBound = cast<SCEVConstant>(upperBound);
      				long temp = constUpperBound->getValue()->getZExtValue();
				iterCount[L]=temp;	
			}
		}
		
		SmallVector<BasicBlock *, 8> ExitBlocks;
		L->getExitingBlocks(ExitBlocks);
		exitsCount[L]=ExitBlocks.size();
		std::vector<Loop *> subLoops = L->getSubLoops();
		for(int j=0;j<subLoops.size();++j){
			depthCount[subLoops[j]]=nestLoop(subLoops[j]);
			countInstAndBB(subLoops[j],&insC,&bbC);
			instCount[subLoops[j]]=insC;
			bbCount[subLoops[j]]=bbC;
			SmallVector<BasicBlock *, 8> ExitBlocks;
			subLoops[j]->getExitingBlocks(ExitBlocks);
			exitsCount[subLoops[j]]=ExitBlocks.size();
			Loop *L = subLoops[j];
			if (SE->hasLoopInvariantBackedgeTakenCount(L)) {
    				const SCEV *upperBound = SE->getBackedgeTakenCount(L);
    				if (isa<SCEVConstant>(upperBound)) {
      					const SCEVConstant *constUpperBound = cast<SCEVConstant>(upperBound);
      					long temp = constUpperBound->getValue()->getZExtValue();
					iterCount[L]=temp;	
				}
			}
		}

	
	}
}

void MLFeatures::collectLoopFeatures(std::vector<BasicBlock *> path, int * loopCount,float  * avgLoopDepth, float * avgLoopExits,int *loopHighInstCnt, float *avgBBLoops, int *condTrueLoops){
	std::map<Loop *, int> loopTrack;
	int lc=0,totDepth=0,totExits=0,totBB=0,highInst=0,trueCond=0;
	for(int i=0;i<path.size();++i){
		Loop * L = LI->getLoopFor(path[i]);
		if(loopTrack[L]!=1){
			++lc;
			loopTrack[L]=1;
			totDepth+=depthCount[L];
			totExits+=exitsCount[L];
			if(instCount[L]>25)
				++highInst;
			totBB+=bbCount[L];
			if(condPresent[L]==0)
				trueCond++;
		}
			
	}
	*loopCount=lc;
	if(lc){
		*avgLoopDepth=totDepth/(float)lc;
		*avgLoopExits=totExits/(float)lc;
		*avgBBLoops= totBB/(float)lc;
		
	}
	else{
		*avgLoopDepth=0;
		*avgLoopExits=0;
		*avgBBLoops=0;

	}
	*loopHighInstCnt=highInst;
	*condTrueLoops = trueCond;
}


bool MLFeatures::runOnModule(Module &M) {
	MS=&getAnalysis<MLStatic>();
	std::map<Value *,int> tempMap;
	for(Module::iterator F = M.begin(),E=M.end();F!=E;++F){
		tempMap.clear();
		uniqVars[F]=0;
		for(int i=0;i<MS->expandedPath[F].size();++i){
			
			for(int j=0;j<MS->expandedPath[F][i].size();++j){
				if(CallInst * I = dyn_cast<CallInst>(MS->expandedPath[F][i][j])){
					Function * FF = I->getCalledFunction();
					if(FF == F)
						recFun[F]=1;
				}
				if(isa<LoadInst>(MS->expandedPath[F][i][j]) && tempMap[MS->expandedPath[F][i][j]->getOperand(0)]!=1 ){
					uniqVars[F]++;	
					tempMap[MS->expandedPath[F][i][j]->getOperand(0)]=1;
				}
				if(isa<StoreInst>(MS->expandedPath[F][i][j]) && tempMap[MS->expandedPath[F][i][j]->getOperand(1)]!=1 ){
					uniqVars[F]++;	
					tempMap[MS->expandedPath[F][i][j]->getOperand(1)]=1;
				}
				
				
			}

		}
	}


	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
        	Function &FF=*F;    
 		if(!FF.isDeclaration()){
			LI = &getAnalysis<LoopInfo>(FF);
			SE=&getAnalysis<ScalarEvolution>(FF);
		}
		errs()<<"---Running for Function:"<<F->getName()<<"\n";
		for(int i=0;i<MS->expandedPath[F].size();++i){
			struct features pathFeatures;
			pathFeatures.instcount=MS->expandedPath[F][i].size();
			int cond = countCond(MS->expandedPath[F][i],&pathFeatures.eqCmp);                	
			pathFeatures.condcount=cond;
			int load,store;
			countVarAssignments(MS->expandedPath[F][i],&load,&store,&pathFeatures.uniqload,&pathFeatures.uniqstore);
			pathFeatures.varassign=load;
			pathFeatures.varstore=store;
			pathFeatures.varUsed= pathFeatures.uniqload/(float)uniqVars[F];
			pathFeatures.varAssign= pathFeatures.uniqstore/(float)uniqVars[F];
			int local,global;
			getGlobalCount(MS->expandedPath[F][i],&local,&global);
			pathFeatures.localvar=local;
			pathFeatures.globalvar=global;
			getSameLoadStoreCount(MS->expandedPath[F][i],&pathFeatures.sameloadtot,&pathFeatures.samestoretot,&pathFeatures.sameloadvar,&pathFeatures.samestorevar); 
			pathFeatures.avgsameload= pathFeatures.sameloadtot/(float)pathFeatures.uniqload;
			pathFeatures.avgsamestore = pathFeatures.samestoretot/(float)pathFeatures.uniqstore;
			getFunctionFeatures(MS->expandedPath[F][i],&pathFeatures.functioncalls,&pathFeatures.avgFuncParam,&pathFeatures.recursiveCall,&pathFeatures.maxCallDepth,&pathFeatures.minCallDepth,&pathFeatures.avgCallDepth);
			getPointerFeatures(MS->expandedPath[F][i],&pathFeatures.pointerAlloc);		
			//getInstTypeFeatures(MS->expandedPath[F][i]);			
			getArrayFeatures(MS->expandedPath[F][i],&pathFeatures.arrayVars,&pathFeatures.arrayStore,&pathFeatures.arrayLoad,&pathFeatures.maxArraySize,&pathFeatures.minArraySize);
			getInstTypeFeatures(MS->expandedPath[F][i],&pathFeatures.floatTy, &pathFeatures.intTy,&pathFeatures.floatTyArith,&pathFeatures.intTyArith, &pathFeatures.intTyLog,&pathFeatures.nullComp );
		}       	 

	}	

	return false;
}


char MLFeatures::ID = 0;
static RegisterPass<MLFeatures> X("mlfeature", "ML Feature Pass", false, false);
