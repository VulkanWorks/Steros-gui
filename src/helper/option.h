#ifndef STEROS_OPTION_H
#define STEROS_OPTION_H

typedef struct option_uint_tag option_uint;

struct option_uint_tag {
  bool has_value;
  uint32_t value;
};

STRS_INTERN void option_uint_create(option_uint *ptr);
STRS_INTERN void option_uint_set_value(option_uint *ptr, uint32_t value);

#ifdef IMPL_OPTION_DEF

STRS_INTERN void option_uint_set_value(option_uint *ptr, uint32_t value) {
  ptr->value = value;
  ptr->has_value = true;
}

STRS_INTERN void option_uint_create(option_uint *ptr) {
  ptr->has_value = false;
  ptr->value = 0;
}

#endif

#endif //STEROS_OPTION_H
