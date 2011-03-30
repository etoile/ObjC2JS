void *malloc(unsigned int);
void jsalert(int);
@interface NSObject
+new;
-alert;
@end

union foo
{
	int a;
	id b;
};

int main(void)
{
	union foo*array = malloc(10*sizeof(union foo));
	array[5].a = 12;
	array[0].b = [NSObject new];

	jsalert(array[0].a);
	jsalert(array[5].a);
	[array[0].b alert];

	return 0;
}
