# statsh: Statistics Shell

> This application writes periodic system resource usage statistics to monitor a system's usage over time.
> It has been designed to serve as a user's login shell.

## Table of Contents

- [General Information](#General-Information)
- [Features](#Features)
- [Technologies Used](#Technologies-Used)
- [Installation](#Installation)
- [Project Status](#Project-Status)
- [Contact](#Contact)
- [License](#License)

## General Information

This application can help a user collect system resource usage statistics from a system while providing limited access to a system.
At this time, the collected data include

- general
  - `timestamp` (optional): the timestamp (ms since epoch)
  - `hostname`: the system's hostname
- CPU
  - `user`: the percent of time running normal (un-niced) user processes
  - `nice`: the percent of time running niced user processes
  - `system`: the percent of time running in kernel mode
  - `idle`: the percent of time not running processes
- disk
  - `read Bps`: the number of sectors read multiplied by 512 bytes
  - `read ops`: the number of reads completed
  - `write Bps`: the number of sectors written multiplied by 512 bytes
  - `write ops`: the number of writes completed
  - `in progress`: the number of I/Os currently in progress
- memory
  - `total`: the total usable RAM, i.e., physical RAM minus a few reserved bits and the kernel binary code.
  - `used`: the used memory (total - free - buffers - cache)
  - `free`: the free memory (LowFree + HighFree)
  - `shared`: the memory consumed in tmpfs filesystems
  - `buff/cache`: the buffers + cache + the in-kernel data structures cache that might be reclaimed (SReclaimable)
  - `available`: an estimate of the memory available for starting new applications without swapping
- network
  - `rx Bps`: the number of bytes received per second
  - `rx pps`: the number of packets received per second
  - `tx Bps`: the number of bytes transmitted per second
  - `tx pps`: the number of packets transmitted per second
  - `total Bps`: the number of bytes received and transmitted per second
  - `total pps`: the number of packets received and transmitted per second

It produces these fields as comma-separated values (CSV).
The user can configure the rate of samples using a command-line argument.

## Features

- The sample rate can be configured using a command line argument.
  The rate is a floating-point number supporting values as low as 0.002 s.
- The command-line arguments can be provided using the `TERM` environment variable.

## Technologies Used

- Linux and its `/proc` filesystem
- C

## Installation

The recommended setup:

1. Run the following commands to compile and install
```bash
cd build
cmake ..
make
# copy the binary executable and manpage into the appropriate directories 
sudo make install
```
2. Create a user for running the tool and configure the user's shell as `/usr/bin/statsh`.
3. (*Optional*) Setup `.ssh/authorized_keys` for this new account to allow users to connect.
4. Run the command.
   1. On a single system, use a command line like
      ```bash
      TERM="-d 0.5" ssh *user*@*host*
      ```
   2. On multiple  systems, use a command line like
      ```bash
	  cat hosts | \
	    TERM="-d 0.5" parallel --env TERM --linebuffer -j $(cat hosts | wc -l) ssh -tt -o StrictHostKeyChecking=no *user*@{} --
	  ```

## Project Status

This project is under active development.

## Contact

Created by Peng Li and Nicholas M. Boers at [MacEwan University](https://www.macewan.ca/ComputerScience).

## License

This project is licensed under the GNU General Public License (Version 3).
