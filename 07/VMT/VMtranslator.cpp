/**
 *	File: VMtranslator.cpp
 *	Author: matthew.james.bird@gmail.com
 *
 *	Translates a single .vm file or a folder of .vm files into HACK assembly code.
 *	
 *	Usage: ./VMtranslator source
 *		where source is either a file of the form 'xxx.vm', or a directory of such files.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <dirent.h>

#include "Parser.hpp"
#include "CodeWriter.hpp"
#include "VMT.hpp"
#include "mediator.hpp"

using namespace std;
using namespace vmt;


int main(int argc, char* argv[])
{
	
	if (argc != 2)
	{
		cerr << "Usage: ./VMtranslator source" << endl;
		return 1;
	}
	
	string translatee = argv[1];	// file/directory to be translated
	
	// set up the output
	ofstream output;
	string outputName;
	
	if (isSingleFile(translatee))
	{
		outputName = translatee.substr(0, translatee.size()-3) + ".asm";
	}
	else
	{
		outputName = translatee + ".asm";
	}
	
	output.open(outputName.c_str());
	if (!output.is_open())
	{
		cerr << "Cannot open output file " << outputName << endl;
		return 1;
	}
	
	CodeWriter writer = CodeWriter(output);
	
	// translate .vm file or directory of .vm files
	if (isSingleFile(translatee))
	{
		ifstream source;
		source.open(translatee);
		if (!source.is_open())
		{
			cerr << "Cannot open source file " << translatee << endl;
			return 1;
		}
			
		Parser parser = Parser(source);

		translate(parser, writer);
		
		source.close();
	}
	else	// directory
	{
		vector<string> files;
		files = getVMFiles(translatee);
		
		// go through the files in the directory, translating each
		for (vector<string>::iterator iter = files.begin(); iter != files.end(); iter++)
		{
			ifstream source;
			source.open(*iter);
			if (!source.is_open())
			{
				cerr << "Error: cannot open " << *iter << endl;
	    		return 1;
			}
			
			Parser parser = Parser(source);
			
			translate(parser, writer);
			
			source.close();
		}
	}	
	output.close();
}
