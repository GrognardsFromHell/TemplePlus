
#pragma once

typedef int BOOL;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

// Forward declaration for primary PyObject type
struct _object;
typedef _object PyObject;
