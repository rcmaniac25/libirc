#ifndef __DEFS_H__
#define __DEFS_H__

#ifndef FALSE
#define FALSE    0
#endif

#ifndef TRUE
#define TRUE     !FALSE
#endif

#define KEY(x)			((void*)x)
#define VALUE(x)		((void*)x)
#define VALUEREF(x)		((void**)&x)

#ifdef _WIN32
#define vsnprintf		_vsnprintf
#define snprintf		_snprintf
#define strcasecmp		_strcmpi
#else
#define SOCKET			int
#define INVALID_SOCKET	-1
#endif /* _WIN32 */

#endif /* __DEFS_H__ */
