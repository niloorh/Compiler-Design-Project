#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace
{
  class ToIRVisitor : public ASTVisitor
  {
    Module *M;
    IRBuilder<> Builder;
    Type *VoidTy;
    Type *Int32Ty;
    Type *Int8PtrTy;
    Type *Int8PtrPtrTy;
    Constant *Int32Zero;

    Value *V;
    StringMap<AllocaInst *> nameMap;
    llvm::FunctionType* MainFty;
    llvm::Function* MainFn;

  public:
    // Constructor for the visitor class.
    ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
    {
      // Initialize LLVM types and constants.
      VoidTy = Type::getVoidTy(M->getContext());
      Int32Ty = Type::getInt32Ty(M->getContext());
      Int8PtrTy = Type::getInt8PtrTy(M->getContext()); // chera sefide ??????
      Int8PtrPtrTy = Int8PtrTy->getPointerTo();
      Int32Zero = ConstantInt::get(Int32Ty, 0, true);
    }

    // Entry point for generating LLVM IR from the AST.
    void run(AST *Tree)
    {
      // Create the main function with the appropriate function type.
      FunctionType *MainFty = FunctionType::get(Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
      Function *MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

      // Create a basic block for the entry point of the main function.
      BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
      Builder.SetInsertPoint(BB);

      // Visit the root node of the AST to generate IR.
      Tree->accept(*this);

      // Create a return instruction at the end of the main function.
      Builder.CreateRet(Int32Zero);
    }

    // Visit function for the GSM node in the AST.
    virtual void visit(GSM &Node) override
    {
      // Iterate over the children of the GSM node and visit each child.
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

    // virtual void visit(If &Node)override{};
    // virtual void visit(Elif &Node)override{};
    // virtual void visit(Else &Node)override{};
    // virtual void visit(Loop &Node)override{};

    virtual void visit(Statements &Node)
    {
      if(Node.getKind() == Statements::ifDecEqLoop::EqKind)
      {
        Equation* equation = (Equation*)&Node;
        equation->accept(*this);
      }
      else if(Node.getKind() == Statements::ifDecEqLoop::DecKind)
      {
        Declaration* dec = (Declaration*)&Node;
        dec->accept(*this);
      }
      else if(Node.getKind() == Statements::ifDecEqLoop::IfKind)
      {
        If* iff = (If*)&Node;
        iff->accept(*this);
      }
      else if(Node.getKind() == Statements::ifDecEqLoop::LoopKind)
      {
        Loop* loopc = (Loop*)&Node;
        loopc->accept(*this);
      }
    };

    virtual void visit(Expr &Node) override
    {
      Node.getLeft()->accept(*this);
      Value *Left = V;

      Node.getRight()->accept(*this);
      Value *Right = V;

      switch (Node.getOperator())
      {
        case Expr::binaryOp::plus:
        V = Builder.CreateNSWAdd(Left, Right);
        break;
        case Expr::binaryOp::minus:
        V = Builder.CreateNSWSub(Left, Right);
        break;
        case Expr::binaryOp::star:
        V = Builder.CreateNSWMul(Left, Right);
        break;
        case Expr::binaryOp::slash:
        V = Builder.CreateSDiv(Left, Right);
        break;
        case Expr::binaryOp::KW_mod:
        {
          Value* div = Builder.CreateSDiv(Left, Right); // TODO: ?
          Value* mull = Builder.CreateNSWMul(div, Right);// TODO: ?
          V = Builder.CreateNSWSub(Left, mull);
        break;
        }

        case Expr::binaryOp::power:
        {
          Final* f = (Final *) Right;
          int RightValue;
          RightValue = f->getVal().getAsInteger(10, RightValue);
          if(RightValue == 0)
          {
            V = ConstantInt::get(Int32Ty, 1, true);
            break;
          }
          else
          {
            for(int i = 1; i < RightValue; i++)
            {
              Left = Builder.CreateNSWMul(Left, Left);
            }
            V = Left;
            break;
          }
        }
      }
    };
	
    virtual void visit(Equation &Node) override
    {
      // Visit the right-hand side of the assignment and get its value.
      Node.getRight()->accept(*this);
      Value *val = V;

      // Get the name of the variable being assigned.
      auto varName = Node.getLeft()->getVal();
      auto op = Node.getAssign();
      Value* tempVal;

      switch (Node.getAssign())
      {
        case Equation::assigning::equal:
        Builder.CreateStore(val, nameMap[varName]);
        break;
        case Equation::assigning::KW_modEq:{
        tempVal = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        Value* div = Builder.CreateSDiv(tempVal, val);
        Value* mull = Builder.CreateNSWMul(div, val);
        Builder.CreateStore(Builder.CreateNSWSub(tempVal, mull), nameMap[varName]);
        break;
        }
        case Equation::assigning::KW_slashEqual:{
        tempVal = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        Builder.CreateStore(Builder.CreateSDiv(tempVal, val), nameMap[varName]);
        break;
        }
        case Equation::assigning::KW_starEqual:{
        tempVal = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        Builder.CreateStore(Builder.CreateNSWMul(tempVal, val), nameMap[varName]);
        break;
        }
        case Equation::assigning::KW_minusEqual:{
        tempVal = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        Builder.CreateStore(Builder.CreateNSWSub(tempVal, val), nameMap[varName]);
        break;
        } 
        case Equation::assigning::KW_plusEqual:{
        tempVal = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        Builder.CreateStore(Builder.CreateNSWAdd(tempVal, val), nameMap[varName]);
        break;
        }
      }

      //Builder.CreateStore(val, nameMap[varName]);

      // Create a function type for the "gsm_write" function.
      FunctionType *CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);

      // Create a function declaration for the "gsm_write" function.
      Function *CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "gsm_write", M);

      // Create a call instruction to invoke the "gsm_write" function with the value.
      CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
    };

    virtual void visit(Final &Node) override
    {
      if (Node.get_Kind() == Final::id)
      {
        // If the factor is an identifier, load its value from memory.
        V = Builder.CreateLoad(Int32Ty, nameMap[Node.getVal()]);
        
      }
      else
      {
        // If the factor is a literal, convert it to an integer and create a constant.
        int intval;
        llvm::StringRef strVal = Node.getVal();//?
        Node.getVal().getAsInteger(10, intval);
        V = ConstantInt::get(Int32Ty, intval, true);
      }
    };

  //   virtual void visit(BinaryOp &Node) override
  //   {
      
  //     // Visit the left-hand side of the binary operation and get its value.
  //     Node.getLeft()->accept(*this);
  //     Value *Left = V;

  //     // Visit the right-hand side of the binary operation and get its value.
  //     Node.getRight()->accept(*this);
  //     Value *Right = V;

  //     // Perform the binary operation based on the operator type and create the corresponding instruction.
  //     switch (Node.getOperator())
  //     {
  //     case BinaryOp::Plus:
  //       V = Builder.CreateNSWAdd(Left, Right);
  //       break;
  //     case BinaryOp::Minus:
  //       V = Builder.CreateNSWSub(Left, Right);
  //       break;
  //     case BinaryOp::star:
  //       V = Builder.CreateNSWMul(Left, Right);
  //       break;
  //     case BinaryOp::slash:
  //       V = Builder.CreateSDiv(Left, Right);
  //       break;
  //     case BinaryOp::equal:
  //       V = Builder.CreateNSWAdd(Left, Right);
  //       break; //we dont know yet
  //     case BinaryOp::power:
  //       V = Builder.CreateNSWSub(Left, Right);
  //       break; // we have to build
  //     case BinaryOp::KW_mod:{
	// Value* Div;
	// Value* Mull;
  //       V = Builder.CreateNSWMul(Left, Right);
  //       Div = Builder.CreateSDiv(Left, Right); // q 
  //       Mull = Builder.CreateNSWMul(Div, Right); // bq
  //       V = Builder.CreateNSWSub(Left, Mull);//r=a_bq
	// 		    }
  //      	break;
  //     case BinaryOp::KW_plusEqual:
  //       V = Builder.CreateSDiv(Left, Right);
  //       break;
  //     case BinaryOp::KW_minusEqual:
  //       V = Builder.CreateNSWAdd(Left, Right);
  //       break;
  //     case BinaryOp::KW_starEqual:
  //       V = Builder.CreateNSWSub(Left, Right);
  //       break;
  //     case BinaryOp::KW_slashEqual:
  //       V = Builder.CreateNSWMul(Left, Right);
  //       break;
  //     case BinaryOp::KW_modEq:
  //       V = Builder.CreateSDiv(Left, Right);
  //       break;
  //     }
  //   };

   virtual void visit(Condition &Node) override
    {
      
      // Visit the left-hand  and right_hand side of the binary operation and get its value.
      Node.getLeft()->accept(*this);
      Value *Left = V;
      Node.getRight()->accept(*this);
      Value *Right = V;

      switch (Node.getOperator())
      {
      case Condition::OperatorCondition::KW_eqNot:
        V = Builder.CreateICmpNE(Left, Right);
        break;
          case Condition::OperatorCondition::KW_EqEq:
        V = Builder.CreateICmpEQ(Left, Right);
        break;
             case Condition::OperatorCondition::KW_greaterEqual:
        V = Builder.CreateICmpSGE(Left, Right);
        break;
             case Condition::OperatorCondition::KW_lessEqual:
        V = Builder.CreateICmpSLE(Left, Right);
        break;
             case Condition::OperatorCondition::KW_greaterThan:
        V = Builder.CreateICmpSGT(Left, Right);
        break;
             case Condition::OperatorCondition::KW_lessThan:
        V = Builder.CreateICmpSLT(Left, Right);
        break;
        default:
        break;
      }
   };

   virtual void visit(Conditions &Node)override
   {
      Node.getLeft()->accept(*this);
      Value* Left = V;
      Node.getRight()->accept(*this);
      Value* Right = V;
      switch (Node.getAO())
      {
        case Conditions::andOr::andd:
            V = Builder.CreateAnd(Left, Right);
            break;
        case Conditions::andOr::orr:
            V = Builder.CreateOr(Left, Right);
            break;
      }

   };


    virtual void visit(Declaration &Node) override //TODO: check more
    {
      llvm::SmallVector<llvm::StringRef, 8> vars = Node.getVars();
      llvm::SmallVector<Expr *> Exprs = Node.getExprs();
      auto expr = Exprs.begin();
      Value *val = nullptr;
      for(auto var = vars.begin(), var_end = vars.end(); var != var_end; ++var)
      {
        llvm::StringRef Var = *var;
        Expr* ExprTemp = *expr;

        if(ExprTemp)
        {
          ExprTemp->accept(*this);
          val = V;
          ++expr;

          nameMap[Var] = Builder.CreateAlloca(Int32Ty);
          if(val != nullptr)
          {
            Builder.CreateStore(val, nameMap[Var]);
          }
        }
        else
        {
          nameMap[Var] = Builder.CreateAlloca(Int32Ty);

          if(val != nullptr)
          {
            Value* zero = ConstantInt::get(Type::getInt32Ty(M->getContext()), 0);
            Builder.CreateStore(zero, nameMap[Var]);
          }
        }
      }
    };
    virtual void visit(If& Node) override{
      llvm::BasicBlock* IfCondBB = llvm::BasicBlock::Create(M->getContext(), "if.cond", MainFn);
      llvm::BasicBlock* IfBodyBB = llvm::BasicBlock::Create(M->getContext(), "if.body", MainFn);
      llvm::BasicBlock* AfterIfBB = llvm::BasicBlock::Create(M->getContext(), "after.if", MainFn);

      Builder.SetInsertPoint(IfCondBB);
      Node.getCondition()->accept(*this);
      llvm::Value* IfCondVal = V;
      Builder.CreateCondBr(IfCondVal, IfBodyBB, nullptr );

      Builder.SetInsertPoint(IfBodyBB);
      llvm::SmallVector<Equation*> Equations = Node.getEquations();
      for(auto I = Equations.begin(), E=Equations.end(); I != E;I++)
      {
        (*I)->accept(*this);
      }
      Builder.CreateBr(AfterIfBB);

      llvm::BasicBlock* PrevCondBB = IfCondBB;
      llvm::BasicBlock* PrevBodyBB = IfBodyBB;
      llvm::Value* PrevCondVal = IfCondVal;

      for(auto& Elif : Node.getElifs()){
      llvm::BasicBlock* ElifCondBB =llvm::BasicBlock::Create(MainFn->getContext(), "elif.cond", MainFn);
      llvm::BasicBlock* ElifBodyBB =llvm::BasicBlock::Create(MainFn->getContext(), "elif.body", MainFn);

      Builder.SetInsertPoint(PrevCondBB);
      Builder.CreateCondBr(PrevCondBB, PrevBodyBB, ElifCondBB);

      Builder.SetInsertPoint(ElifCondBB);
      Elif->getCondition()->accept(*this);
      llvm::Value* ElifCondVal =V;
      Builder.CreateCondBr(ElifCondVal, ElifBodyBB ,nullptr);

      Builder.SetInsertPoint(ElifBodyBB);
      Elif->accept(*this);
      Builder.CreateBr(AfterIfBB);

      PrevCondBB = ElifCondBB;
      PrevCondVal = ElifCondVal;
      PrevBodyBB = ElifBodyBB;
      }

      llvm::BasicBlock* ElseBB =nullptr;
      Else* els = Node.getElse();
      if(els){
          ElseBB = llvm::BasicBlock::Create(M->getContext(), "else,body", MainFn);
          Builder.SetInsertPoint(ElseBB);
          Node.getElse()->accept(*this);
          Builder.CreateBr(AfterIfBB);

          Builder.SetInsertPoint(PrevCondBB);
          Builder.CreateCondBr(IfCondVal, IfBodyBB, ElseBB);
      }
      else{
          Builder.SetInsertPoint(PrevCondBB);
          Builder.CreateCondBr(IfCondVal, IfBodyBB, AfterIfBB);
      }
      Builder.SetInsertPoint(AfterIfBB);
    }

    virtual void visit(Elif& Node) override{
      llvm::SmallVector<Equation*> Equations = Node.EquationsGet();
      for(auto I = Equations.begin(), E=Equations.end(); I != E;++I)
      {
        (*I)->accept(*this);
      }
    }

    virtual void visit(Else& Node) override{
      llvm::SmallVector<Equation*> Equations = Node.EquationsGet();
      for(auto I = Equations.begin(),  E=Equations.end(); I != E;++I)
      {
        (*I)->accept(*this);
      }
    }

    virtual void visit(Loop& Node) override
    {
      llvm::BasicBlock* WhileCondBB = llvm::BasicBlock::Create(M->getContext(), "loopc.cond", MainFn);
      llvm::BasicBlock* WhileBodyBB = llvm::BasicBlock::Create(M->getContext(), "loopc.body", MainFn);
      llvm::BasicBlock* AfterWhileBB = llvm::BasicBlock::Create(M->getContext(), "after.loopc", MainFn);

      Builder.CreateBr(WhileCondBB);
      Builder.SetInsertPoint(WhileCondBB);
      Node.getCondition()->accept(*this);
      Value* val=V;
      Builder.CreateCondBr(val, WhileBodyBB, AfterWhileBB);
      Builder.SetInsertPoint(WhileBodyBB);
      llvm::SmallVector<Equation* > Equations = Node.getEquations();
      for (auto I = Equations.begin(), E = Equations.end(); I != E; ++I)
      {
        (*I)->accept(*this);//TODO: check for I!=E
      }
      Builder.CreateBr(WhileCondBB);

      Builder.SetInsertPoint(AfterWhileBB);
    }
  };
}; // namespace

void CodeGen::compile(AST *Tree)
{
  // Create an LLVM context and a module.
  LLVMContext Ctx;
  Module *M = new Module("calc.expr", Ctx);

  // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR.
  ToIRVisitor ToIR(M);
  ToIR.run(Tree);

  // Print the generated module to the standard output.
  M->print(outs(), nullptr);
}


