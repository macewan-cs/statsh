#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "cpu_stats.h"

#define PROC_STAT_PATH "/proc/stat"
#define BUF_SIZE (1024 * 16)
#define FORMATTED_BUF_SIZE (1024 * 1)

typedef struct {
  long user;
  long nice;
  long system;
  long idle;

  long total;
} proc_stat_data;

static int proc_stat_fd = -1;
static proc_stat_data past, latest;

void initialize_cpu_state()
{
  proc_stat_fd = open(PROC_STAT_PATH, O_RDONLY);
  if (proc_stat_fd == -1)
    {
      perror("open (" PROC_STAT_PATH ")");
      exit(EXIT_FAILURE);
    }
}

void cleanup_cpu_state()
{
  int status = close(proc_stat_fd);
  if (status == -1)
    {
      perror("close (" PROC_STAT_PATH ")");
      // Continue to so that other things might be cleaned up.
    }
}

void refresh_cpu_state()
{
  past = latest;

  off_t offset = lseek(proc_stat_fd, 0, SEEK_SET);
  if (offset == -1)
    {
      perror("lseek (" PROC_STAT_PATH ")");
      exit(EXIT_FAILURE);
    }

  char buf[BUF_SIZE];
  ssize_t bytes = read(proc_stat_fd, buf, BUF_SIZE);
  if (bytes == -1)
    {
      perror("read (" PROC_STAT_PATH ")");
      exit(EXIT_FAILURE);
    }

  sscanf(buf, "%*s %ld %ld %ld %ld",
	 &latest.user, &latest.nice, &latest.system, &latest.idle);
}

cpu_stats get_cpu_stats()
{
  long total_diff = ((latest.user + latest.nice + latest.system + latest.idle) -
		     (past.user + past.nice + past.system + past.idle));

  cpu_stats return_val = (cpu_stats){
    .user = (float)(latest.user - past.user) / total_diff,
    .nice = (float)(latest.nice - past.nice) / total_diff,
    .system = (float)(latest.system - past.system) / total_diff,
    .idle = (float)(latest.idle - past.idle) / total_diff,
  };

  return return_val;
}

char *format_cpu_stats(cpu_stats *stats, const char *sep)
{
  static char buf[FORMATTED_BUF_SIZE];

  if (stats != NULL)
    {
      snprintf(buf, FORMATTED_BUF_SIZE, "%f%s%f%s%f%s%f",
	       stats->user, sep,
	       stats->nice, sep,
	       stats->system, sep,
	       stats->idle);
    }
  else
    {
      snprintf(buf, FORMATTED_BUF_SIZE, "%s%s%s%s%s%s%s",
	       "cpu.user.%", sep,
	       "cpu.nice.%", sep,
	       "cpu.sys.%", sep,
	       "cpu.idle.%");
    }

  return buf;
}
