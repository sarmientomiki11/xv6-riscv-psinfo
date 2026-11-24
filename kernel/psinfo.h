#ifndef XV6_PSINFO_H
#define XV6_PSINFO_H

#define PS_NAME_LEN 16
enum ps_state {
  PS_SLEEPING = 1,
  PS_RUNNABLE,
  PS_RUNNING,
  PS_ZOMBIE
};

struct psinfo {
  int pid;
  int ppid;
  int state;
  uint32 reserved;
  uint64 vbytes;
  char name[PS_NAME_LEN];
};

typedef char psinfo_size_check[sizeof(struct psinfo) == 40 ? 1 : -1];

#endif
