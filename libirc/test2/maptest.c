#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../src/defs.h"
#include "../src/map.h"

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
     int		value;
     map_t		map;
     map_prop_value_t	props[] = {
	  { MP_TABLE_SIZE,	VALUE(7) },
	  { MP_KEY_CMP_FUNC,	VALUE(strcmp) },
	  { MP_KEY_HASH_FUNC,	VALUE(hash_str) }
     };

     trace_start(2); /* trace to stderr */

     map = map_create(MT_HASHTABLE, props, sizeof(props) / sizeof(props[0]));

     map_put(map, KEY("one"),		VALUE(1));
     map_put(map, KEY("two"),		VALUE(2));
     map_put(map, KEY("three"),	VALUE(3));
     map_put(map, KEY("four"),		VALUE(4));
     map_put(map, KEY("five"),		VALUE(5));
     map_put(map, KEY("six"),		VALUE(6));
     map_put(map, KEY("seven"),	VALUE(7));
     map_put(map, KEY("eight"),	VALUE(8));
     map_put(map, KEY("nine"),		VALUE(9));
     map_put(map, KEY("ten"),		VALUE(10));

     map_get(map, KEY("three"), VALUEREF(value));
     printf("value = %d\n", value);
     printf("retval = %d\n", map_put(map, KEY("ten"), VALUE(8)));
     map_get(map, KEY("ten"), VALUEREF(value));
     printf("value = %d\n", value);

     map_destroy(map);

     trace_end();
     return 0;
}
