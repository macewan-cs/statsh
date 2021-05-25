#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "mem_stats.h"

#define PROC_MEMINFO_PATH "/proc/meminfo"
#define BUF_SIZE (1024 * 16)
#define FORMATTED_BUF_SIZE (1024 * 1)

static int proc_meminfo_fd = -1;
static mem_stats latest;

void initialize_mem_state()
{
  proc_meminfo_fd = open(PROC_MEMINFO_PATH, O_RDONLY);
  if (proc_meminfo_fd == -1)
    {
      perror("open (" PROC_MEMINFO_PATH ")");
      exit(EXIT_FAILURE);
    }
}

void cleanup_mem_state()
{
  int status = close(proc_meminfo_fd);
  if (status == -1)
    {
      perror("close (" PROC_MEMINFO_PATH ")");
      // Continue to so that other things might be cleaned up.
    }
}

static void parse_proc_meminfo(char buf[BUF_SIZE])
{
  struct _name_to_location {
    char *name;
    long *location;
  } name_to_location[] = {
    { "MemTotal:", &(latest.total) },
    { "MemFree:", &(latest.free) },
    { "MemAvailable:", &(latest.available) },
    { "Buffers:", &(latest.buffers) },
    { "Cached:", &(latest.cache) },
    { "Shmem:", &(latest.shared) },
    { "SReclaimable:", &(latest.s_reclaimable) },
  };

  char *buf_iter = buf;

  for (int i = 0; i < sizeof name_to_location / sizeof *name_to_location; i++)
    {
      char format_string[80];
      sprintf(format_string, "%s%%ld", name_to_location[i].name);
      buf_iter = strstr(buf_iter, name_to_location[i].name);
      sscanf(buf_iter, format_string, name_to_location[i].location);
    }

  latest.cache += latest.s_reclaimable;
  latest.used = latest.total - latest.free - latest.buffers - latest.cache;
}

void refresh_mem_state()
{
  off_t offset = lseek(proc_meminfo_fd, 0, SEEK_SET);
  if (offset == -1)
    {
      perror("lseek (" PROC_MEMINFO_PATH ")");
      exit(EXIT_FAILURE);
    }

  char buf[BUF_SIZE];
  ssize_t bytes = read(proc_meminfo_fd, buf, BUF_SIZE);
  if (bytes == -1)
    {
      perror("read (" PROC_MEMINFO_PATH ")");
      exit(EXIT_FAILURE);
    }

  parse_proc_meminfo(buf);
}

mem_stats get_mem_stats()
{
  return latest;
}

char *format_mem_stats(mem_stats *stats, const char *sep)
{
  static char buf[FORMATTED_BUF_SIZE];

  if (stats != NULL)
    {
      snprintf(buf, FORMATTED_BUF_SIZE, "%ld%s%ld%s%ld%s%ld%s%ld%s%ld",
	       stats->total, sep,
	       stats->used, sep,
	       stats->free, sep,
	       stats->shared, sep,
	       stats->buffers + stats->cache, sep,
	       stats->available);
    }
  else
    {
      snprintf(buf, FORMATTED_BUF_SIZE, "%s%s%s%s%s%s%s%s%s%s%s",
	       "total", sep,
	       "used", sep,
	       "free", sep,
	       "shared", sep,
	       "buff/cache", sep,
	       "available");
    }

  return buf;
}
