
int x = 0;
int y = 0;
int z = 0;


int main()
{
    int a = 0x10;
    int b = 2;
    int c = 3;
    int d = 4;

    int r1 = a + b * c - d;
    int r2 = (a + b) * (c - d);
    int r3 = a + (b * (c - d));
    int r4 = ((a + b) * c) - d;

	int result = r1 + r2 + r3 + r4;
	
	test:
		result = result + 1;
	if (result != 255) goto test;

    return result;

    test1:
		result1 = result1 + 1;
	if () goto test1;

    return result1;
}

// test comment

/*
"test:" convert to "test:"
"jump" convert to "if"
"if (condition) goto ;"
*/
