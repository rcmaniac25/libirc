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
#ifndef __AVLTREE_H__
#define __AVLTREE_H__

#include "funcptrs.h"

#ifdef __cplusplus
extern "C" {
#endif 

struct _avl_tree;
typedef struct _avl_tree* avl_tree_t;

avl_tree_t avl_create(key_cmp_func_t key_cmp_func, destructor_t val_destructor_func);
void       avl_destroy(avl_tree_t avl);

int        avl_add(avl_tree_t avl, void* key, void* data);
void       avl_remove(avl_tree_t avl, void* key, void** data);

int        avl_find(avl_tree_t avl, void* key, void** data);

#ifdef __cplusplus
}
#endif 

#endif /* __AVLTREE_H__ */
