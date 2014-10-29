#ifndef CORE_LOG_H
#define CORE_LOG_H

#include <stdio.h>

#define LOG_DEBUG   "\x1b[34;1m" "dbg" "\x1b[0m"
#define LOG_INFO    "\x1b[32;1m" "inf" "\x1b[0m"
#define LOG_WARNING "\x1b[33;1m" "wrn" "\x1b[0m"
#define LOG_ERROR   "\x1b[31;1m" "err" "\x1b[0m"

#define log_print(level,msg,...) \
  fprintf (stderr, " " level " ~ " msg "\n", ##__VA_ARGS__)

#define debug(...)      log_print (LOG_DEBUG, __VA_ARGS__)
#define info(...)       log_print (LOG_INFO, __VA_ARGS__)
#define warning(...)    log_print (LOG_WARNING, __VA_ARGS__)
#define error(...)      log_print (LOG_ERROR, __VA_ARGS__)

#define fatal(...) \
  do { \
    error (__VA_ARGS__); exit (EXIT_FAILURE); \
  } while (0)

#define progress(i,t,msg,...) \
  info (msg " => %.2f %%", ##__VA_ARGS__, ((float) (i) / (float) (t)) * 100.0f)

#endif
