
void jsalert(char);

void passedPointer(char *x)
{
	*x = 'a';
}
void passedValue(char x)
{
	x = 'b';
}

int main(void)
{
	char a;
	passedPointer(&a);
	passedValue(a);
	jsalert(a);
}
