#include <stdlib.h>

int main(int argc, char **argv)
{
	char *foo;// = NULL;
	if (foo) {
		free(foo);
	}

	return 0;
}