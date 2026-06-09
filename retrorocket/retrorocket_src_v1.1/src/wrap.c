#include <stdlib.h>
#include <nds.h>

void *__real_malloc(size_t size);
void *__real_calloc(size_t nmemb, size_t size);
void *__real_realloc(void *ptr, size_t size);
void __real_free(void *ptr);

void *__wrap_malloc (size_t c) {
  int savedIME = REG_IME;
  REG_IME = IME_DISABLE;
  void *ptr = __real_malloc (c);
  REG_IME = savedIME;
  return ptr;
}

void *__wrap_calloc (size_t nmemb, size_t c) {
  int savedIME = REG_IME;
  REG_IME = IME_DISABLE;
  void *ptr = __real_calloc (nmemb, c);
  REG_IME = savedIME;
  return ptr;
}

void *__wrap_realloc(void *ptr, size_t size) {
  int savedIME = REG_IME;
  REG_IME = IME_DISABLE;
  void *ptr2 = __real_realloc(ptr, size);
  REG_IME = savedIME;
  return ptr2;
}

void __wrap_free(void *ptr) {
  int savedIME = REG_IME;
  REG_IME = IME_DISABLE;
  __real_free(ptr);
  REG_IME = savedIME;
}

