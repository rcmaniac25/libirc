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
#ifndef __LIST_H__
#define __LIST_H__

#include "funcptrs.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _slist {
    void*          value;
    struct _slist* next;
}* slist_t;

slist_t slist_append(slist_t list, void* value);
slist_t slist_prepend(slist_t list, void* value);
slist_t slist_remove(slist_t list, void* value);
slist_t slist_destroy(slist_t list, destructor_t dest_func);
slist_t slist_remove_if(slist_t list, key_cmp_func_t cmp_func, void* arg, void** value);

#define slist_value(list)  (list)->value
#define slist_next(list)   (list)->next

#define EMPTY_LIST NULL

#ifdef __cplusplus
}
#endif 

#endif /* __LIST_H__ */
