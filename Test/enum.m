enum test
{
	one = 1,
	two = 2,
	three = one + two
};
void jsalert(enum test);

int main(void)
{
	jsalert(one);
	jsalert(two);
	jsalert(three);
	return 0;
}
