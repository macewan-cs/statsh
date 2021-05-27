#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/time.h>
#include "net_stats.h"

#define PROC_NET_DEV_PATH "/proc/net/dev"
#define BUF_SIZE (1024 * 16)
#define FORMATTED_BUF_SIZE (1024 * 1)

typedef struct {
    char dev_name[MAX_DEV_NAME + 1];
    long rx_bytes;
    long rx_packets;
    long tx_bytes;
    long tx_packets;
    struct timeval ts;
} net_stats_internal;

static char *dev_name = "enp68s0";
static int proc_net_dev_fd = -1;
static net_stats_internal past, latest;

void set_dev_dev (char *new_dev)
{
  dev_name = new_dev;
}

void initialize_net_state ()
{
  proc_net_dev_fd = open (PROC_NET_DEV_PATH, O_RDONLY);
  if (proc_net_dev_fd == -1)
    {
      perror ("open (" PROC_NET_DEV_PATH ")");
      exit (EXIT_FAILURE);
    }

}

void cleanup_net_state ()
{
  int status = close (proc_net_dev_fd);
  if (status == -1)
    {
      perror ("close (" PROC_NET_DEV_PATH ")");
      // Continue to so that other things might be cleaned up.
    }
}

static void parse_proc_net_dev (char *buf)
{
  net_stats_internal tmp;
  // Skip first 2 lines
  char *line = strchr (buf, '\n') + 1;
  line = strchr (line, '\n') + 1;
  while (line != NULL)
    {
      line++;
      sscanf (line, " %[^:]:%ld %ld %*d %*d %*d %*d %*d %*d %ld %ld %*s",
              tmp.dev_name,
              &tmp.rx_bytes,
              &tmp.rx_packets,
              &tmp.tx_bytes,
              &tmp.tx_bytes
      );
      if (strcmp (dev_name, tmp.dev_name) == 0)
        {
          past = latest;
          latest = tmp;
          gettimeofday (&latest.ts, NULL);
          return;
        }
      line = strchr (line, '\n');
    }
  fprintf (stderr, "Cannot find interface: %s\n", dev_name);
  exit (EXIT_FAILURE);
}

net_stats get_net_stats ()
{
  long ms_lapsed = (latest.ts.tv_sec - past.ts.tv_sec) * 1000
                   + (latest.ts.tv_usec - past.ts.tv_usec) / 1000;
  net_stats stats = {
      .rx_Bps = (latest.rx_bytes - past.rx_bytes) * 1000 / ms_lapsed,
      .rx_pps = (latest.rx_packets - past.rx_packets) * 1000 / ms_lapsed,
      .tx_Bps = (latest.tx_bytes - past.tx_bytes) * 1000 / ms_lapsed,
      .tx_pps = (latest.tx_packets - past.tx_packets) * 1000 / ms_lapsed,
  };
  return stats;
}

char *format_net_stats (net_stats *stats, const char *sep)
{
  static char buf[FORMATTED_BUF_SIZE];

  if (stats != NULL)
    {
      snprintf (buf, FORMATTED_BUF_SIZE, "%ld%s%ld%s%ld%s%ld%s%ld%s%ld",
                stats->rx_Bps, sep,
                stats->rx_pps, sep,
                stats->tx_Bps, sep,
                stats->rx_pps, sep,
                stats->rx_Bps + stats->tx_Bps, sep,
                stats->rx_pps + stats->tx_pps
      );
    }
  else
    {
      snprintf (buf, FORMATTED_BUF_SIZE, "%s%s%s%s%s%s%s%s%s%s%s",
                "rx Bps", sep,
                "rx pps", sep,
                "tx Bps", sep,
                "tx pps", sep,
                "total Bps", sep,
                "total pps"
      );
    }
  return buf;
}

void refresh_net_state ()
{
  off_t offset = lseek (proc_net_dev_fd, 0, SEEK_SET);
  if (offset == -1)
    {
      perror ("lseek (" PROC_NET_DEV_PATH ")");
      exit (EXIT_FAILURE);
    }

  char buf[BUF_SIZE];
  int bytes = read (proc_net_dev_fd, buf, BUF_SIZE);
  if (bytes == -1)
    {
      perror ("read (" PROC_NET_DEV_PATH ")");
      exit (EXIT_FAILURE);
    }
  parse_proc_net_dev (buf);
}
