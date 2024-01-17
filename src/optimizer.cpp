#include "AST.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"



class OptimizationVisitor : public ASTVisitor{

    std::vector<llvm::StringRef> liveVars;


    std::vector<llvm::StringRef> aliveIDinDec;
    
    AST* M;


    OptimizationVisitor(){
        liveVars.push_back(llvm::StringRef("result"));

        aliveIDinDec.push_back(llvm::StringRef("result"));    
    }

    bool mustRemove;
  //virtual void visit(Statements &) = 0;
  //virtual void visit(Final &) = 0;          
  virtual void visit(BinaryOp &) = 0;        // Visit the binary operation node
  virtual void visit(Equation &) = 0;      // Visit the assignment expression node
  //virtual void visit(Declaration &) = 0;     // Visit the variable declaration node
  virtual void visit(Condition &) = 0;
  virtual void visit(Conditions &) = 0;
  virtual void visit(If &) = 0;
  virtual void visit(Elif &) = 0;
  virtual void visit(Else &) = 0;
  virtual void visit(Loop&) =0;

virtual void visit(GSM &gsm){
        auto stats = gsm.getStatements(); //statements


        auto end = stats.end();
        auto begin = stats.begin();

        for (; end != begin; --end) {
            (*end)->accept(*this);
            if (mustRemove) {
            stats.erase(end);
            }
}
        gsm.setStatements(v);  

}


    virtual void visit(Statements &statement){
        Final* lValue = statement.getLValue();
        auto iter = find(liveVars.begin(), liveVars.end(), lValue->getVal());
        if(iter != liveVars.end()){
            if(statement.getAssignmentOP() == Statements::AssOp::Assign){
                liveVars.erase(iter);
            }
            statement.getRValue()->accept(*this);
            mustRemove = false;
            return;
        }
        mustRemove =  true;
    }


        virtual void visit(Declaration &statement){
        llvm::StringRef lValue = *(statement.getVars().begin());
        auto iter = find(liveVars.begin(), liveVars.end(), lValue);
        if(iter != liveVars.end()){
            llvm::errs() << lValue;
            liveVars.erase(iter);
            auto rightV = *(statement.getExprs().begin());
            rightV->accept(*this);
            mustRemove = false;
            return;
        }
        
        if(find(aliveIDinDec.begin(), aliveIDinDec.end(), lValue) == aliveIDinDec.end())
            mustRemove = true;
        else 
            mustRemove = false;
    }



     virtual void visit(Expr &statement){
        auto LValue = statement.getLeft();
        auto RValue = statement.getRight();

        LValue->accept(*this);

        RValue->accept(*this);
        
    }


        virtual void visit(Final &statement){
        if(statement.getKind() == Final::ValueKind::Ident){
            liveVars.push_back(statement.getVal());
            aliveIDinDec.push_back(statement.getVal());
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