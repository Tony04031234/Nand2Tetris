/**   CompilationEngine.h
 *    Recursive top-down parser.
 *    Matches the tokenised stream of atoms to the syntax rules of Jack.
 *    matthew.james.bird@gmail.com
 */

#ifndef compilationEngine_h
#define compilationEngine_h

#include "JackStatus.h"
#include "Tokeniser.h"

#include <fstream>

/** 
 *  Effects the actual compilation output. Gets its input from Tokeniser and
 *  emits its parsed struture into an output file. The output is generated by
 *  a series of compilexxx() routines, one for every syntactic element xxx of
 *  the Jack grammar. The contract between these routines is that each
 *  compilexxx() routine should read the syntactic construct xxx from the input,
 *  advance() the tokeniser exactly beyond xxx, and output the parsing of xxx.
 *  Thus, compilexxx() may only be called if indeed xxx is the next syntactic
 *  element of the input.
 */
class CompilationEngine {
public:
    CompilationEngine(Tokeniser&, std::ofstream&);
    ~CompilationEngine();

    JackStatus compileClass();

    /** Compiles static or field declaration */
    JackStatus compileClassVarDec();

    /** Compiles a complete method, function, or constructor */
    JackStatus compileSubroutine();

    /** Compiles a (poss. empty) parameter list, not including the '()' */
    JackStatus compileParameterList();

    /** Compiles a var declaration */
    JackStatus compileVarDec();
    
    /** Compiles a sequence of statements, not including the '{}' */
    JackStatus compileStatements();

    JackStatus compileDo();

    JackStatus compileLet();

    JackStatus compileWhile();

    JackStatus compileReturn();

    /** Compiles an if statement, possibly with a trailing 'else' clause */
    JackStatus compileIf();

    JackStatus compileExpression();

    /** Compiles a term. If the current token is an identifier, the routine
     *  must distinguish between a variable, an array entry, and a subroutine
     *  call. A single look-ahead token, which may be one of '[', '(', or '.'
     *  suffices to distinguish between the three possibilities (any other
     *  token is not part of this term and should be advanced over)
     */
    JackStatus compileTerm(); 

    /** Compiles a (possibly empty) comma-separated list of expressions */
    JackStatus compileExpressionList();

private:
    CompilationEngine();
    
    inline void writeTextInTag(const std::string text, const std::string tag)
    {
        m_output << "<" << tag << "> " << text << " </" << tag << ">" << std::endl;
    }

    inline void writeTextInTag(const char text, const std::string tag)
    {
        m_output << "<" << tag << "> " << text << " </" << tag << ">" << std::endl;
    }

    inline JackStatus returnAndLogError(const char* errorMessage)
    {
        std::cerr << "PARSE ERROR" << std::endl;
        std::cerr << "File: " << m_tokeniser.filename() << std::endl;
        std::cerr << "Line: " << m_tokeniser.lineNumber() << std::endl;
        std::cerr << errorMessage << std::endl;
        return ParseFailure;
    }

    Tokeniser& m_tokeniser;

    std::ofstream& m_output;
};

#endif // compilationEngine_h    