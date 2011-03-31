static int j;
int test(void)
{
	static int i = 0;
	return (++i) + (++j);
}
void jsalert(int);

int main(void)
{
	test();
	test();
	jsalert(test());
}
