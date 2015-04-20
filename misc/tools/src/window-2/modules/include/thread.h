#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

extern int window_lock(void);
extern int window_unlock(void);
extern int window_trylock(void);
extern int window_self(void);

#endif
