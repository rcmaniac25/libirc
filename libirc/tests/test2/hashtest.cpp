#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../src/defs.h"
#include "../src/hashtable.h"

unsigned int hash_str(const char* t)
{
    unsigned int h = 0, g;
    for( ; *t; ++t) {
        h = (h << 4) + *t;
        if((g = h & 0xf0000000)) {
            h ^= g>>24;
            h ^= g;
        }
    }
    return h;
}



int main(void) 
{
     int value;
     hashtable_t ht;

     trace_start(2); /* trace to stderr */

     ht = ht_create((key_cmp_func_t)strcmp, (key_hash_func_t)hash_str, NULL, 7);

     ht_add(ht, KEY("one"),		VALUE(1));
     ht_add(ht, KEY("two"),		VALUE(2));
     ht_add(ht, KEY("three"),	VALUE(3));
     ht_add(ht, KEY("four"),		VALUE(4));
     ht_add(ht, KEY("five"),		VALUE(5));
     ht_add(ht, KEY("six"),		VALUE(6));
     ht_add(ht, KEY("seven"),	VALUE(7));
     ht_add(ht, KEY("eight"),	VALUE(8));
     ht_add(ht, KEY("nine"),		VALUE(9));
     ht_add(ht, KEY("ten"),		VALUE(10));

     ht_get(ht, KEY("three"), VALUEREF(value));
     printf("value = %d\n", value);
     printf("retval = %d\n", ht_add(ht, KEY("eleven"), VALUE(8)));
     ht_get(ht, KEY("eleven"), VALUEREF(value));
     printf("value = %d\n", value);

     ht_destroy(ht);

     trace_end();
     return 0;
}
