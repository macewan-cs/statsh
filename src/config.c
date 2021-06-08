#include <libgen.h>
#include <stdbool.h>

#include "config.h"

config create_config(char *program_name)
{
  return (config){
    .delay_seconds = 0.1,
    .program_name = basename(program_name),
    .quiet = false,
    .timestamp = false,
  };
}
