#include "Parser.h"

// main point is that the whole input has been consumed
AST *Parser::parse()
{
    AST *Res = parseGSM();
    return Res;
}

AST *Parser::parseNAN()
{
    llvm::SmallVector<Expr *> exprs;
    while (!Tok.is(Token::eoi))
    {
        switch (Tok.getKind())
        {
        case Token::KW_int:
            Expr *d1;
            d1 = parseDec();
            if (d1)
                exprs.push_back(d1);
            else
                goto _error2;
            break;

            if (!Tok.is(Token::semicolon))
            {
                error();
                goto _error2;
            }

        case Token::id:
            Expr *d2;
            d2 = parseEquation();
            if (d2)
                exprs.push_back(d2);
            else
                goto _error2;
            break;

            if (!Tok.is(Token::semicolon))
            {
                error();
                goto _error2;
            }

        case Token::KW_loopc:
            Expr *d3;
            d3 = parseLoop();
            if (d3)
                exprs.push_back(d3);
            else
                goto _error2;
            break;

            if (!Tok.is(Token::semicolon))
            {
                error();
                goto _error2;
            }
        case Token::KW_if:
            Expr *d4;
            d4 = parseIf();
            if (d4)
                exprs.push_back(d4);
            else
                goto _error2;
            break;

            if (!Tok.is(Token::semicolon))
            {
                error();
                goto _error2;
            }
        default:
            goto _error2;
            break;
        }
        advance(); // TODO: watch this part
    }
    return new NAN(exprs);
_error2:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseDec()
{
    Expr *E;
    llvm::SmallVector<llvm::StringRef, 8> Vars;
    if (!(Tok.is(Token::KW_int) && ((Tok.is(Token::id)))
        goto _error;

    advance();
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
    }

    if (expect(Token::semicolon))
        goto _error;

    return new Declaration(Vars, E);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseEquation()
{
    Expr *E;
    Final *Fi;
   // F = (Factor *)(parseFactor());

    if (!Tok.is(Token::id) && !Tok.is(Token::num) && !Tok.is(Token::l_paren))
    {
        error();
        return nullptr;
    }

    advance();           // id += 2;
    E = parseExpr();
    Fi = parseFinal();
    return new Equation(Fi, E); //MUST EDIT AST
}

Expr *Parser::parseExpr()
{
    Expr *Left = parseTerm();
    while (Tok.isOneOf(Token::plus, Token::minus))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;
        advance();
        Expr *Right = parseTerm();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseTerm()
{
    Expr *Left = parseFactor();
    while (Tok.isOneOf(Token::star, Token::slash , Token::KW_mod))
    {
        BinaryOp::Operator Op =
            //Tok.is(Token::star) ? BinaryOp::star : BinaryOp::slash;
            if (Tok.is(Token::star)) 
                BinaryOp::star;
            else if (Tok.is(Token::slash))  
                BinaryOp::slash;
            else if (Tok.is(Token::KW_mod))  
                BinaryOp::KW_mod;  


        advance();
        Expr *Right = parseFactor();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseFactor()
{
    Expr *Left = parseFinal();
    while (Tok.isOneOf(Token::power))
    {
        BinaryOp::Operator Op = BinaryOp::power;
        
        advance();
        Expr *Right = parseFinal();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expr *Parser::parseFinal()
{
    Expr *Res = nullptr;
    switch (Tok.getKind())
    {
    case Token::num:
        Res = new Factor(Factor::num, Tok.getText());
        advance();
        break;
    case Token::id:
        Res = new Factor(Factor::id, Tok.getText());
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