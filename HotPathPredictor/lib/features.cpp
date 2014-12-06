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
#include <vector>
#include "mlanalyzer.h"
#include <fstream>
#include <iostream>
#include <map>
using namespace std;
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
	int loopHighInstCnt;
	float avgBBLoops;
	int condTrueLoops;
	int eqCmp;
	int tryBlock;
	int catchBlock;	
	float avgPred;
	float avgSucc;
	int sideEnt;
};

void printFeatures(struct features f){
	DEBUG(dbgs()<<"Inst Count:"<<f.instcount);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Cond Count:"<<f.condcount);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Var Assign:"<<f.varassign);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Var Store :"<<f.varstore);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Uniq Load :"<<f.uniqload);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Uniq Store:"<<f.uniqstore);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Global Var :"<<f.globalvar);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Local Var :"<<f.localvar);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Same Load Tot :"<<f.sameloadtot);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Same Store Tot :"<<f.samestoretot);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Same Load Var :"<<f.sameloadvar);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Same Store Var :"<<f.samestorevar);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Avg Same Load  :"<<f.avgsameload);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Avg Same Store :"<<f.avgsamestore);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Function Calls :"<<f.functioncalls);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Avg Func Params :"<<f.avgFuncParam);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Recursive Calls :"<<f.recursiveCall);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Pointer Alloc :"<<f.pointerAlloc);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"% Var Used :"<<f.varUsed);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"% Var Assign :"<<f.varAssign);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Max Call Depth :"<<f.maxCallDepth);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Min Call Depth :"<<f.minCallDepth);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Avg Call Depth :"<<f.avgCallDepth);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Array Vars :"<<f.arrayVars);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Array Store :"<<f.arrayStore);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Array Load :"<<f.arrayLoad);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Max Array Size :"<<f.maxArraySize);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Min Array Size :"<<f.minArraySize);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Float Type :"<<f.floatTy);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Int Type :"<<f.intTy);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Float Type Arith :"<<f.floatTyArith);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Int Type Arith :"<<f.intTyArith);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Int Type Log:"<<f.intTyLog);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Basic Blocks :"<<f.basicBlocks);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Null Comparisons :"<<f.nullComp);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Loop Count :"<<f.loopCount);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Average Loop Depth :"<<f.avgLoopDepth);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Average Loop Exits :"<<f.avgLoopExits);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"High Inst Cnt Loops :"<<f.loopHighInstCnt);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Avg Loops BB :"<<f.avgBBLoops);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Cond True Loops :"<<f.condTrueLoops);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Equal Comp :"<<f.eqCmp);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Try Block :"<<f.tryBlock);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"catch Block :"<<f.catchBlock);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Avg Pred :"<<f.avgPred);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Avg Succ :"<<f.avgSucc);
	DEBUG(dbgs()<<"\n");
	DEBUG(dbgs()<<"Side Ent :"<<f.sideEnt);
	DEBUG(dbgs()<<"\n");
		
	DEBUG(dbgs()<<"\n");
}

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
	std::map<Function *, std::vector< struct features> > pathFeatureCollecn;
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
	void countInstAndBB(Loop *L,int *bbCount,int *insCount);
	void printFeaturesToFile(double freq, features pathFreatures);
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
				}	
				if(f)
					varl++;

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
				}	
				if(f)
					vars++;

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
	else
		*avgpara=0;
	*recCall=rc;
	if(c>0){
		*maxNest=maxN;
		*minNest=minN;
	}
	else{
		*maxNest=0;
		*minNest=0;
	}	
	if(c>0)
		*avgNest=totNest/(float)c;

	else
		*avgNest=0;
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
	DEBUG(dbgs()<<"\t >>Depth of :"<<F->getName()<<" "<<depth<<"\n");
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
			Type * vtemp =AI->getAllocatedType();	
			long size =1;	
			while(ArrayType *at =  dyn_cast<ArrayType>(vtemp)){
				DEBUG(dbgs()<<"\t >> Array Type:"<<*vtemp<<"\n");
				DEBUG(dbgs()<<"\t >> ArraySize:"<<" "<<at->getNumElements()<<" "<<*vtemp->getArrayElementType()<<"\n");
				vtemp = vtemp->getArrayElementType();
				size*=at->getNumElements();
			}
			if(size>1){
				minArraySize=min(minArraySize,size);
				maxArraySize=max(maxArraySize,size);
			}
		}
	}
	*arrayVar=arraysV;
	*arrayStore=arrayS;
	*arrayLoad=arrayL;
	if(arraysV >0){
		*maxSize=maxArraySize;
		*minSize=minArraySize;
	}
	else{
		*maxSize=0;
		*minSize=0;
	}
		
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
		if(s1.find("try") == 0){
			DEBUG(dbgs()<<"\t >> found try"<<BB->getName()<<"\n");
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
	DEBUG(dbgs()<<"\t >> Pred Tot:"<<pred<<"\n");
	DEBUG(dbgs()<<"\t >> Succ Tot:"<<succ<<"\n");
	*avgPred=pred/(float)path.size();
	*avgSucc=succ/(float)path.size();
	*sideEnt=sent;
	*tryBB=tryB;
	*catchBB=catchB;
}

void getSubLoops(Loop *l,std::vector<Loop *> &loops){
        std::vector<Loop*> subLoops = l->getSubLoops();
        for(int i=0;i<subLoops.size();++i){
                loops.push_back(subLoops[i]);
                getSubLoops(subLoops[i],loops);
        }    
}

int nestLoop(Loop *L){
	int maxDepth =L->getLoopDepth();
	std::vector<Loop *> subLoops;
	getSubLoops(L,subLoops);
	for(int i = 0;i<subLoops.size();++i){
		if(subLoops[i]->getLoopDepth()>maxDepth)
			maxDepth=subLoops[i]->getLoopDepth();
	}
	return maxDepth-L->getLoopDepth()+1;

}

void MLFeatures::countInstAndBB(Loop *L,int *bbCount,int *insCount){
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
	DEBUG(dbgs()<<"\t >> Collecting Loop info\n");

	if(!LI) return;
	for (LoopInfo::iterator i = LI->begin(), e =LI->end(); i != e; ++i){
		Loop *L =*i;
		depthCount[L]=nestLoop(L);
		DEBUG(dbgs()<<"\t >> Depth of "<<L->getHeader()->getName()<<" "<<depthCount[L]<<"\n");

		int insC,bbC;
		countInstAndBB(L,&insC,&bbC);

		instCount[L]=insC;
		bbCount[L]=bbC;
		DEBUG(dbgs()<<"\t >> Loop BB:"<<bbCount[L]<<"\n");
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
		std::vector<Loop *> subLoops; 
		getSubLoops(L,subLoops);
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
		if(L && loopTrack[L]!=1){
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
		DEBUG(dbgs()<<"\t >> Uniq Var or F:"<<F->getName()<<" "<<uniqVars[F]<<"\n");
	}

	for(Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
        	Function &FF=*F;    
 		if(!FF.isDeclaration()){
			LI = &getAnalysis<LoopInfo>(FF);
			SE=&getAnalysis<ScalarEvolution>(FF);
			errs()<<"---Running for Function:"<<F->getName()<<"\n";
			collectLoopInfo();
		}

			std::vector<double> pathFreqs = MS->pathCollecnFreq[&FF];
			errs()<<"Function -----------: "<< FF.getName() << "  --> " << pathFreqs.size() <<" paths \t" << MS->expandedPath[F].size()<<" "<<MS->pathCollecn2[F].size() << "\n";



		for(int i=0;i<MS->expandedPath[F].size();++i){
			double frequency = pathFreqs[i];
			for(int k=0;k<MS->pathCollecn2[F][i].size();++k){
				DEBUG(dbgs()<<MS->pathCollecn2[F][i][k]->getName()<<" ");
			}
			DEBUG(dbgs()<<"\n");
			struct features pathFeatures;
			pathFeatures.instcount=MS->expandedPath[F][i].size();
			int cond = countCond(MS->expandedPath[F][i],&pathFeatures.eqCmp);                	
			pathFeatures.condcount=cond;
			int load,store;
			countVarAssignments(MS->expandedPath[F][i],&load,&store,&pathFeatures.uniqload,&pathFeatures.uniqstore);
			pathFeatures.varassign=load;
			pathFeatures.varstore=store;
			if(uniqVars[F]){
				pathFeatures.varUsed= pathFeatures.uniqload/(float)uniqVars[F];
				pathFeatures.varAssign= pathFeatures.uniqstore/(float)uniqVars[F];
			}
			else{
				pathFeatures.varUsed=0; 
				pathFeatures.varAssign= 0;

			}
			int local,global;
			getGlobalCount(MS->expandedPath[F][i],&local,&global);
			pathFeatures.localvar=local;
			pathFeatures.globalvar=global;
			getSameLoadStoreCount(MS->expandedPath[F][i],&pathFeatures.sameloadtot,&pathFeatures.samestoretot,&pathFeatures.sameloadvar,&pathFeatures.samestorevar); 
			if(pathFeatures.uniqload!=0)
				pathFeatures.avgsameload= pathFeatures.sameloadtot/(float)pathFeatures.uniqload;
			else
				 pathFeatures.avgsameload=0;

			if(pathFeatures.uniqstore!=0)
				pathFeatures.avgsamestore = pathFeatures.samestoretot/(float)pathFeatures.uniqstore;
			
			else
				pathFeatures.avgsamestore=0;
			getFunctionFeatures(MS->expandedPath[F][i],&pathFeatures.functioncalls,&pathFeatures.avgFuncParam,&pathFeatures.recursiveCall,&pathFeatures.maxCallDepth,&pathFeatures.minCallDepth,&pathFeatures.avgCallDepth);
			getPointerFeatures(MS->expandedPath[F][i],&pathFeatures.pointerAlloc);		
			//getInstTypeFeatures(MS->expandedPath[F][i]);			
			getArrayFeatures(MS->expandedPath[F][i],&pathFeatures.arrayVars,&pathFeatures.arrayStore,&pathFeatures.arrayLoad,&pathFeatures.maxArraySize,&pathFeatures.minArraySize);
			getInstTypeFeatures(MS->expandedPath[F][i],&pathFeatures.floatTy, &pathFeatures.intTy,&pathFeatures.floatTyArith,&pathFeatures.intTyArith, &pathFeatures.intTyLog,&pathFeatures.nullComp );
			getBlockLevelFeatures(MS->pathCollecn2[F][i],&pathFeatures.avgSucc,&pathFeatures.avgPred,&pathFeatures.sideEnt,&pathFeatures.tryBlock,&pathFeatures.catchBlock);
			collectLoopFeatures(MS->pathCollecn2[F][i],&pathFeatures.loopCount,&pathFeatures.avgLoopDepth,&pathFeatures.avgLoopExits,&pathFeatures.loopHighInstCnt,&pathFeatures.avgBBLoops,&pathFeatures.condTrueLoops);
			pathFeatures.basicBlocks=MS->pathCollecn2[F][i].size();
			pathFeatureCollecn[F].push_back(pathFeatures);
			
			printFeatures(pathFeatures);
			printFeaturesToFile(frequency, pathFeatures );
		}       	 

	}	

	return false;
}

//This function prints the path execution frequency along with its feature vector
void MLFeatures::printFeaturesToFile(double freq, features f){
	ofstream myfile;
	const char * fileName= "/home/mahmoud/Documents/paths_training.txt";
  	myfile.open (fileName,ios::app);
	myfile << freq << 
	  "\t" << f.instcount<<
	  "\t"<<f.condcount <<
	  "\t"<<f.varassign <<
	  "\t"<<f.varstore <<
	  "\t"<<f.uniqload <<
	  "\t"<<f.uniqstore <<
	  "\t"<<f.globalvar <<
	  "\t"<<f.localvar <<
	  "\t"<<f.sameloadtot <<
	  "\t"<<f.samestoretot <<
	  "\t"<<f.sameloadvar <<
	  "\t"<<f.samestorevar <<
	  "\t"<<f.avgsameload <<
	  "\t"<<f.avgsamestore <<
	  "\t"<<f.functioncalls <<
	  "\t"<<f.avgFuncParam <<
	  "\t"<<f.recursiveCall <<
	  "\t"<<f.pointerAlloc <<
	  "\t"<<f.varUsed <<
	  "\t"<<f.varAssign <<
	  "\t"<<f.maxCallDepth <<
	  "\t"<<f.minCallDepth <<
	  "\t"<<f.avgCallDepth <<
	  "\t"<<f.arrayVars <<
	  "\t"<<f.arrayStore <<
	  "\t"<<f.arrayLoad <<
	  "\t"<<f.maxArraySize <<
	  "\t"<<f.minArraySize <<
	  "\t"<<f.floatTy <<
	  "\t"<<f.intTy <<
	  "\t"<<f.floatTyArith <<
	  "\t"<<f.intTyArith <<
	  "\t"<<f.intTyLog <<
	  "\t"<<f.basicBlocks <<
	  "\t"<<f.nullComp <<
	  "\t"<<f.loopCount <<
	  "\t"<<f.avgLoopDepth <<
	  "\t"<<f.avgLoopExits <<
	  "\t"<<f.loopHighInstCnt <<
	  "\t"<<f.avgBBLoops <<
	  "\t"<<f.condTrueLoops <<
	  "\t"<<f.eqCmp <<
	  "\t"<<f.tryBlock <<
	  "\t"<<f.catchBlock <<
	  "\t"<<f.avgPred <<
	  "\t"<<f.avgSucc <<
	  "\t"<<f.sideEnt <<
	  "\n";
}


char MLFeatures::ID = 0;
static RegisterPass<MLFeatures> X("mlfeature", "ML Feature Pass", false, false);
