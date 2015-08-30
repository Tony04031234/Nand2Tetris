/**
 *	file: assembler.c
 *	author: matthew.james.bird@gmail.com
 *
 *	assembler for the hack assembly language.
 *
 *	usage: assembler source output
 */

#include <stdio.h>
#include <stdbool.h>	// bool type
#include <ctype.h>		// isspace(), isdigit()
#include <stdlib.h>		// atoi()

/** 
itob: reads in an A instruction from source, and outputs the a-instruction to out, converted to binary
*/
void itob(FILE* source, FILE* output)
{
	char* number = malloc(6); //holds the number in the @instruction
	if (number == NULL)
	{
		fprintf(stderr, "Error (itob): cannot malloc number\n");
	}
	
	// read in the @ instruction
	int i = 0;
	char c;
	while ((c = fgetc(source)) && isdigit(c))
	{
		if (i > 4)
		{
			fprintf(stderr, "Error (itob): integer too large\n");
		}
		number[i++] = c;
	}
	number[i] = '\0';
	
	// rewind one byte
	fseek(source, -1, SEEK_CUR);
	
	// convert the number to int
	int v = atoi(number);
	
	// output the a-instruction converted to binary
	for (i = 15; i >= 0; i--)
		fputc('0' + ((v >> i) & 1), output);
}

int main(int argc, char* argv[])
{
	// check input is correct
	if (argc != 3)
	{
		fprintf(stderr, "Usage: assembler source output\n");
		return 1;
	}
	
	// open source file
	FILE* source = fopen(argv[1], "rb");
	if (source == NULL)
	{
		fprintf(stderr, "Error: cannot open source file %s", argv[1]);
		return 1;
	}
	
	// open output file
	FILE* out = fopen(argv[2], "wb");
	if (out == NULL)
	{
		fprintf(stderr, "Error: cannot open output file %s", argv[2]);
		fclose(source);
		return 1;
	}
	
	// main read loop
	char c;
	bool comment = false;
	bool content = false;	// whether there is content on the current line
	char number[6];			// holds the number in the @instruction
	int i; 		// iterator
	while ((c = fgetc(source)) != EOF)
	{
		if (c == '/')
		{
			comment = true;
		}
		else if (c == '\n')
		{
			if (content)	// only print newlines if there was content on that line
			{
				fputc('\n', out);
			}
			comment = false;	// newline breaks comments
			content = false;
		}
		else if (isspace(c))
		{
			continue;
		}
		else if (comment) 
		{
			continue;	// skip comments
		}
		else if (c == '@')	// A-INSTRUCTION
		{
			content = true;
			itob(source, out);
		}
	}
	fputc('\n', out);
}