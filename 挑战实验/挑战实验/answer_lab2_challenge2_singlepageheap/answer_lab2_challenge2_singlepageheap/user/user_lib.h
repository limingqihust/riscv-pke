/*
 * header file to be used by applications.
 */
typedef unsigned long long uint64;
int printu(const char *s, ...);
int exit(int code);
void* better_malloc();
void better_free(void* va);
uint64 naive_sbrk(uint64 n);
