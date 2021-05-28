#ifndef DISK_STATS_H
#define DISK_STATS_H

typedef struct {
  float read_Bps;
  float read_ops;
  float write_Bps;
  float write_ops;
  unsigned long in_progress;
} disk_stats;

// initialize_disk_state initializes any internal state for recording disk stats.
void initialize_disk_state();

// refresh_disk_state refreshes the internal state for recording disk stats.
void refresh_disk_state();

// get_disk_stats returns disks stats computed from the internal state.
disk_stats get_disk_stats();

// format_disk_stats returns a pointer to a static NUL-terminated
// string for the formatted disk stats. If the disk_stats pointer is
// NULL, it formats a header line in the string.
char *format_disk_stats(disk_stats *stats, const char *sep);

// cleanup_disk_state cleans up the internal state for recording disk stats.
void cleanup_disk_state();

#endif /* DISK_STATS_H */
