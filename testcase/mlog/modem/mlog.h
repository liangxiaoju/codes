#ifndef __MLOG_H__
#define __MLOG_H__

/*
 * Print modem log to AP.
 *
 * @return
 * >=0: return the number of characters printed (excluding the null byte used to end output to strings).
 * < 0: on error.
 */
extern int mlog(const char *fmt, ...);

#endif
