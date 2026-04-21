#include <stdio.h>
#include <stdlib.h>
#include <emission.h>

int main()
{
    printf("%d\n", test);
    emitExecutable("a");
    printf("%d\n", test);
}

//extern void impFunc(void*);

int retInt()
{
	return 10;
}

void prntint(int arg)
{
	void* buf = malloc(100);	
	//impFunc(buf);
	printf("%d", arg);
	free(buf);
}

float retFlo(float f, int i)
{
	for (int it = 0; it < i; it++)
		f *= f;
	return f;
}
