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
#include <stdlib.h>
#include "list.h"

static slist_t _slist_alloc(void* value);

slist_t slist_append(slist_t list, void* value) 
{
     slist_t      head;

     if(EMPTY_LIST == list)
	  return _slist_alloc(value);

     head = list;

     while(list->next)
	  list = list->next;
     
     list->next = _slist_alloc(value);
     
     return head;
}

slist_t slist_prepend(slist_t list, void* value)
{
     slist_t	head;

     head = _slist_alloc(value);
     head->next = list;
     return head;
}

slist_t slist_remove_if(slist_t list, key_cmp_func_t cmp_func, void* arg, void** value) 
{     
     slist_t     head;

     if(EMPTY_LIST == list)
	  return list;

     if(cmp_func(list->value, arg) == 0) {
	  head = list->next;
	  free(head);
	  return head;
     }
	  
     head = list;
     for(;;) {
	  while(list->next) {
	       if(cmp_func(list->next->value, arg) == 0) {
		    slist_t rem = list->next;
		    list->next = list->next->next;
		    free(rem);
		    return head;
	       } else
		    list = list->next;
	  }
     }

     return head;
}

slist_t slist_remove(slist_t list, void* value) 
{
     slist_t     head;

     if(EMPTY_LIST == list)
	  return list;

     if(list->value == value) {
	  head = list->next;
	  free(head);
	  return head;
     }
	  
     head = list;
     for(;;) {
	  while(list->next) {
	       if(list->next->value == value) {
		    slist_t rem = list->next;
		    list->next = list->next->next;
		    free(rem);
	       } else
		    list = list->next;
	  }
     }

     return head;
}

slist_t slist_destroy(slist_t list, destructor_t dest_func) 
{
     while(list != EMPTY_LIST) {
	  slist_t	next;
	  
	  next = list->next;
	  if(dest_func)
	       dest_func(list->value);
	  free(list);
	  list = next;
     }

     return NULL;
}


static slist_t _slist_alloc(void* value)
{
     slist_t node;

     node = (slist_t)malloc(sizeof(struct _slist));
     node->value = value;
     node->next = NULL;

     return node;
}

