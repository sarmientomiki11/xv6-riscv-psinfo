#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/psinfo.h"
#include "user/user.h"

static struct psinfo rows[NPROC];

static const char *
state_name(int state)
{
  switch (state) {
  case PS_SLEEPING: return "SLEEPING";
  case PS_RUNNABLE: return "RUNNABLE";
  case PS_RUNNING:  return "RUNNING";
  case PS_ZOMBIE:   return "ZOMBIE";
  default:          return "UNKNOWN";
  }
}

int
main(int argc, char **argv)
{
  if (argc != 1) {
    fprintf(2, "uso: %s\n", argv[0]);
    exit(2);
  }

  int n = getpsinfo(rows, NPROC);
  if (n < 0) {
    fprintf(2, "psinfo: consulta fallida\n");
    exit(1);
  }

  printf("PID\tPPID\tESTADO\tVIRT_B\tNOMBRE\n");
  for (int i = 0; i < n; i++) {
    const struct psinfo *r = &rows[i];
    printf("%d\t%d\t%s\t%lu\t%s\n", r->pid, r->ppid,
           state_name(r->state), r->vbytes, r->name);
  }
  exit(0);
}
