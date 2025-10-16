
struct get_fnct
{
	int (*f) (/* void *p */);
	void *p;
};

struct put_fnct
{
	void (*f) (/* void *p, char c */);
	void *p;
};

