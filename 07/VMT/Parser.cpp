/**
 *  File: VMparser.cpp
 *  
 *  Handles the parsing of a single .vm file, and encapsulates access to the input code. 
 *  It reads VM commands, parses them, and provides convenient access to their components. 
 *  In addition, it removes all white space and comments.
 *
 *  Author: matthew.james.bird@gmail.com
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>

#include "Parser.hpp"

using namespace vmt;
using namespace std;

Parser::Parser(ifstream& file, string name)
    : source(file), fileName(name)
{}
    
/** Returns whether there are more commands in source. */
bool Parser::hasMoreCommands()
{
    return (source.peek() != EOF);
}

/** Reads the next command from input. Returns false if command empty, else true */
bool Parser::advance()
{
    string temp;
    char c;
    bool insertSpace = false;
    while ((source >> noskipws >> c) && (c != EOF) && (c != '\n') && (c != '\r') && (c != '/'))
    {
        if (isspace(c) && insertSpace)
        {
            temp += " ";
            insertSpace = false;
        }
        else if (!isspace(c))
        {
            temp += c;
            insertSpace = true;
        }
    }
    
    // continue reading until newline or EOF
    while (c != '\n' && c != EOF && (source >> noskipws >> c));
    
    command = temp;
    // remove trailing whitespace (Jaysus, std C++ sucks...)
    command = std::regex_replace(command, std::regex(" +$"), "");
 
    if (command.size() == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/** Returns the type of the current command. */
command_t Parser::commandType()
{
    if (command.compare(0, 4, "push") == 0)
    {
        return C_PUSH;
    }
    else if (command.compare(0, 3, "pop") == 0)
    {
        return C_POP;
    }
    else if (command.compare(0, 5, "label") == 0)
    {
        return C_LABEL;
    }
    else if (command.compare(0, 4, "goto") == 0)
    {
        return C_GOTO;
    }
    else if (command.compare(0, 7, "if-goto") == 0)
    {
        return C_IF;
    }
    else if (command.compare(0, 8, "function") == 0)
    {
        return C_FUNCTION;
    }
    else if (command.compare(0, 6, "return") == 0)
    {
        return C_RETURN;
    }
    else if (command.compare(0, 4, "call") == 0)
    {
        return C_CALL;
    }
    else 
    {
        return C_ARITHMETIC;
    }
}

/** Returns the first argument of the current command.
    In the case of C_ARITHMETIC, the command itself (add, sub, etc.) is returned.
    Should not be called in the current command is C_RETURN */
string Parser::arg1()
{
    if (commandType() == C_ARITHMETIC)
    {
        return command;
    }
    else
    {
        return nth_argument(1);
    }
}

/** Returns the second argument of the current command.
    Should be called only if the current command is C_PUSH, C_POP, C_FUNCTION, or C_CALL. */
string Parser::arg2()
{
    return nth_argument(2);
}

/** Returns the nth argument of the current command. */
string Parser::nth_argument(int n)
{
    string buf;
    stringstream ssCommand(command);
    int word = 0;
    while (word++ != n+1)
    {
        ssCommand >> buf;
    }
    return buf;
}

/** Returns the name of the source file */
string& Parser::getFileName()
{
    return fileName;
}

