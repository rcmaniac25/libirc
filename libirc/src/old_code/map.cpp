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
#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include "conf.h"
#include "defs.h"
#include "map.h"
#include "hashtable.h"
#include "avltree.h"

#if !defined(USE_MAP_AVLTREE) && !defined(USE_MAP_HASHTABLE)
#error You must either define USE_MAP_AVLTREE or USE_MAP_HASHTABLE in order use maps
#endif

#if defined(USE_MAP_AVLTREE) && !defined(USE_AVLTREE)
#error You need to add avl tree support in order to use avl trees in maps
#endif

#if defined(USE_MAP_HASHTABLE) && !defined(USE_HASHTABLE)
#error You need to add hash table support in order to use hash tables in maps
#endif


struct _map {
     map_type_t		type;

     union {
#ifdef USE_MAP_AVLTREE
	  avl_tree_t	tree;
#endif 
#ifdef USE_MAP_HASHTABLE
	  hashtable_t	table;
#endif
     } impl;
};

static void* find_prop(map_prop_value_t props[], map_prop_t type, int num_props) {
     int i;

     for(i = 0; i < num_props; i++)
	  if(props[i].type == type)
	       return props[i].value;
     
     return NULL;
}

map_t map_create(map_type_t type, map_prop_value_t props[], unsigned int num_props)
{
     map_t	map = NULL;
     
     if(MT_TREE == type) {
#ifdef USE_MAP_AVLTREE
	  key_cmp_func_t     key_cmp_func = (key_cmp_func_t)find_prop(props, MP_KEY_CMP_FUNC, num_props);
	  destructor_t        val_destructor_func = (destructor_t)find_prop(props, MP_VALUE_DESTRUCTOR, num_props);

	  assert(key_cmp_func != NULL);

	  map = (map_t)malloc(sizeof(struct _map));	  
	  map->type = type;
	  map->impl.tree = avl_create(key_cmp_func, val_destructor_func);
#else
	  assert(0);
#endif
     } else if(MT_HASHTABLE == type) {
#ifdef USE_MAP_HASHTABLE
	  key_cmp_func_t	key_cmp_func = find_prop(props, MP_KEY_CMP_FUNC, num_props);
	  destructor_t		val_destructor_func = find_prop(props, MP_VALUE_DESTRUCTOR, num_props);
	  key_hash_func_t	key_hash_func = find_prop(props, MP_KEY_HASH_FUNC, num_props);
	  unsigned int		size = (unsigned int)find_prop(props, MP_TABLE_SIZE, num_props);

	  assert(key_cmp_func != NULL);
	  assert(key_hash_func != NULL);

	  map = malloc(sizeof(struct _map));
	  map->type = type;
	  map->impl.table = ht_create(key_cmp_func, key_hash_func, val_destructor_func, size);
#else
	  assert(0);
	  
#endif
     } else {
	  assert(0);
     }
     
     return map;
}

void map_destroy(map_t m) 
{
     assert(m != NULL);

     if(MT_TREE == m->type) {
#ifdef USE_MAP_AVLTREE
	  avl_destroy(m->impl.tree);
#else
	  assert(0);
#endif
     } else if(MT_HASHTABLE == m->type) {
#ifdef USE_MAP_HASHTABLE
	  ht_destroy(m->impl.table);
#else
	  assert(0);
#endif
     }
     free(m);
}

int map_put(map_t m, void* key, void* data) {
     assert(m != NULL);

     if(MT_TREE == m->type) {
#ifdef USE_MAP_AVLTREE
	  return avl_add(m->impl.tree, key, data);
#else
	  assert(0);
	  return FALSE;
#endif	
     } else if(MT_HASHTABLE == m->type) {
#ifdef USE_MAP_HASHTABLE
	  return ht_add(m->impl.table, key, data);
#else
	  assert(0);
	  return FALSE;
#endif 
     } else {
	  assert(0);
	  return FALSE;
     }
}

int map_get(map_t m, void* key, void** data)
{
     assert(m != NULL);
     
     if(MT_TREE == m->type) {
#ifdef USE_MAP_AVLTREE
	  return avl_find(m->impl.tree, key, data);
#else
	  assert(0); 
#endif
     } else if(MT_HASHTABLE == m->type) {
#ifdef USE_MAP_HASHTABLE
	  return ht_get(m->impl.table, key, data);
#else
	  assert(0);
	  return FALSE;
#endif
     } else {
	  assert(0);
	  return FALSE;
     }
}

void map_remove(map_t m, void* key, void** data)
{
     assert(m != NULL);

     if(MT_TREE == m->type) {
#ifdef USE_MAP_AVLTREE
	  avl_remove(m->impl.tree, key, data);
#else
	  assert(0);
#endif 
     } else if(MT_HASHTABLE == m->type) {
#ifdef USE_MAP_HASHTABLE
	  ht_remove(m->impl.table, key, data);
#else
	  assert(0);
#endif
     } else {
	  assert(0);
     }
}
