#include "Lexer.h"

// classifying characters
namespace charinfo
{
    // ignore whitespaces
    LLVM_READNONE inline bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
               c == '\r' || c == '\n';
    }
    

    LLVM_READNONE inline bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    LLVM_READNONE inline bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
}

void Lexer::next(Token &token)
{
    while (*BufferPtr && charinfo::isWhitespace(*BufferPtr))
    {
        ++BufferPtr;
    }
    // make sure we didn't reach the end of input
    if (!*BufferPtr)
    {
        token.Kind = Token::eoi;
        return;
    }
    // collect characters and check for keywords or ident
    if (charinfo::isLetter(*BufferPtr) )
    {
        const char *end = BufferPtr + 1;
        while (charinfo::isLetter(*end) || charinfo::isDigit(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr); //   intdxfghjklgyfh , sw6 , 
        Token::TokenKind kind;
        
        if (Name == "int")
            kind = Token::KW_int;
        else if (Name=="if")
            kind=Token::KW_if;
        else if (Name=="elif")
            kind=Token::KW_elif;
        else if (Name=="else")
            kind=Token::KW_else;
        else if (Name=="end")
            kind=Token::KW_end;
        else if (Name=="begin")
            kind=Token::KW_begin;
        else if (Name=="and")
            kind=Token::KW_and;
        else if (Name=="or")
            kind=Token::KW_or;
        else if (Name=="loopc")
            kind=Token::KW_loopc;
        else
            kind = Token::id;
        // generate the token
        formToken(token, end, kind);
        return;
    }
    // check for numbers
    else if (charinfo::isDigit(*BufferPtr))
    {
        const char *end = BufferPtr + 1;
        while (charinfo::isDigit(*end)) //852369854
            ++end;
        formToken(token, end, Token::num);
        return;
    }
    else
    {
        if (*BufferPtr == '+' && *(BufferPtr+1) == '=') //                   += 9
            formToken(token, BufferPtr + 2, Token::KW_plusEqual);
        else if (*BufferPtr == '-' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_minusEqual);
        else if (*BufferPtr == '*' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_starEqual);
        else if (*BufferPtr == '/' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_slashEqual);
        else if (*BufferPtr == '%' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_modEq);
        else if (*BufferPtr == '^' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_poEq);
        else if (*BufferPtr == '>' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_greaterEqual);
        else if (*BufferPtr == '<' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_lessEqual);
        else if (*BufferPtr == '=' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_EqEq);
        else if (*BufferPtr == '!' && *(BufferPtr+1) == '=')
            formToken(token, BufferPtr + 2, Token::KW_eqNot);
        else if (*BufferPtr == '+')
            formToken(token, BufferPtr + 1, Token::plus);
        else if (*BufferPtr == '-')
            formToken(token, BufferPtr + 1, Token::minus);
        else if (*BufferPtr == '*')
            formToken(token, BufferPtr + 1, Token::star);
        else if (*BufferPtr == '/')
            formToken(token, BufferPtr + 1, Token::slash);
        else if (*BufferPtr == '%')
            formToken(token, BufferPtr + 1, Token::KW_mod);
        else if (*BufferPtr == '^')
            formToken(token, BufferPtr + 1, Token::power);
        else if (*BufferPtr == '(')
            formToken(token, BufferPtr + 1, Token::l_paren);
        else if (*BufferPtr == ')')
            formToken(token, BufferPtr + 1, Token::r_paren);
        else if (*BufferPtr == ';')
            formToken(token, BufferPtr + 1, Token::semicolon);
        else if (*BufferPtr == ',')
            formToken(token, BufferPtr + 1, Token::comma);
        else if (*BufferPtr == '=')
            formToken(token, BufferPtr + 1, Token::equal);
        else if (*BufferPtr == '>')
            formToken(token, BufferPtr + 1, Token::KW_greaterThan);
        else if (*BufferPtr == '<')
            formToken(token, BufferPtr + 1, Token::KW_lessThan);
        else if (*BufferPtr == ':')
            formToken(token, BufferPtr + 1, Token::KW_colon);
        //return;
        else 
            formToken(token, BufferPtr + 1, Token::unknown);

    }
}

void Lexer::formToken(Token &Tok, const char *TokEnd,
                      Token::TokenKind Kind)
{
    Tok.Kind = Kind;
    Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
    BufferPtr = TokEnd;
}
