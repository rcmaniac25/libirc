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
#if defined(CALLBACK_COLLECTION_IS_AVLTREE) && defined(CALLBACK_COLLECTION_IS_HASHTABLE)
#	error Define either CALLBACK_COLLECTION_IS_HASHTABLE or CALLBACK_COLLECTION_IS_AVLTREE
#endif

#ifdef CALLBACK_COLLECTION_IS_AVLTREE
#	define USE_MAP_AVLTREE
#	define USE_AVLTREE
#elif  CALLBACK_COLLECTION_IS_HASHTABLE
#	define USE_MAP_HASHTABLE
#	define USE_HASHTABLE
#else
#	error Define either CALLBACK_COLLECTION_IS_HASHTABLE or CALLBACK_COLLECTION_IS_AVLTREE
#endif
