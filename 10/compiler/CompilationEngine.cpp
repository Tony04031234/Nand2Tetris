/**   CompilationEngine.cpp
 *    Recursive top-down parser.
 *    matthew.james.bird@gmail.com
 */

#include "CompilationEngine.h"

#include "JackStatus.h"

#include <fstream>

using std::endl;
using std::cerr;

CompilationEngine::CompilationEngine(Tokeniser& tokeniser, std::ofstream& output)
    : m_tokeniser(tokeniser)
    , m_output(output)
{
    // output that first xml tag?
};

CompilationEngine::~CompilationEngine()
{
};

// class:   'class' className '{' classVarDec* subroutineDec* '}'
JackStatus CompilationEngine::compileClass()
{
    m_output << "<class>" << endl;
    
    // 'class'
    if (m_tokeniser.tokenType() == KEYWORD && m_tokeniser.keyword() == CLASS)
    {
        writeTextInTag("class", "keyword");
    }
    else
        return logAndReturn("ERROR: expected class name", ParseFailure);
    if (!m_tokeniser.advance()) return PrematureEnd;

    // className
    if (m_tokeniser.tokenType() == IDENTIFIER)
        writeTextInTag(m_tokeniser.identifier(), "identifier");
    else
        return logAndReturn("ERROR: expected identifier after class name", ParseFailure);
    if (!m_tokeniser.advance()) return PrematureEnd;

    // {
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '{')
        writeTextInTag(m_tokeniser.symbol(), "symbol"); 
    else
        return logAndReturn("Expected opening brace after class name", ParseFailure);
    if (!m_tokeniser.advance()) return PrematureEnd;

    // classVarDec*
    while (m_tokeniser.tokenType() == KEYWORD 
           && (m_tokeniser.keyword() == STATIC || m_tokeniser.keyword() == FIELD))
    {
        JackStatus status = compileClassVarDec();
        if (status != Success)
        {
            return status;
        }
    }

    // subroutineDec*
    while (m_tokeniser.tokenType() == KEYWORD
           && (m_tokeniser.keyword() == CONSTRUCTOR
               || m_tokeniser.keyword() == FUNCTION
               || m_tokeniser.keyword() == METHOD))
    {
        JackStatus status = compileSubroutine();
        if (status != Success)
        {
            return status;
        }
    }

    // }
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '}')
        writeTextInTag(m_tokeniser.symbol(), "symbol");
    else
        return logAndReturn("Expected closing brace after class", ParseFailure);
    if (!m_tokeniser.advance()) return PrematureEnd;

    m_output << "</class>" << endl;
    return Success;
}

// ('static' | 'field') type varName (',' varName)* ';'
JackStatus CompilationEngine::compileClassVarDec()
{
    // ('static' | 'field')
    if (m_tokeniser.tokenType() == KEYWORD 
        && (m_tokeniser.keyword() == STATIC || m_tokeniser.keyword() == FIELD))
    {
        writeTextInTag(m_tokeniser.currentToken(), "keyword");
    }
    else
    {
        return logAndReturn("Expected static or field keyword", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    // type
    if (m_tokeniser.tokenType() == KEYWORD)
    {
        if (m_tokeniser.tokenType() == KEYWORD 
            && (m_tokeniser.keyword() == INT 
                || m_tokeniser.keyword() == CHAR
                || m_tokeniser.keyword() == BOOLEAN))
        {
           writeTextInTag(m_tokeniser.currentToken(), "keyword"); 
        }
        else if (m_tokeniser.tokenType() == IDENTIFIER)
        {
            writeTextInTag(m_tokeniser.currentToken(), "identifier");
        }
        else
        {
            return logAndReturn("Return type must be int, char, boolean or class", ParseFailure);
        }
    } 
    if (!m_tokeniser.advance()) return PrematureEnd;

    // varName
    if (m_tokeniser.tokenType() == IDENTIFIER)
    {
        writeTextInTag(m_tokeniser.currentToken(), "identifier");
    }
    else
    {
        return logAndReturn("Variable declaration must have identifier", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    // (',' varName)*
    while (m_tokeniser.tokenType() == SYMBOL && !(m_tokeniser.symbol() == ';'))
    {
        // , 
        if (m_tokeniser.symbol() == ',')
        {
            writeTextInTag(",", "symbol");
        }
        else
        {
            return logAndReturn("Illegal symbol: expected ',' or ';'", ParseFailure);
        }
        if (!m_tokeniser.advance()) return PrematureEnd;
        
        // varName
        if (m_tokeniser.tokenType() == IDENTIFIER)
        {
            writeTextInTag(m_tokeniser.identifier(), "identifier");
        }
        else
        {
            return logAndReturn("Expected variable identifier", ParseFailure);
        }
        if (!m_tokeniser.advance()) return PrematureEnd;
    }

    // ;
    if (m_tokeniser.symbol() == ';')
    {
         writeTextInTag(";", "symbol");
    }
    else
    {
        return logAndReturn("Variable declaration must end with ';'", ParseFailure);
    }
    if (!m_tokeniser.advance()) return EndOfData;
    return Success;
}

JackStatus CompilationEngine::compileSubroutine()
{
    if (!m_tokeniser.advance()) return PrematureEnd;
    return Success;
}
