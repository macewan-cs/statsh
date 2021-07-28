#define _DEFAULT_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "disk_stats.h"
#include "config.h"

// Sector size is 512 B inside kernel even for 4 KB disks.
//
// Reference:
// https://kernelnewbies.kernelnewbies.narkive.com/mWzSfGUI/why-is-sector-size-512-inside-kernel
#define SECTOR_SIZE 512

#define PROC_DISKSTATS_PATH "/proc/diskstats"
#define BUF_SIZE (1024 * 16)
#define FORMATTED_BUF_SIZE (1024 * 1)
#define MAX_DEV_NAME 16
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

static char dev_name[MAX_DEV_NAME + 1];
static int proc_diskstats_fd = -1;
static disk_stats_internal past, latest;

void initialize_disk_state (config *cfg)
{
  strncpy(dev_name, cfg->disk_device, MAX_DEV_NAME);
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

  char *line = strsep (&diskstats_data, "\n");
  while (line && *line != '\0')
    {
      char temp_dev_name[BUF_SIZE];
      disk_stats_internal temp_stats;

      int count = sscanf (line,
          // mj mn dv rc  rm  sr  tsr wc  wm  sw  tsw ioc
                          "%*d %*d %s %lu %lu %lu %*u %lu %lu %lu %*u %lu",
                          temp_dev_name,
                          &temp_stats.reads_completed, &temp_stats.reads_merged,
                          &temp_stats.sectors_read,
                          &temp_stats.writes_completed, &temp_stats.writes_merged,
                          &temp_stats.sectors_written,
                          &temp_stats.in_progress);
      if (count != 8)
        {
          fprintf (stderr, "cannot parse diskstats line: %s\n", line);
          continue;
        }

      if (strcmp (dev_name, temp_dev_name) == 0)
        {
          past = latest;
          latest = temp_stats;
          gettimeofday (&latest.ts, NULL);
          return;
        }

      line = strsep (&diskstats_data, "\n");
    }
}

disk_stats get_disk_stats ()
{
  long ms_lapsed = (latest.ts.tv_sec - past.ts.tv_sec) * 1000
                   + (latest.ts.tv_usec - past.ts.tv_usec) / 1000;

  disk_stats stats = (disk_stats) {
      .read_Bps = (float) (latest.sectors_read - past.sectors_read) * 512.0 * 1000.0 / ms_lapsed,
      .read_ops = (float) (latest.reads_completed - past.reads_completed) * 1000.0 / ms_lapsed,
      .write_Bps = (float) (latest.sectors_written - past.sectors_written) * 512.0 * 1000.0 / ms_lapsed,
      .write_ops = (float) (latest.writes_completed - past.writes_completed) * 1000.0 / ms_lapsed,
      .in_progress = latest.in_progress,
  };

  return stats;
}

char *format_disk_stats (disk_stats *stats, const char *sep)
{
  static char buf[FORMATTED_BUF_SIZE] = "";

  if (stats == NULL)
    {
      snprintf (buf, FORMATTED_BUF_SIZE, "%s%s%s%s%s%s%s%s%s",
                "disk.read.Bps", sep,
                "disk.read.ops", sep,
                "disk.write.Bps", sep,
                "disk.write.ops", sep,
                "disk.in-progress.n");
    }
  else
    {
      snprintf (buf, FORMATTED_BUF_SIZE, "%.2f%s%.2f%s%.2f%s%.2f%s%lu",
                stats->read_Bps, sep,
                stats->read_ops, sep,
                stats->write_Bps, sep,
                stats->write_ops, sep,
                stats->in_progress);
    }

  return buf;
}

