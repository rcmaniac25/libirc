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
#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include "funcptrs.h"

#ifdef __cplusplus
extern "C" {
#endif 

struct _hashtable;
typedef struct _hashtable* hashtable_t;

hashtable_t	ht_create(key_cmp_func_t key_cmp_func, key_hash_func_t key_hash_func,
			  destructor_t, unsigned int size);
void		ht_destroy(hashtable_t ht);

int		ht_add(hashtable_t ht, void* key, void* value);
int		ht_get(hashtable_t ht, void* key, void** value);
int		ht_remove(hashtable_t ht, void* key, void** value); 

#ifdef __cplusplus
}
#endif 

#endif /* __HASHTABLE_H__ */
