#include <dlfcn.h>
#include "test.h"
LIB("main")

int main (int argc, char *argv[])
{
  printf ("enter main\n");
  void *h = dlopen ("libtest.so", RTLD_LAZY);
  printf ("dlopen completed\n");
  dlclose (h);
  printf ("leave main\n");
  return 0;
}
