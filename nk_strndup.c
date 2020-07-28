#include "nk_strndup.h"

#ifndef strndup
#ifdef _WIN32
#pragma message "Using own strndup"
char * strndup(const char* str, size_t maxlen){
  size_t len = strnlen(str, maxlen);
  char* dup = (char *) malloc(len + 1);
  memcpy(dup, str, len);
  dup[len] = 0;
  return dup;
}
#endif
#endif