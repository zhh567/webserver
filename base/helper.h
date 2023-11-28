#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void printStackTrace() {
  void* callstack[128];
  int frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);

  if (strs) {
    printf("Call stack:\n");
    for (int i = 0; i < frames; ++i) {
      printf("%s\n", strs[i]);
    }
    free(strs);
  }
}