void *malloc(unsigned int);
void jsalert(const char*);

@interface NSObject
+alloc;
+new;
-init;
-retain;
-(void)release;
-(void)alert;
@end
@interface Test : NSObject
@property int foo;
@end
@implementation Test
@synthesize foo;
- self { return self; }
@end

struct s
{
	int a;
	float b;
	const char *s;
	id obj;
};

void takesPointer(struct s*v)
{
	v->s = "Correctly set";
}
void takesCopy(struct s v)
{
	v.s = "Incorrectly set";
}

#define FOO(x) do { x; } while(0)

int bar(int a) { return 2*a; }

int main(int b, char**argv)
{
	int a = 12, c;
	c = (a + b) * bar(a);
	for (int i=0 ; i< 100 ; i++)
	{
		int j;
		while ((j = (a < 12)))
		{
			do {
			a++;
			} while (0);
		}
	}
	FOO(a++);
	Test *f = [Test new];
	f.foo += 1;
	[f alert];

	int *array = malloc(1024);
	float *alias = (float*)array;
	array[1] = alias[2];

	struct s structVarNoInit;
	struct s structVar = { .b = 1,.a= 2, .s ="C string", .obj = @"ObjC String"};
	int array2[12];
	int element = array2[1];
	takesPointer(&structVar);
	takesCopy(structVar);
	jsalert(structVar.s);
}
