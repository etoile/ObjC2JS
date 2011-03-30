void *malloc(unsigned int);
void jsalert(int);

int main(void)
{
	int *array = malloc(1024);
	float *alias = (float*)array;
	alias[2] = 1;
	array[1] = alias[2];
	jsalert(array[1]);
	jsalert(array[2]);
	return 0;
}
