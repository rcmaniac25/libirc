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
#ifndef __TRACE_H__
#define __TRACE_H__

#ifdef __cplusplus
extern "C" {
#endif 

/* Debugging aids... */
void trace_start(int fd);
void trace_end();
void trace(const char* fmt, ...);

#ifdef ENABLE_TRACE
#define TRACE trace
#else
static void TRACE(const char* fmt, ...) { /* these are no-ops. just fooling gcc */ void* p = TRACE; p = NULL; }
#endif 

#ifdef __cplusplus
}
#endif 

#endif /* __TRACE_H__ */
