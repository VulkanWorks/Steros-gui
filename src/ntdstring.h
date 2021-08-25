#ifndef STEROS_NTDSTRING_H
#define STEROS_NTDSTRING_H

#include "steros.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
  bool heap;
  union {
    wchar_t *heapStr;
    wchar_t stackStr[16];
  };
  uint64_t size;
} StrsStr;

STRS_NTD_LIB StrsStr strsStrCreateFromCString(const char *c_str, uint64_t size);
STRS_NTD_LIB StrsStr strsStrCreate(const wchar_t *str, uint64_t size);
STRS_NTD_LIB char *strsStrToCString(StrsStr *string);

#ifdef STRS_IMPL_STR
STRS_NTD_LIB StrsStr strsStrCreateFromCString(const char *c_str, uint64_t size) {
  StrsStr result = {
    .size = size - 1,
  };
  if(size - 1 > 16) {
    result.heap = true;
    result.heapStr = malloc(sizeof(wchar_t) * size);
    for(uint64_t i = 0; i < size; i++) {
      result.heapStr[i] = (wchar_t)c_str[i];
    }
    return result;
  } else {
    result.heap = false;
    for(uint64_t i = 0; i < size; i++) {
      result.stackStr[i] = (wchar_t)c_str[i];
    }
    return result;
  }
}

STRS_NTD_LIB char *strsStrToCString(StrsStr *string) {
  if(string->heap) {
    char *result = malloc(sizeof(char) * (string->size + 1));
    memset(result, 0x00, string->size + 1);
    for(uint64_t i = 0; i < string->size; i++) {
      result[i] = (char)string->heapStr[i];
    }
    return result;
  } else {
    char *result = malloc(sizeof(char) * (string->size + 1));
    memset(result, 0x00, string->size + 1);
    for(uint64_t i = 0; i < string->size + 1; i++) {
      result[i] = (char)string->stackStr[i];
    }
    return result;
  }
}

STRS_NTD_LIB StrsStr strsStrCreate(const wchar_t *str, uint64_t size) {
  StrsStr result = {
    .size = size - 1,
    };

  if(size - 1 > 16) {
    result.heap = true;
    result.heapStr = malloc(sizeof(wchar_t) * size);
    for(uint64_t i = 0; i < size; i++) {
      result.heapStr[i] = str[i];
    }
    return result;
  } else {
    result.heap = false;
    for(uint64_t i = 0; i < size; i++) {
      result.stackStr[i] = str[i];
    }
    return result;
  }
}

#endif //STRS_IMPL_STR

#endif //STEROS_NTDSTRING_H
