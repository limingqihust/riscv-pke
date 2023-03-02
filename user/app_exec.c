// /*exec2.c*/
//  #include "user_lib.h"
// #include "util/types.h"

// int main(int argc, char *argv[]) {
//   printu("\n======== exec /bin/app_read in app_exec ========\n");
//   printu("call exec in main\n");
//   int ret = exec("/bin/app_read");
//   if (ret == -1)
//     printu("exec failed!\n");

//   exit(0);
//   return 0;
// }



/*exec.c*/
#include "user_lib.h"
#include "util/types.h"

int main(int argc, char *argv[]) {
  printu("\n======== exec /bin/app_ls in app_exec ========\n");
  printu("call exec in main\n");
  int ret = exec("/bin/app_ls");
  if (ret == -1)
    printu("exec failed!\n");

  exit(0);
  return 0;
}
