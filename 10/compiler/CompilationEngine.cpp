/**   CompilationEngine.cpp
 *    Recursive top-down parser.
 *    Each compile method has a comment above it describing the grammatical unit 
 *    that is being compiled, according to the following key:
 *          - 'xxx' :  A terminal which appears verbatim
 *          - xxx   :  A name of a non-terminal language construct
 *          - ()    :  Groups together language constructs
 *          - x|y   :  Either x or y can appear
 *          - x?    :  x appears 0 or 1 times
 *          - x*    :  x appears 0 or more times
 *    matthew.james.bird@gmail.com
 */

// TODO: I currently return EndOfInput or whatever instead of Premature End, but
// every end before the class } is a PrematureEnd!

#include "CompilationEngine.h"

#include "JackStatus.h"

#include <fstream>

using std::endl;
using std::cerr;

CompilationEngine::CompilationEngine(Tokeniser& tokeniser, std::ofstream& output)
    : m_tokeniser(tokeniser)
    , m_output(output)
{
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
        return logAndReturn("Expected class name", ParseFailure);
    if (!m_tokeniser.advance()) return PrematureEnd;

    // className
    if (m_tokeniser.tokenType() == IDENTIFIER)
        writeTextInTag(m_tokeniser.identifier(), "identifier");
    else
        return logAndReturn("Expected identifier after class name", ParseFailure);
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
            if (status == EndOfData)
            {
                return logAndReturn("Expected closing brace after class", PrematureEnd);
            }
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
            if (status == PrematureEnd)
            {
                return logAndReturn("File ended prematurely", PrematureEnd);
            }
            else if (status == EndOfData)
            {
                return logAndReturn("Expected closing brace after class", PrematureEnd);
            }
            return status;
        }
    }

    // }
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '}')
        writeTextInTag(m_tokeniser.symbol(), "symbol");
    else
        return logAndReturn("Expected closing brace after class", ParseFailure);

    m_output << "</class>" << endl;
    if (!m_tokeniser.advance())
    {
        return Success;
    }
    else
    {
        return compileClass();
    }
}

JackStatus CompilationEngine::compileType()
{
    if (m_tokeniser.tokenType() == KEYWORD)
    {
        if (m_tokeniser.tokenType() == KEYWORD 
            && (m_tokeniser.keyword() == INT 
                || m_tokeniser.keyword() == CHAR
                || m_tokeniser.keyword() == BOOLEAN))
        {
            writeTextInTag(m_tokeniser.currentToken(), "keyword"); 
        }
    }
    else if (m_tokeniser.tokenType() == IDENTIFIER)
    {
        writeTextInTag(m_tokeniser.currentToken(), "identifier");
    }
    else
    {
        return logAndReturn("Type must be int, char, boolean or class", ParseFailure);
    } 
    if (!m_tokeniser.advance()) return PrematureEnd;
    return Success;
}

// ('static' | 'field') type varName (',' varName)* ';'
JackStatus CompilationEngine::compileClassVarDec()
{
    m_output << "<classVarDec>" << endl;

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
    JackStatus status = compileType();
    if (status != Success) return status;

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

    m_output << "</classVarDec>" << endl;
    if (!m_tokeniser.advance()) return EndOfData;
    return Success;
}

// ('constructor' | 'function' | 'method') 
// ('void' | type) subroutineName '(' parameterList ')'
// subroutineBody
JackStatus CompilationEngine::compileSubroutine()
{
    m_output << "<subroutineDec>" << endl;

    // ('constructor' | 'function' | 'method') 
    if (m_tokeniser.tokenType() == KEYWORD
        && (m_tokeniser.keyword() == CONSTRUCTOR
            || m_tokeniser.keyword() == FUNCTION
            || m_tokeniser.keyword() == METHOD))
    {
        writeTextInTag(m_tokeniser.currentToken(), "keyword");
    }
    else
    {
        return logAndReturn("Subroutine definitions must begin with" 
                            "'constructor', 'function', or 'method'", ParseFailure);
    } 
    if (!m_tokeniser.advance()) return PrematureEnd;

    // ('void' | 'type')
    if (m_tokeniser.tokenType() == KEYWORD && m_tokeniser.keyword() == VOID)
    {
        writeTextInTag(m_tokeniser.currentToken(), "keyword");
        if (!m_tokeniser.advance()) return PrematureEnd;
    }
    else
    {
        JackStatus status = compileType();
        if (status != Success) return status;
    }

    // subroutineName
    if (m_tokeniser.tokenType() == IDENTIFIER)
    {
        writeTextInTag(m_tokeniser.identifier(), "identifier");
    }
    else
    {
        return logAndReturn("Subroutine must have a name", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    // (
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '(')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol"); 
    }
    else
    {
        return logAndReturn("Expected '('", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    JackStatus status = compileParameterList();
    if (status != Success) return status;

    // )
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ')')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol"); 
    }
    else
    {
        return logAndReturn("Expected ')'", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileSubroutineBody();

    m_output << "</subroutineDec>" << endl;
    if (status != Success)
    {
        return status;
    }
    return Success;
}

// '{' varDec* statements '}'
JackStatus CompilationEngine::compileSubroutineBody()
{
    m_output << "<subroutineBody>" << endl;
    JackStatus status = Success;

    // {
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '{')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Subroutine body must begin with '{'", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    // varDec*
    while (m_tokeniser.tokenType() == KEYWORD && m_tokeniser.keyword() == VAR)
    {
        status = compileVarDec();
        if (status != Success) return status;
    }

    status = compileStatements();
    if (status != Success) return status;

    // }
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '}')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Subroutine body must end with '}'", ParseFailure);
    }

    m_output << "</subroutineBody>" << endl;
    if (!m_tokeniser.advance()) return EndOfData;
    return Success;
}

// ((type varName) (',' type varName)*)?
JackStatus CompilationEngine::compileParameterList()
{
    m_output << "<parameterList>" << endl;
    JackStatus status = Success;

    // (type varName)?
    if (!(m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ')'))
    {
        status = compileType(); 
        if (status != Success) return status;
        if (m_tokeniser.tokenType() == IDENTIFIER)
        {
            writeTextInTag(m_tokeniser.identifier(), "identifier"); 
        }
        else
        {
            return logAndReturn("Parameter requires identifier", ParseFailure);
        }
        if (!m_tokeniser.advance()) return PrematureEnd;
        
        // (',' type varName)*
        while (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ',')
        {
            status = compileType();
            if (status != Success) return status;
            if (m_tokeniser.tokenType() == IDENTIFIER)
            {
                writeTextInTag(m_tokeniser.identifier(), "identifier"); 
            }
            else
            {
                return logAndReturn("Parameter requires identifier", ParseFailure);
            }
            if (!m_tokeniser.advance()) return PrematureEnd;
        }
    }

    m_output << "</parameterList>" << endl;
    return Success;
}

// 'var' type varName (',' varName)* ';'
JackStatus CompilationEngine::compileVarDec()
{
    m_output << "<varDec>" << endl;

    // 'var'
    if (m_tokeniser.tokenType() == KEYWORD && m_tokeniser.keyword() == VAR)
    {
        writeTextInTag(m_tokeniser.currentToken(), "keyword");
    }
    else
    {
        return logAndReturn("Variable declarations must begin with 'var'", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    // type
    JackStatus status = compileType();
    if (status != Success) return status;

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
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ';')
    {
         writeTextInTag(";", "symbol");
    }
    else
    {
        return logAndReturn("Variable declaration must end with ';'", ParseFailure);
    }

    m_output << "</varDec>" << endl;
    if (!m_tokeniser.advance()) return EndOfData;
    return Success;
}

// statement*
JackStatus CompilationEngine::compileStatements()
{
    JackStatus status = Success;
    while (m_tokeniser.tokenType() == KEYWORD)
    {
        switch(m_tokeniser.keyword())
        {
            case LET:
                status = compileLet();
                if (status != Success) return status;
                break;

            case IF:
                status = compileIf();
                if (status != Success) return status;
                break;

            case WHILE:
                status = compileWhile();
                if (status != Success) return status;
                break;

            case DO:
                status = compileDo();
                if (status != Success) return status;
                break;

            case RETURN:
                status = compileReturn();
                if (status != Success) return status;
                break;

            default:
                return Success;
        }
    }
    return Success;
}

// 'let' varName ('[' expression ']')? '=' expression ';'
JackStatus CompilationEngine::compileLet()
{
    JackStatus status = Success;
    m_output << "<letStatement>" << endl;

    // 'let'
    writeTextInTag(m_tokeniser.currentToken(), "keyword");        
    if (!m_tokeniser.advance()) return PrematureEnd;

    // varName
    if (m_tokeniser.tokenType() == IDENTIFIER)
    {
        writeTextInTag(m_tokeniser.identifier(), "identifier");
    }
    else
    {
        return logAndReturn("Let statement requires identifier", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;
  
    // ('[' expression ']')?
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '[')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
        if (!m_tokeniser.advance()) return PrematureEnd;
        
        status = compileExpression();
        if (status != Success) return status;
     
        if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ']')
        {
            writeTextInTag(m_tokeniser.currentToken(), "symbol");
            if (!m_tokeniser.advance()) return PrematureEnd;
        }
    }

    // '='
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '=')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected '=' in let statement", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileExpression();
    if (status != Success) return status;

    // ';'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ';')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected ';' following statement", ParseFailure);
    }

    m_output << "</letStatement>" << endl;
    if (!m_tokeniser.advance()) return EndOfData;

    return Success;
}

// 'if' '(' expression ')' '{' statements '}'
// ('else' '{' statements '}')?
JackStatus CompilationEngine::compileIf()
{
    JackStatus status = Success;

    m_output << "<ifStatement>" << endl;

    // 'if'
    writeTextInTag(m_tokeniser.currentToken(), "keyword");        
    if (!m_tokeniser.advance()) return PrematureEnd;

    // '('
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '(')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else 
    {
        return logAndReturn("Expected condition following 'if'", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileExpression();
    if (status != Success) return status;

    // ')'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ')')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected ')' following condition", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;
    
    // '{'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '{')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected block following condition", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileStatements();
    if (status != Success) return status;

    // '}'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '}')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected '}' following block", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    // ('else' '{' statements '}')?
    if (m_tokeniser.tokenType() == KEYWORD && m_tokeniser.keyword() == ELSE)
    {
        // 'else'
        writeTextInTag(m_tokeniser.currentToken(), "keyword");

        // '{'
        if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '{')
        {
            writeTextInTag(m_tokeniser.currentToken(), "symbol");
        }
        else
        {
            return logAndReturn("Expected block following condition", ParseFailure);
        }
        if (!m_tokeniser.advance()) return PrematureEnd;

        // statements
        status = compileStatements();
        if (status != Success) return status;

        // '}'
        if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '}')
        {
            writeTextInTag(m_tokeniser.currentToken(), "symbol");
        }
        else
        {
            return logAndReturn("Expected '}' following block", ParseFailure);
        }
        if (!m_tokeniser.advance()) return PrematureEnd;
    } 

    m_output << "</ifStatement>" << endl;
    return Success;
}

JackStatus CompilationEngine::compileWhile()
{
    JackStatus status = Success;
    m_output << "<whileStatement>" << endl;

    // 'while'
    writeTextInTag(m_tokeniser.currentToken(), "keyword");        
    if (!m_tokeniser.advance()) return PrematureEnd;

    // '('
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '(')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else 
    {
        return logAndReturn("Expected condition following 'while'", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileExpression();
    if (status != Success) return status;

    // ')'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ')')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected ')' following condition", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;
    
    // '{'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '{')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected block following condition", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileStatements();
    if (status != Success) return status;

    // '}'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '}')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected '}' following block", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    m_output << "</whileStatement>" << endl;
    return Success;
}

// 'do' subroutineCall ';'
JackStatus CompilationEngine::compileDo()
{
    JackStatus status = Success;
    m_output << "<doStatement>" << endl;

    writeTextInTag(m_tokeniser.currentToken(), "keyword");
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileSubroutineCall();
    if (status != Success) return status;

    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ';')
    {
        writeTextInTag(m_tokeniser.symbol(), "symbol");
    }
    else
    {
        return logAndReturn("Expected ';' following do statement", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    m_output << "</doStatement>" << endl;
    return Success;
}

// (subroutineName '(' expressionList ')') | ((className | varName)
// '.' subroutineName '(' expressionList ')')
JackStatus CompilationEngine::compileSubroutineCall()
{
    JackStatus status = Success;

    // subroutineName | className | varName
    if (m_tokeniser.tokenType() == IDENTIFIER)
    {
        writeTextInTag(m_tokeniser.identifier(), "identifier");
    }
    else
    {
        return logAndReturn("Expected identifier", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '.')
    {
        // '.'
        writeTextInTag(m_tokeniser.symbol(), "symbol");
        if (!m_tokeniser.advance()) return PrematureEnd;

        // subroutineName
        if (m_tokeniser.tokenType() == IDENTIFIER)
        {
            writeTextInTag(m_tokeniser.identifier(), "identifier");
        }
        else
        {
            return logAndReturn("Expected subroutine name", ParseFailure);
        }
    }
    
    // '('
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == '(')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected '(' following subroutine name", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;

    status = compileExpressionList();
    if (status != Success) return status;

    // ')'
    if (m_tokeniser.tokenType() == SYMBOL && m_tokeniser.symbol() == ')')
    {
        writeTextInTag(m_tokeniser.currentToken(), "symbol");
    }
    else
    {
        return logAndReturn("Expected ')' following expressionList", ParseFailure);
    }
    if (!m_tokeniser.advance()) return PrematureEnd;
    return Success;
}

// 'return' expression? ';'
JackStatus CompilationEngine::compileReturn()
{
    m_output << "<returnStatement>" << endl;

    // 'return'
    writeTextInTag(m_tokeniser.currentToken(), "keyword");
    if (!m_tokeniser.advance()) return PrematureEnd;

    if (m_tokeniser.currentToken() == SYMBOL && m_tokeniser.symbol() == ';')
    {
        writeTextInTag(m_tokeniser.symbol(), "symbol");    
        if (!m_tokeniser.advance()) return PrematureEnd;
    }
    else
    {
        JackStatus status = compileExpression();
        if (status != Success) return status;
    }
    m_output << "</returnStatement>" << endl;
    return Success;
}

JackStatus CompilationEngine::compileExpression()
{
    return Success;
}

JackStatus CompilationEngine::compileExpressionList()
{
    return Success;
}
