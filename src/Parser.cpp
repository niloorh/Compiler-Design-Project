#include "Parser.h"

// main point is that the whole input has been consumed
AST *Parser::parse()
{
    AST *Res = parseGSM();
    return Res;
}

GSM *Parser::parseGSM() //changed
{
    llvm::SmallVector<Statements *> stats;
    while (!Tok.is(Token::eoi))
    {
        switch (Tok.getKind())
        {
        case Token::KW_int:
            Declaration *d1;
            d1 = parseDec();
            if (d1)
                stats.push_back(d1);
            else
                goto _error;
            break;

            if (!Tok.is(Token::semicolon))
            {
               // error();
                goto _error;
            }

        case Token::id:
            Equation *d2;
            d2 = parseEquation();
            if (d2)
                stats.push_back(d2);
            else
                goto _error;
            break;

            if (!Tok.is(Token::semicolon))
            {
                error();
                goto _error;
            }

        case Token::KW_loopc:
            Loop *d3;
            d3 = parseLoop();
            if (d3)
                stats.push_back(d3);
            else
                goto _error;
            break;

            /*if (!Tok.is(Token::semicolon))
            {
                error();
                goto _error2;
            }*/
        case Token::KW_if:
            If *d4;
            d4 = parseIf();
            if (d4)
                stats.push_back(d4);
            else
                goto _error;
            break;

            // if (!Tok.is(Token::semicolon))
            // {
            //     error();
            //     goto _error2;
            // }
        default:
        error();
            goto _error;
            break;
        }
        advance(); // TODO: watch this part
    }
    return new GSM(stats);
_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Declaration *Parser::parseDec()
{
    Expr *E;
    llvm::SmallVector<Expr *> exprs;
    llvm::SmallVector<llvm::StringRef, 8> Vars;
    // if (!Tok.is(Token::KW_int) && !(Tok.is(Token::id)))
    //     goto _error;

    // advance();

    if (expect(Token::KW_int))
        goto _error;

    advance();

    if (expect(Token::id))
        goto _error;
    Vars.push_back(Tok.getText());
    advance();

    while (Tok.is(Token::comma))
    {
        advance();
        if (expect(Token::id))
            goto _error;
        Vars.push_back(Tok.getText());
        advance();
    }
    

    if (Tok.is(Token::equal))
    {
        advance();
        E = parseExpr();
        if(E) {
            exprs.push_back(E);
        }
        else
            goto _error;

        while(Tok.is(Token::comma)){
            advance();
            E = parseExpr();
            if(E) {
            exprs.push_back(E);
            }
            else
                goto _error;
            if(Tok.getKind() == Token::semicolon)
                break;
            advance();


        }
    }

    if (expect(Token::semicolon))
        goto _error;

    return new Declaration(Vars, exprs);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Equation *Parser::parseEquation()
{
    Expr *E;
    Final  *F = (Final*)(parseFactor());
    Equation::assigning asigned;

    if (!Tok.isOneOf(Token::plus, Token::minus , Token::equal, Token::power, Token::star, Token::slash , Token::KW_mod, Token::KW_plusEqual, Token::KW_minusEqual ,Token::KW_starEqual ,Token::KW_slashEqual , Token::KW_modEq))
    {
        error();
        // return nullptr;
        goto _error;
    }

    advance();


            if (Tok.is(Token::equal))  
                asigned =  Equation::assigning::equal;
            else if (Tok.is(Token::KW_plusEqual))  
                asigned =  Equation::assigning::KW_plusEqual;   
            else if (Tok.is(Token::KW_minusEqual))  
                asigned =  Equation::assigning::KW_minusEqual;   
            else if (Tok.is(Token::KW_starEqual))  
                asigned =  Equation::assigning::KW_starEqual;   
            else if (Tok.is(Token:: KW_slashEqual))  
                asigned =  Equation::assigning::KW_slashEqual;  
            else if (Tok.is(Token:: KW_modEq))  
                 asigned =  Equation::assigning::KW_modEq; 
            else    
                goto _error;

    
     advance();

    E = parseExpr();
    

    return new Equation(F ,asigned,E); 

_error:
    while (Tok.getKind() != Token::eoi)
	    advance();
    return nullptr;
}

Expr *Parser::parseExpr()
{
    Expr *Left = parseTerm();
    while (Tok.isOneOf(Token::plus, Token::minus))
    {
        Expr::binaryOp Op =
            Tok.is(Token::plus) ? Expr::plus : Expr::minus;
        advance();
        Expr *Right = parseTerm();
        Left = new Expr(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseTerm()
{
    Expr *Left = parseFactor();
    while (Tok.isOneOf(Token::star, Token::slash , Token::KW_mod))
    {
        Expr::binaryOp Op;
            
            if (Tok.is(Token::star)) 
                Op = Expr::binaryOp::star;
            else if (Tok.is(Token::slash))  
                Op = Expr::binaryOp::slash;
            else if (Tok.is(Token::KW_mod))  
                Op = Expr::binaryOp::KW_mod;  


        advance();
        Expr *Right = parseFactor();
        Left = new Expr(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseFactor()
{
    Expr *Left = parseFinal();
    while (Tok.is(Token::power))
    {
        Expr::binaryOp Op = Expr::binaryOp::power;
        
        advance();
        Expr *Right = parseFinal();
        Left = new Expr(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseFinal() 
{
    Expr *Res = nullptr;
    switch (Tok.getKind())
    {
    case Token::num:
        Res = new Final(Final::ValueKind::num, Tok.getText());
        advance();
        break;
    case Token::id:
        Res = new Final(Final::ValueKind::id, Tok.getText());
        advance();
        break;
    case Token::l_paren:
        advance();
        Res = parseExpr();
        if (!consume(Token::r_paren))
            break;
    default: // error handling
        if (!Res)
            error();
        while (!Tok.isOneOf(Token::r_paren, Token::star, Token::plus, Token::minus, Token::slash, Token::eoi))
            advance();
        break;
    }
    return Res;
}





//------------------------------------------------------------------------------------



If *Parser::parseIf() {

    Conditions  *condits = parseConditions();
    llvm::SmallVector<Equation *> Equations;
    //llvm::SmallVector<Statements *> stats;
    llvm::SmallVector<Elif *> Elifs;
    Else *Elsee;
    if(expect(Token::KW_if))
        {
            error();
            goto _error;}

    advance();
    
    if(expect(Token::KW_colon))
        goto _error;
    
    advance();

     if(expect(Token::KW_begin))
        goto _error;
    
    advance();

   while(!Tok.is(Token::KW_end))
   { 
    if(Tok.is(Token::id)){
        Equation *Eq = parseEquation();
        if(!Tok.is(Token::semicolon))
            {
                error();
                goto _error;
            }
        if(Eq)
            Equations.push_back(Eq);
        //if ()

    }
    }

    advance();

    while(Tok.is(Token::KW_elif)){
        Elif *El =  parseElif();
        if(El)
            Elifs.push_back(El);
        else{
            error();
            goto _error;
        }
        advance();
    }

        if(Tok.is(Token::KW_else)){
        Elsee = parseElse();
            } 



    return new If(condits , Equations , Elifs , Elsee);

_error:
    while(Tok.getKind() != Token::eoi)
	    advance();
    return nullptr;


    
}



Elif *Parser::parseElif()
{
    llvm::SmallVector<Equation *> Equations;
    Conditions *condits = parseConditions();

    if (!Tok.is(Token::KW_elif))
    {
        error();
        goto _error;
    }
    
    advance();

    if(!Tok.is(Token::KW_colon))
    {error();
        goto _error;}
    
    advance();

    if(!Tok.is(Token::KW_begin))
        {error();
        goto _error;}
    
    advance();


    while (!Tok.is(Token::KW_end)){
        if(Tok.is(Token::id)){
            Equation *eq = parseEquation();

        
        if (!Tok.is(Token::semicolon)){
            error();
            goto _error;
        }

        if (eq) {
            Equations.push_back(eq);
        }
        advance();
    }
    }
   
    return new  Elif(condits , Equations);
    

_error:
    while (Tok.getKind() != Token::eoi)
	    advance();
    return nullptr;



}

Else *Parser::parseElse(){
    llvm::SmallVector<Equation *> Equations;
    Conditions *cond = parseConditions();
    if(expect(Token::KW_else))
       {error();
        goto _error;}
    
    advance();

    if(expect(Token::KW_colon))
        {error();
        goto _error;}
    advance();

     if(expect(Token::KW_begin))
        {error();
        goto _error;}
    advance();


    while(!Tok.is(Token::KW_end))
    {
        if(Tok.is(Token::id)){
        Equation *Eq = parseEquation();
        if (!Tok.is(Token::semicolon)){
            error();
        goto _error;
        }
        if(Eq)
            Equations.push_back(Eq);

        advance();
        }
}

  

    return new Else(cond ,Equations);


_error:
    while(Tok.getKind() != Token::eoi)
	    advance();
    return nullptr;


}

//------------------------------------------------------------------------------


Conditions *Parser::parseConditions(){ // cop

    Conditions *Left = parseCondition();
    while (Tok.isOneOf(Token::KW_and , Token::KW_or)){
        Conditions::andOr AO = 
            Tok.is(Token::KW_and) ? Conditions::andOr::andd  : Conditions::andOr::orr;
        advance();
        Conditions *Right = parseCondition();
        Left = new Conditions(Left , AO  , Right);  
    }

    return Left;

}


Condition *Parser::parseCondition () {

    Expr *Left = parseExpr();
   // while (Tok.isOneOf(Token::KW_EqEq, Token::KW_eqNot , Token::KW_greaterEqual , Token::KW_lessEqual , Token::KW_lessThan , Token::KW_greaterThan)) {
        
        Condition::OperatorCondition Op;
            if (Tok.is(Token::KW_EqEq)) 
                Op = Condition::OperatorCondition::KW_EqEq;
            else if (Tok.is(Token::KW_eqNot))  
                Op= Condition::OperatorCondition::KW_eqNot;
            else if (Tok.is(Token::KW_greaterEqual))  
                Op= Condition::OperatorCondition::KW_greaterEqual;
            else if (Tok.is(Token::KW_lessEqual))  
                Op= Condition::OperatorCondition::KW_lessEqual;
            else if (Tok.is(Token::KW_lessThan))  
                Op= Condition::OperatorCondition::KW_lessThan;
            else if (Tok.is(Token::KW_greaterThan))  
                Op= Condition::OperatorCondition::KW_greaterThan;
            else {
                while (Tok.getKind() != Token::eoi)
                advance();
                return nullptr;
            }
            advance();
            Expr *Right = parseExpr();
            
    return new Condition( Left, Op, Right);

}

//----------------
Loop *Parser:: parseLoop(){
    
    Conditions *conditions = parseConditions();
    llvm::SmallVector<Equation*> Equ;



    if(!Tok.is(Token::KW_loopc)){
        error();
        goto _error;

    }
    advance();

    
    //advance();

    if(!Tok.is(Token::KW_colon)){
        error();
        goto _error;
    }

    advance();

    if (!Tok.is(Token::KW_begin))
    {
        error();
        goto _error;
    }
    advance();
    

    while(!Tok.is(Token::KW_end)){
        if(Tok.is(Token::id)){
            Equation *e = parseEquation();

            if (!Tok.is(Token::semicolon))
            {
                error();
                goto _error;
            }
            if(e)
            {
                Equ.push_back(e);
            }
            advance();
            
        }
    }

    return new Loop(conditions,Equ);
    
_error:
    while(Tok.getKind() != Token::eoi)
	    advance();
    return nullptr;
}






//-----------------------

