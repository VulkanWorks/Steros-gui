#ifndef STEROS_STRING_H
#define STEROS_STRING_H

#include "steros.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct strs_string_tag strs_string;

struct strs_string_tag {
  bool heap;
  union {
    wchar_t *heap_str;
    wchar_t stack_str[16];
  };
  uint64_t length;
};

STRS_NTD_LIB strs_string strs_string_create_from_cstr(const char *c_str, uint64_t length);
STRS_NTD_LIB strs_string strs_string_create_from_wstr(const wchar_t *str, uint64_t length);
STRS_NTD_LIB char *strs_string_to_cstr(strs_string *string);

#endif //STEROS_STRING_H

#ifdef NTD_IMPL_STRING

STRS_NTD_LIB strs_string strs_string_create_from_cstr(const char *c_str, uint64_t length) {
  strs_string result = {
    .length = length - 1,
    };
  if(length - 1 > 16) {
    result.heap = true;
    result.heap_str = malloc(sizeof(wchar_t) * length);
    for(uint64_t i = 0; i < length; i++) {
      result.heap_str[i] = (wchar_t)c_str[i];
    }
    return result;
  } else {
    result.heap = false;
    for(uint64_t i = 0; i < length; i++) {
      result.stack_str[i] = (wchar_t)c_str[i];
    }
    return result;
  }
}

STRS_NTD_LIB char *strs_string_to_cstr(strs_string *string) {
  if(string->heap) {
    char *result = malloc(sizeof(char) * (string->length + 1));
    memset(result, 0x00, string->length + 1);
    for(uint64_t i = 0; i < string->length; i++) {
      result[i] = (char)string->heap_str[i];
    }
    return result;
  } else {
    char *result = malloc(sizeof(char) * (string->length + 1));
    memset(result, 0x00, string->length + 1);
    for(uint64_t i = 0; i < string->length + 1; i++) {
      result[i] = (char)string->stack_str[i];
    }
    return result;
  }
}

STRS_NTD_LIB strs_string strs_string_create_from_wstr(const wchar_t *str, uint64_t length) {
  strs_string result = {
    .length = length - 1,
    };

  if(length - 1 > 16) {
    result.heap = true;
    result.heap_str = malloc(sizeof(wchar_t) * length);
    for(uint64_t i = 0; i < length; i++) {
      result.heap_str[i] = str[i];
    }
    return result;
  } else {
    result.heap = false;
    for(uint64_t i = 0; i < length; i++) {
      result.stack_str[i] = str[i];
    }
    return result;
  }
}

#endif
