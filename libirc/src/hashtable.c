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
#include "conf.h"

#ifdef USE_HASHTABLE
#include <stdlib.h>
#include <assert.h>
#include "defs.h"
#include "hashtable.h"
#include "list.h"
#include "trace.h"

typedef struct _key_value_pair {
     void*		key;
     void*		value;
}* key_value_pair_t;

struct _hashtable {
     slist_t*		table;
     unsigned int	size;
     key_cmp_func_t	key_cmp_func;
     key_hash_func_t	key_hash_func;
     destructor_t	val_destructor_func;
};

static int _ht_find_in_list(slist_t list, void* key, key_cmp_func_t cmp_func, key_value_pair_t* pair);
static int _ht_pair_key_cmp(key_value_pair_t pair, key_value_pair_t arg);

hashtable_t ht_create(key_cmp_func_t key_cmp_func, key_hash_func_t key_hash_func,
		      destructor_t val_destructor_func, unsigned int size)
{
     hashtable_t	ht;
     assert(key_cmp_func != NULL);
     assert(key_hash_func != NULL);
     assert(size != 0);


     ht = malloc(sizeof(struct _hashtable));
     ht->size = size;
     ht->key_cmp_func = key_cmp_func;
     ht->key_hash_func = key_hash_func;
     ht->val_destructor_func = val_destructor_func;
     ht->table = calloc(ht->size, sizeof(slist_t));
     
     // Woops! out of memory...
     if(ht->table == NULL) {
	  free(ht);
	  return NULL; 
     }

     return ht;
}

void ht_destroy(hashtable_t ht)
{
     unsigned int i;
     for(i = 0; i < ht->size; ++i) {
	  if(ht->table[i] != NULL) {
	       slist_t l;

	       if(ht->val_destructor_func != NULL) {
		    for(l = ht->table[i]; l != EMPTY_LIST; l = slist_next(l)) {
			 key_value_pair_t pair;
			 pair = (key_value_pair_t)slist_value(l);

			 ht->val_destructor_func(pair->value);
		    }
	       }
	       slist_destroy(ht->table[i], (destructor_t)free);
	  }
     }
     free(ht->table);
     free(ht);
}

int ht_add(hashtable_t ht, void* key, void* value)
{
     unsigned int	hv;
     key_value_pair_t	pair;

     hv = ht->key_hash_func(key) % ht->size;

     if(!_ht_find_in_list(ht->table[hv], key, ht->key_cmp_func, &pair)) {
	  pair = malloc(sizeof(struct _key_value_pair));
	  pair->key = key;
	  pair->value = value;
	  ht->table[hv] = slist_prepend(ht->table[hv], pair);
     } else
	  return FALSE;

     return TRUE;
}

int ht_get(hashtable_t ht, void* key, void** value)
{
     unsigned int	hv;
     key_value_pair_t	pair;
     assert(value != NULL);

     hv = ht->key_hash_func(key) % ht->size;

     if(!_ht_find_in_list(ht->table[hv], key, ht->key_cmp_func, &pair))
	  return FALSE;

     *value = pair->value;
     return TRUE;
}

int ht_remove(hashtable_t ht, void* key, void** value)
{
     unsigned int		hv;
     key_value_pair_t		pair;
     struct _key_value_pair	p;

     p.key = key;
     p.value = ht;

     hv = ht->key_hash_func(key) % ht->size;

     ht->table[hv] = slist_remove_if(ht->table[hv], (key_cmp_func_t)_ht_pair_key_cmp, &p, VALUEREF(pair));
     if(pair) {
	  if(value != NULL)
	       *value = pair->value;
	  else if(ht->val_destructor_func != NULL)
	       ht->val_destructor_func(pair->value);
	  free(pair);
     }

     return TRUE;
}

static int _ht_find_in_list(slist_t list, void* key, key_cmp_func_t cmp_func, key_value_pair_t* pair)
{
     slist_t i;

     for(i = list; i != EMPTY_LIST; i = slist_next(i)) {
	  key_value_pair_t    val;

	  val = (key_value_pair_t)slist_value(i);
	  if(cmp_func(val->key, key) == 0) {
	       *pair = val;
	       return TRUE;
	  }
     }

     return FALSE;
}

static int _ht_pair_key_cmp(key_value_pair_t pair, key_value_pair_t arg)
{
     hashtable_t ht = (hashtable_t)arg->value;
     return ht->key_cmp_func(pair->key, arg->key);
}

#endif /* USE_HASHTABLE */
