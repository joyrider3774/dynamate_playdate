#include "Functions.h"

#include <string.h>

char chr(int ascii)
{
	return((char)ascii);
}

int ord(char chr)
{
	return((int)chr);
}

void AddUnderScores (char *string)
{
	size_t Teller;
	for(Teller=0;Teller<strlen(string);Teller++)
		if(string[Teller] == ' ')
			string[Teller] = '_';
}

void RemoveUnderScores (char *string)
{
	size_t Teller;
	for(Teller=0;Teller<strlen(string);Teller++)
		if(string[Teller] == '_')
			string[Teller] = ' ';
}
