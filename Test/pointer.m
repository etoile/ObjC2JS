void *malloc(unsigned int);
void jsalert(int);
@interface NSObject
+new;
-alert;
@end

int main(void)
{
	int i = 1;
	jsalert((int)*(float*)&i);
	i = i + 1;
	jsalert(i);
	i += 1;
	jsalert(i);
	jsalert(i++);
	jsalert(i);
	char *str = "abc";
	char *ptr = str++;
	jsalert(*ptr);
	int *array = malloc(1024);
	float *alias;
	alias = (float*)array;
	alias[2] = 1;
	array[1] = alias[2];
	((id*)array)[12] = [NSObject new];
	jsalert(sizeof(array));
	jsalert(array[1]);
	jsalert(array[2]);
	jsalert(array[12]);
	jsalert(*((array+13) - 1));
	i = 12;
	jsalert(array[i]);

	[((id*)alias)[12] alert];

	return 0;
}
