/*  Programmable IRC function library 
    Copyright (C) 1999-2002 Jorgen Sigvardsson <jorgen@wermland.se>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __MAP_H__
#define __MAP_H__

#include "funcptrs.h"

#ifdef __cplusplus
extern "C" {
#endif 

struct _map;
typedef struct _map* map_t;

typedef enum _map_type {
    MT_TREE,
    MT_HASHTABLE
} map_type_t;

typedef enum _map_prop {
    MP_TABLE_SIZE,
    MP_KEY_CMP_FUNC,
    MP_VALUE_DESTRUCTOR,
    MP_KEY_HASH_FUNC,
} map_prop_t;

typedef struct _map_prop_value {
    map_prop_t	type;
    void*	value;
} map_prop_value_t;

map_t      map_create(map_type_t type, map_prop_value_t props[], unsigned int num_props);
void       map_destroy(map_t m);

int        map_put(map_t m, void* key, void* data);
int        map_get(map_t m, void* key, void** data);
void       map_remove(map_t m, void* key, void** data);

#ifdef __cplusplus
}
#endif 

#endif /* __MAP_H__ */
