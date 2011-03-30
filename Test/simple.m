struct foo
{
	int c;
	struct 
	{
		id a;
	} b;
} s;
void jsalert(const char*);

@interface NSObject
+new;
-alert;
@end

int main(void)
{
	s.b.a = [NSObject new];
	struct foo bar = { 12, { 0 }};
	[s.b.a alert];
	id x;
	[x alert];
}
