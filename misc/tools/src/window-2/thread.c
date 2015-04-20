#include <pthread.h>

pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

int window_lock_init(void)
{
	return pthread_mutex_init(&g_lock, NULL);
}
int window_lock(void)
{
	return pthread_mutex_lock(&g_lock);
}

int window_unlock(void)
{
	return pthread_mutex_unlock(&g_lock);
}

int window_trylock(void)
{
	return pthread_mutex_trylock(&g_lock);
}

int window_self(void)
{
	return pthread_self();
}
