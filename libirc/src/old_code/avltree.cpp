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

#ifdef USE_AVLTREE
#include <stdlib.h>
#include <assert.h>
#include "defs.h"
#include "avltree.h"

typedef struct _avl_node {
     void*               key;
     void*               value;
     int                 balance;

     struct _avl_node*   left;
     struct _avl_node*   right;
} *avl_node_t;

#define BALANCE(node)		((node) ? (node)->balance : 0)
#define ABS(x)			((x) < 0 ? -(x) : (x))

static void _avl_destroy(avl_node_t node, destructor_t val_destructor_func);
static avl_node_t _avl_add(avl_node_t node, key_cmp_func_t key_cmp_func, void* key, void* data, int* success);
static avl_node_t _avl_remove(avl_node_t node, key_cmp_func_t key_cmp_func, destructor_t val_destructor_func, 
			      void* key, void** data);
static int _avl_find(avl_node_t node, key_cmp_func_t key_cmp_func, void* key, void** data);
static avl_node_t _avl_rebalance(avl_node_t node);
static avl_node_t _avl_rot_ll(avl_node_t node);
static avl_node_t _avl_rot_rr(avl_node_t node);
static avl_node_t _avl_rot_lr(avl_node_t node);
static avl_node_t _avl_rot_rl(avl_node_t node);

struct _avl_tree {
     avl_node_t          root;
     key_cmp_func_t     key_cmp_func;
     destructor_t        val_destructor_func;
};

avl_tree_t avl_create(key_cmp_func_t key_cmp_func, destructor_t val_destructor_func)
{
     avl_tree_t   avl;
     assert(key_cmp_func != NULL);

     avl = (avl_tree_t)malloc(sizeof(struct _avl_tree));
     avl->root = NULL;
     avl->key_cmp_func = key_cmp_func;
     avl->val_destructor_func = val_destructor_func;

     return avl;
}

void avl_destroy(avl_tree_t avl) 
{
     assert(avl != NULL);

     _avl_destroy(avl->root, avl->val_destructor_func);
     free(avl);
}

int avl_add(avl_tree_t avl, void* key, void* data) 
{
     int           success;
     avl_node_t    possibly_new_root;

     assert(avl != NULL);
     

     possibly_new_root = _avl_add(avl->root, avl->key_cmp_func, key, data, &success);

     if(success)
	  avl->root = possibly_new_root;

     return success;
}

void avl_remove(avl_tree_t avl, void* key, void** data) 
{
     assert(avl != NULL);
     
     avl->root = _avl_remove(avl->root, avl->key_cmp_func, avl->val_destructor_func, key, data);
}

int avl_find(avl_tree_t avl, void* key, void** data) 
{
     assert(avl != NULL);

     return _avl_find(avl->root, avl->key_cmp_func, key, data);
}


static void _avl_destroy(avl_node_t node, destructor_t val_destructor_func)
{
     if(NULL == node)
	  return;

     _avl_destroy(node->left, val_destructor_func);
     _avl_destroy(node->right, val_destructor_func);

     val_destructor_func(node->value);
     free(node);
}

static avl_node_t _avl_add(avl_node_t node, key_cmp_func_t key_cmp_func, void* key, void* data, int* success)
{
     int cmp;

     if(NULL == node) {
	  node = (avl_node_t)malloc(sizeof(struct _avl_node));
	  node->key = key;
	  node->value = data;
	  node->left = node->right = NULL;
	  node->balance = 0;
	  *success = TRUE;
	  return node;
     }

     cmp = key_cmp_func(key, node->key);
     if(0 == cmp) {
	  *success = FALSE;
     } else if(cmp > 0) {
	  node->right = _avl_add(node->right, key_cmp_func, key, data, success);
	  node->balance = BALANCE(node->left) - BALANCE(node->right);
     } else /* cmp < 0 */ {
	  node->left = _avl_add(node->left, key_cmp_func, key, data, success);
	  node->balance = BALANCE(node->left) - BALANCE(node->right);
     }
     
     if(*success) {
	  if(ABS(node->balance) > 1)
	       node = _avl_rebalance(node);
     }
     return node;
}

#define _AVL_FIND_SMALLEST(node, smallest_key)	       	\
do {							\
     while(node->left != NULL) node = node->left;	\
     smallest_key = node->key;		       		\
} while(0)

static avl_node_t _avl_remove(avl_node_t node, key_cmp_func_t key_cmp_func, destructor_t val_destructor_func,
			      void* key, void** data)
{
     int cmp;

     if(NULL == node)
	  return NULL;
     
     cmp = key_cmp_func(key, node->key);
     if(0 == cmp) {
	  if(node->right == NULL) {
	       avl_node_t   left = node->left;
	       if(data != NULL)
		    *data = node->value;
	       else if(NULL != val_destructor_func)
		    val_destructor_func(node->value);
	       free(node);
	       node = left;
	  } else {
	       void* smallest_key;
	       void* smallest_value;

	       _AVL_FIND_SMALLEST(node->right, smallest_key);
	       node->right = _avl_remove(node->right, key_cmp_func, NULL, smallest_key, &smallest_value);

	       if(data != NULL)
		    *data = node->value;
	       else if(NULL != val_destructor_func)
		    val_destructor_func(node->value);

	       node->value = smallest_value;
	       node->key = smallest_key;
	  }
     } else if(cmp > 0) {
	  node->right = _avl_remove(node->right, key_cmp_func, val_destructor_func, key, data);
     } else /* cmp < 0 */ {
	  node->left = _avl_remove(node->left, key_cmp_func, val_destructor_func, key, data);
     }
     
     node->balance = BALANCE(node->left) - BALANCE(node->right);
     if(ABS(node->balance) > 1)
	  node = _avl_rebalance(node);
     
     return node;
}

#undef _AVL_FIND_SMALLEST

static int _avl_find(avl_node_t node, key_cmp_func_t key_cmp_func, void* key, void** data)
{
     int cmp;

     if(node == NULL)
	  return FALSE;

     cmp = key_cmp_func(key, node->key);
     if(0 == cmp) {
	  if(NULL != data)
	       *data = node->value;
	  return TRUE;
     } else if(cmp > 0) {
	  return _avl_find(node->right, key_cmp_func, key, data);
     } else /* cmp < 0 */ {
	  return _avl_find(node->left, key_cmp_func, key, data);
     }
}

static avl_node_t _avl_rot_ll(avl_node_t node)
{
     avl_node_t b = node->left;
     node->left = b->right;
     node->balance = BALANCE(node->left) - BALANCE(node->right);
     b->balance = BALANCE(b->left) - BALANCE(b->right);
     return b;
}

static avl_node_t _avl_rot_rr(avl_node_t node)
{
     avl_node_t b = node->right;
     node->right = b->left;
     node->balance = BALANCE(node->left) - BALANCE(node->right);
     b->balance = BALANCE(b->left) - BALANCE(b->right);
     return b;
}

static avl_node_t _avl_rot_lr(avl_node_t node)
{
     node->left = _avl_rot_rr(node->left);
     return _avl_rot_ll(node);
}

static avl_node_t _avl_rot_rl(avl_node_t node)
{
     node->right = _avl_rot_ll(node->right);
     return _avl_rot_rr(node);
}

static avl_node_t _avl_rebalance(avl_node_t node)
{
     if(node->balance > 1) {
	  if(node->left->balance > 0)
	       node = _avl_rot_ll(node);
	  else
	       node = _avl_rot_lr(node);
     } else if(node->balance < -1) {
	  if(node->right->balance < 0)
	       node = _avl_rot_rr(node);
	  else
	       node = _avl_rot_rl(node);
     }

     return node;
}

#endif /* USE_AVLTREE */
