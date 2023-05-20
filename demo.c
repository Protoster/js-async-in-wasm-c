#include <stddef.h>

#define EXPORT(name) __attribute__((export_name(#name)))
#define IMPORT(name) __attribute__((import_name(#name)))
typedef void callback(void *result, void *state);
typedef void* promise;

IMPORT(log)
void log_int(int value);

IMPORT(log_str)
void log_str(char *str);

IMPORT(fetch)
void fetch(char *path, callback callback, void *state);

IMPORT(resolve_promise)
void resolve_promise(promise promise, void *result);

extern unsigned char __heap_base; // address of this variable is the start of memory that we can do what we want with

EXPORT(malloc)
void *malloc(size_t size)
{
  static unsigned char *bump_pointer = &__heap_base;
  unsigned char *result = bump_pointer;
  bump_pointer += size;
  return result;
}

EXPORT(free)
void free(void *memory)
{
  // kekw
}

EXPORT(invoke_callback)
void invoke_callback(callback callback, void *result, void *state)
{
  callback(result, state);
}

/*
demo, demo_continuation_1, and demo_continuation_2 are parts of what would have been a single function that uses async, it might look something like this:

async int demo(void)
{
  log_str("B");  
  int my_int = 1;
  char *chars = await fetch("demo.c");

  log_str("C");
  log_int(my_int);
  my_int = 2;

  chars[8] = 0;
  log_str(chars);
  free(chars);

  chars = await fetch("build.bat");

  log_str("D");
  log_int(my_int);

  chars[5] = 0;
  log_str(chars);
  free(chars);

  return 42;
}

*/

static
void demo_continuation_2(void *result, void *state)
{
  log_str("D");
  int *my_int = state;
  log_int(*my_int);

  char *chars = result;
  chars[5] = 0;
  log_str(chars);  
  free(result); // we're responsible for freeing result

  // the other side is responsible for freeing this
  int *promiseResult = malloc(sizeof(int));
  *promiseResult = 42;
  resolve_promise(state, promiseResult);
  
  free(state);
}

static
void demo_continuation_1(void *result, void *state)
{
  log_str("C");
  int *my_int = state;
  log_int(*my_int);
  *my_int = 2;

  char *chars = result;
  chars[8] = 0;
  log_str(chars);
  free(result); // we're taking ownership of result memory and are responsible for freeing it

  fetch("build.bat", demo_continuation_2, state);
}

EXPORT(demo)
promise demo(void)
{
  log_str("B"); // these logs indicate the sequence of calls, they should appear in alphabetical order
  
  int *my_int = malloc(sizeof(*my_int)); // we're responsible for freeing this  
  *my_int = 1; // this variable shows how to have a "local variable" in our chain
  fetch("demo.c", demo_continuation_1, my_int);
  
  // We need to return some kind of unique identifier of the promise. A pointer to the memory we allocated for state will do fine.
  return my_int;
}
