#include "AST.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"



class OptimizationVisitor : public ASTVisitor{

    std::vector<llvm::StringRef> liveVars;

    
    AST* M;


    OptimizationVisitor(){
        liveVars.push_back(llvm::StringRef("result"));

        aliveIDinDec.push_back(llvm::StringRef("result"));    
    }

    bool mustRemove;
    
  virtual void visit(BinaryOp &) = 0;        
  virtual void visit(Equation &) = 0;    
  virtual void visit(Condition &) = 0;
  virtual void visit(Conditions &) = 0;
  virtual void visit(If &) = 0;
  virtual void visit(Elif &) = 0;
  virtual void visit(Else &) = 0;
  virtual void visit(Loop&) =0;

virtual void visit(GSM &gsm){
        auto stats = gsm.getEquations(); 
        

        for (; stats.end() != stats.begin(); --stats.end()) {
            (*stats.end())->accept(*this);
            if (mustRemove) {
            stats.erase(stats.end());
            }
}
        gsm.setEquations(stats);  

}

    virtual void visit(Equation &statement){
        Final* leftValue = statement.getLValue();
        auto iter = find(liveVars.begin(), liveVars.end(), leftValue->getVal());
        if(iter != liveVars.end()){ //is live
            if(statement.getAssign() == Equation::AssOp::Assign){
                liveVars.erase(iter);
            }
            statement.getRValue()->accept(*this);
            mustRemove = false;
            return;
        }
        mustRemove =  true;
    }
    virtual void visit(Expr &statement){
        auto leftValue = statement.getLeft();
        auto rightValue = statement.getRight();
        leftValue->accept(*this);
        rightValue->accept(*this);

    }
    virtual void visit(Final &statement){
        if(statement.getKind() == Final::ValueKind::Id){
            liveVars.push_back(statement.getVal());
        }
        else{
            return;
        }
    }
    virtual void visit(Declaration &statement){
        llvm::StringRef leftValue = *(statement.getVars().begin());
        auto iter = find(liveVars.begin(), liveVars.end(), leftValue);
        if(iter != liveVars.end()){ //is live
            liveVars.erase(iter);
            auto rV = *(statement.getExprs().begin());
            rightValue->accept(*this);
            mustRemove = false;
            return;
        }

        else {
            mustRemove = (find(liveVars.begin(), liveVars.end(), leftValue) == liveVars.end()) ? true : false;
        }

        
        
        
    }
       
};


class Optimization{
    public:
    void Optimize(AST *Tree) {
        OptimizationVisitor Op;
        Tree->accept(Op);
    }
};