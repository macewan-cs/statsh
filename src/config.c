#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"

config create_default_config (char *program_name)
{
  return (config){
    .delay_seconds = 0.1f,
    .program_name = basename (program_name),
    .quiet = false,
    .timestamp = false,
    .disk_device = "dm-0"
  };
}

char const *is_valid_config (config cfg)
{
  if (cfg.delay_seconds < 0.002)
    {
      return "delay is less than minimum (0.002 s)";
    }

  return NULL;
}
