#ifndef __FUNCPTRS_H__
#define __FUNCPTRS_H__

typedef int (*key_cmp_func_t)(void* key_left, void* key_right);
typedef unsigned int (*key_hash_func_t)(void* value);
typedef void (*destructor_t)(void* object);

#endif /* __FUNCPTRS_H__ */
