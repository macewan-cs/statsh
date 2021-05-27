#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "disk_stats.h"

#define PROC_DISKSTATS_PATH "/proc/diskstats"
#define BUF_SIZE (1024 * 16)
#define FORMATTED_BUF_SIZE (1024 * 1)

/* Reference:
   https://www.kernel.org/doc/Documentation/ABI/testing/procfs-diskstats
   https://www.kernel.org/doc/Documentation/iostats.txt
*/
typedef struct {
  unsigned long reads_completed;
  unsigned long reads_merged;
  unsigned long sectors_read;
  unsigned long writes_completed;
  unsigned long writes_merged;
  unsigned long sectors_written;
  unsigned long in_progress;
  struct timeval ts;
} disk_stats_internal;

static char *dev_name = "dm-0";
static int proc_diskstats_fd = -1;
static disk_stats_internal past, latest;

void initialize_disk_state ()
{
  proc_diskstats_fd = open (PROC_DISKSTATS_PATH, O_RDONLY);
  if (proc_diskstats_fd == -1)
    {
      perror ("open (" PROC_DISKSTATS_PATH ")");
      exit (EXIT_FAILURE);
    }
}

void cleanup_disk_state ()
{
  int status = close (proc_diskstats_fd);
  if (status == -1)
    {
      perror ("close (" PROC_DISKSTATS_PATH ")");
      // Continue to so that other things might be cleaned up.
    }
}

static char *read_proc_diskstats ()
{
  static char buf[BUF_SIZE];

  off_t offset = lseek (proc_diskstats_fd, 0, SEEK_SET);
  if (offset == -1)
    {
      perror ("lseek (" PROC_DISKSTATS_PATH ")");
      exit (EXIT_FAILURE);
    }

  ssize_t bytes = read (proc_diskstats_fd, buf, BUF_SIZE);
  if (bytes == -1)
    {
      perror ("read (" PROC_DISKSTATS_PATH ")");
      exit (EXIT_FAILURE);
    }

  return buf;
}

void refresh_disk_state ()
{
  char *diskstats_data = read_proc_diskstats ();
}

disk_stats get_disk_stats()
{
  disk_stats return_val = (disk_stats){
    0
  };

  return return_val;
}

char *format_disk_stats(disk_stats *stats, const char *sep)
{
  static char buf[FORMATTED_BUF_SIZE] = "";

  return buf;
}
