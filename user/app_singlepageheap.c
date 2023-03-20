

#include "user_lib.h"
#include "util/types.h"
#include "util/string.h"
int main(void) {

  char str[20] = "cross page";
  char *m = (char *)better_malloc(100);
  char *p = (char *)better_malloc(4096);  // cross page
  if((uint64)p - (uint64)m > 512 ){
    printu("you need to manage the vm space precisely!");
    exit(-1);
  }
  better_free((void *)m);

  strcpy(p,str);
  printu("%s\n",p);
  exit(0);
  return 0;
}




//#include "user_lib.h"
////#include "util/string.h"
//
//typedef unsigned long long uint64;
//
//char* strcpy(char* dest, const char* src) {
//    char* d = dest;
//    while ((*d++ = *src++))
//        ;
//    return dest;
//}
//int main(void) {
//
//    char str[20] = "hello, world!!!";
//    char *m = (char *)better_malloc(100);
//    printu("m is %llx\n",(uint64)m);
//    char *p = (char *)better_malloc(50);
//    printu("p is %llx\n",(uint64)p);
//    if((uint64)p - (uint64)m > 512 ){
//        printu("you need to manage the vm space precisely!");
//        exit(-1);
//    }
//    better_free((void *)m);
//
//    strcpy(p,str);
//    printu("%s\n",p);
//    char *n = (char *)better_malloc(50);
//
//    if(m != n)
//    {
//        printu("your malloc is not complete.\n");
//        exit(-1);
//    }
//    exit(0);
//    return 0;
//}
