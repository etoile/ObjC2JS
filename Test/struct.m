void *malloc(unsigned int);
void jsalert(int);
@interface NSObject
+new;
-alert;
@end

struct foo
{
	int a;
	id b;
};

int main(void)
{
	struct foo*array = malloc(10*sizeof(struct foo));
	array[0].a = 12;
	array[0].b = [NSObject new];

	jsalert(array[0].a);
	[array[0].b alert];

	return 0;
}
