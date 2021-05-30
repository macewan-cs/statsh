#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
  char *program_name;
  float delay_seconds;
  bool quiet;
  bool timestamp;
} config;

config create_config(char *program_name);

#endif /* CONFIG_H */
