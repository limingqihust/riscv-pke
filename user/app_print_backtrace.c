/*
 * Below is the given application for lab1_challenge1_backtrace.
 * This app prints all functions before calling print_backtrace().
 */

#include "user_lib.h"
#include "util/types.h"

/*
输出依次调用的函数 回溯层数为x
输出x行 每一行为调用的函数符号名
到main函数为止
*/

void f8() { print_backtrace(7); }
void f7() { f8(); }
void f6() { f7(); }
void f5() { f6(); }
void f4() { f5(); }
void f3() { f4(); }
void f2() { f3(); }
void f1() { f2(); }

int main(void) {
  printu("back trace the user app in the following:\n");
  exit(0);
  return 0;

}
