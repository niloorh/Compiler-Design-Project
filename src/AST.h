#ifndef AST_H
#define AST_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

// Forward declarations of classes used in the AST
class AST; //h
class Expr; 
class GSM; //h
class BinaryOp; //h
class Declaration; //h
class Final; //h
class Equation; //h
class Conditions;
class Condition; //h
class If;
class Elif;
class Else;
class Loop;
// ASTVisitor class defines a visitor pattern to traverse the AST
class ASTVisitor
{
public:
  // Virtual visit functions for each AST node type
  virtual void visit(AST &) {}               // Visit the base AST node
  virtual void visit(Expr &) {}              // Visit the expression node
  virtual void visit(GSM &) = 0;             // Visit the group of expressions node
  virtual void visit(Final &) = 0;          // Visit the factor node
  virtual void visit(BinaryOp &) = 0;        // Visit the binary operation node
  virtual void visit(Equation &) = 0;      // Visit the assignment expression node
  virtual void visit(Declaration &) = 0;     // Visit the variable declaration node
  virtual void visit(Condition &) = 0;
  virtual void visit(If &) = 0;
  virtual void visit(Elif &) = 0;
  virtual void visit(Else &) = 0;
  virtual void visit(Loop&) =0;
};

// AST class serves as the base class for all AST nodes
class AST
{
public:
  virtual ~AST() {}
  virtual void accept(ASTVisitor &V) = 0;    // Accept a visitor for traversal
};

// Expr class represents an expression in the AST
class Expr : public AST
{
public:
  Expr() {}
};

// GSM class represents a group of expressions in the AST
class GSM : public Expr
{
  using ExprVector = llvm::SmallVector<Expr *>;

private:
  ExprVector exprs;                          // Stores the list of expressions

public:
  GSM(llvm::SmallVector<Expr *> exprs) : exprs(exprs) {}

  llvm::SmallVector<Expr *> getExprs() { return exprs; }

  ExprVector::const_iterator begin() { return exprs.begin(); }

  ExprVector::const_iterator end() { return exprs.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};



class Final  : public Expr{
  public:
  enum ValueKind{
    id,
    num
  };
  private:
   ValueKind Kind;
   llvm::StringRef Val;

  public:
  Final(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val) , Expr(){}

  ValueKind getKind() { return Kind; }

  llvm::StringRef getVal() { return Val; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};
// Factor class represents a factor in the AST (either an identifier or a number)


// BinaryOp class represents a binary operation in the AST (plus, minus, multiplication, division)
class BinaryOp : public Expr
{
public:
  enum Operator
  {
    Plus,
    Minus,
    equal,
    power,
    star, //*
    slash,///
    KW_mod,//%
    KW_plusEqual, //+=
    KW_minusEqual, //-=
    KW_starEqual,//*=
    KW_slashEqual,///=
    KW_modEq, // %=
    
  
  };

private:
  Expr *Left;                               // Left-hand side expression
  Expr *Right;                              // Right-hand side expression
  Operator Op;                              // Operator of the binary operation

public:
  BinaryOp(Operator Op, Expr *L, Expr *R) : Op(Op), Left(L), Right(R), Expr() {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Assignment class represents an assignment expression in the AST
class Equation : public Expr
{
private:
  Final *Left;                             // Left-hand side factor (identifier)
  Expr *Right;                              // Right-hand side expression

public:
  Equation(Final *L, Expr *R) : Left(L), Right(R) , Expr() {}

  Final *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Declaration class represents a variable declaration with an initializer in the AST
class Declaration : public Expr
{
  using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
  VarVector Vars;                           // Stores the list of variables
  Expr *E;                                  // Expression serving as the initializer

public:
  Declaration(llvm::SmallVector<llvm::StringRef, 8> Vars, Expr *E) : Vars(Vars), E(E) , Expr(){}

  VarVector::const_iterator begin() { return Vars.begin(); }

  VarVector::const_iterator end() { return Vars.end(); }

  Expr *getExpr() { return E; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};





class Conditions : public AST {
	public: enum andOr {
      andd,
      orr
  };


  private :
    Conditions *Left;
    andOr AO;
    Conditions *Right;

  public :
 
    Conditions(Conditions *Left1 , andOr AO1 , Conditions *Right1): Left(Left1) , AO(AO1) , Right(Right1) {}
    Conditions() {}
  
    Conditions *getLeft() { return Left; }
    andOr getAO() {return AO; }
    Conditions *getRight() {return Right; }


    virtual void accept(ASTVisitor &V) {
      V.visit(*this);
    }


};

class Condition : public Conditions {
public:
  enum OperatorCondition {
    KW_eqNot,       // !=
    KW_EqEq,        // ==
    KW_greaterEqual, // >=
    KW_lessEqual,    // <=
    KW_lessThan,     // <
    KW_greaterThan,
  };

private:
  Expr *Left;  // Left-hand side expression
  Expr *Right; // Right-hand side expression
  OperatorCondition Op;

public:
  
  Condition(Expr *L,OperatorCondition Op, Expr *R) :  Left(L), Op(Op), Right(R), Conditions() {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  OperatorCondition getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override {
    V.visit(*this);
  }
};

class If : public Expr {
private:
  Conditions *condition;
  //llvm::SmallVector<Equation *> Conds;
  llvm::SmallVector<Equation *> Equations;
  llvm::SmallVector<Elif *> Elifs;
  Else *Elsee;

public:
  If(Conditions *condition, llvm::SmallVector<Equation *> Equations, llvm::SmallVector<Elif *> Elifs, Else *Elsee) :
    condition(condition), Equations(Equations), Elifs(Elifs), Elsee(Elsee) , Expr(){}

  Conditions *getCondition() { return condition; }

  llvm::SmallVector<Elif *> getElifs() { return Elifs; }
  llvm::SmallVector<Equation *>getEquations(){return Equations;}
	

  Else *getElse() { return Elsee; }

  virtual void accept(ASTVisitor &V) override {
    V.visit(*this);
  }

};


class Elif : public Expr {
  private : 
  Conditions *condition;
  llvm::SmallVector<Equation *> Equations;
  //llvm::SmallVector<Elif *> Elifs;
  //Else *Else;
  public :
  
  Elif(Conditions *cnd , llvm::SmallVector<Equation* > Equ ) : condition(cnd) , Equations(Equ) , Expr(){}
  Conditions *getCondition() {return condition; }
  llvm::SmallVector<Equation *> EquationsGet() {return Equations;}


};



class Else : public Expr {
  private :
  Conditions *condition;
  llvm::SmallVector<Equation *> Equations;
  public :
  Else(Conditions *cnd , llvm::SmallVector<Equation *> Equ ) : condition(cnd) , Equations(Equ), Expr(){}
  Conditions *getCondition() {return condition; }
  llvm::SmallVector<Equation *> EquationsGet() {return Equations;}


};

class Loop : public Expr{
  private: 

  Conditions *condition;
  llvm::SmallVector<Equation *> Equations;

  public: 
  Loop(Conditions *condition , llvm::SmallVector<Equation *> Equations) : condition(condition), Equations(Equations) , Expr(){}
  Conditions *getCondition(){return condition;}
  llvm::SmallVector<Equation *> getEquations() {return Equations;}
  virtual void accept(ASTVisitor &V) override {V.visit(*this);}
};


#endif
