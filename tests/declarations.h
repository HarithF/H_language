int;
int x;
int *x;
int **x;
int (*(x));
int (*(x)) (int a, int b);

int f(int a, int b) {;}
int f(int a, int b) {}

int f(int a, int, int*, int(*)) {}

int f(void);