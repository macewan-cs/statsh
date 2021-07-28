#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
  char *program_name;
  char *disk_device;
  float delay_seconds;
  bool quiet;
  bool timestamp;
} config;

config create_default_config (char *program_name);

char const *is_valid_config (config cfg);

#endif /* CONFIG_H */
