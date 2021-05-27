#ifndef NET_STATS_H
#define NET_STATS_H

// max device name length is 16.
// ref: https://elixir.bootlin.com/linux/v5.10.39/source/include/uapi/linux/if.h#L33
#define MAX_DEV_NAME 16

typedef struct {
    long rx_Bps;
    long rx_pps;
    long tx_Bps;
    long tx_pps;
} net_stats;

void initialize_net_state ();
void cleanup_net_state ();
void refresh_net_state ();

net_stats get_net_stats ();
void set_net_dev (char *new_dev);
char *format_net_stats (net_stats *stats, const char *sep);
#endif //NET_STATS_H
