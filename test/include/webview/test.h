#ifndef TEST_H
#define TEST_H

#include <cstdlib>
#include <stdio.h>

#define REQUIRE(condition)                                                     \
  if (!(condition)) {                                                          \
    printf("############################################################\n"    \
           "Assertion failed: %s\n"                                            \
           "Line: %u\n"                                                        \
           "File: %s\n"                                                        \
           "############################################################\n",   \
           #condition, __LINE__, __FILE__);                                    \
    exit(1);                                                                   \
  }

#endif // TEST_H
