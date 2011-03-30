void *malloc(unsigned int);
void jsalert(int);
@interface NSObject
+new;
-alert;
@end

int main(void)
{
	int *array = malloc(1024);
	float *alias = (float*)array;
	alias[2] = 1;
	array[1] = alias[2];
	((id*)array)[12] = [NSObject new];
	jsalert(array[1]);
	jsalert(array[2]);
	jsalert(array[12]);

	[((id*)array)[12] alert];

	return 0;
}
