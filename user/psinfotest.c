#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "kernel/psinfo.h"
#include "user/user.h"

static struct psinfo rows[NPROC];
static int failures;

static int
find_pid(int pid)
{
  int n = getpsinfo(rows, NPROC);
  if (n < 0)
    return -2;
  for (int i = 0; i < n; i++)
    if (rows[i].pid == pid)
      return i;
  return -1;
}

static void
check(const char *id, int ok)
{
  printf("%s %s\n", id, ok ? "PASS" : "FAIL");
  if (!ok)
    failures++;
}

static int
valid(void)
{
  int n = getpsinfo(rows, NPROC);
  int own = 0;
  if (n <= 0 || n > NPROC)
    return 0;
  for (int i = 0; i < n; i++) {
    struct psinfo *r = &rows[i];
    if (r->pid <= 0 || r->ppid < 0 || r->reserved ||
        r->state < PS_SLEEPING || r->state > PS_ZOMBIE ||
        r->name[PS_NAME_LEN - 1] != 0)
      return 0;
    for (int j = 0; j < i; j++)
      if (rows[j].pid == r->pid)
        return 0;
    if (r->pid == getpid())
      own = r->state == PS_RUNNING &&
            strcmp(r->name, "psinfotest") == 0 && r->vbytes > 0;
  }
  return own;
}

static int
bounds(void)
{
  struct psinfo one[2];
  memset(one, 0, sizeof(one));
  one[1].pid = 123456;
  int n = getpsinfo(one, 1);
  return n == 1 && one[0].pid > 0 && one[1].pid == 123456 &&
         getpsinfo(one, 0) == -1 &&
         getpsinfo(one, -1) == -1 &&
         getpsinfo(one, NPROC + 1) == -1;
}

static int
bad_address(void)
{
  return getpsinfo(0, 1) == -1 &&
         getpsinfo((struct psinfo *)~0ULL, 1) == -1 &&
         getpsinfo((struct psinfo *)(sbrk(0) - 1), 1) == -1 &&
         getpsinfo((struct psinfo *)(uint64)&bad_address, 1) == -1 &&
         valid();
}

static void
lifecycle(void)
{
  int fd[2], status = -1;
  if (pipe(fd) < 0) {
    check("T04", 0);
    check("T05", 0);
    return;
  }

  int child = fork();
  if (child == 0) {
    char token;
    close(fd[1]);
    int n = read(fd[0], &token, 1);
    close(fd[0]);
    exit(n == 0 ? 0 : 1);
  }

  close(fd[0]);
  int sleeping = 0, zombie = 0;
  if (child > 0) {
    for (int k = 0; k < 20; k++) {
      int i = find_pid(child);
      if (i >= 0 && rows[i].state == PS_SLEEPING &&
          rows[i].ppid == getpid()) {
        sleeping = 1;
        printf("T04 hijo=%d padre=%d estado=SLEEPING\n",
               child, rows[i].ppid);
        break;
      }
      pause(1);
    }
  }
  check("T04", sleeping);
  close(fd[1]);

  if (child > 0) {
    for (int k = 0; k < 20; k++) {
      int i = find_pid(child);
      if (i >= 0 && rows[i].state == PS_ZOMBIE &&
          rows[i].ppid == getpid()) {
        zombie = 1;
        break;
      }
      pause(1);
    }
  }
  int reaped = child > 0 && wait(&status) == child && status == 0;
  int absent = reaped && find_pid(child) == -1;
  printf("T05 hijo=%d zombie=%d ausente=%d\n", child, zombie, absent);
  check("T05", zombie && absent);
}

static void
size_change(void)
{
  int pid = getpid();
  int i = find_pid(pid);
  if (i < 0) {
    check("T06", 0);
    return;
  }
  uint64 before = rows[i].vbytes;
  char *space = sbrklazy(2 * PGSIZE);
  if (space == SBRK_ERROR) {
    check("T06", 0);
    return;
  }
  i = find_pid(pid);
  uint64 grown = i < 0 ? 0 : rows[i].vbytes;
  int shrunk = sbrk(-2 * PGSIZE) != SBRK_ERROR;
  i = find_pid(pid);
  uint64 after = i < 0 ? 0 : rows[i].vbytes;
  printf("T06 antes=%lu ampliado=%lu recuperado=%lu\n",
         before, grown, after);
  check("T06", grown == before + 2 * PGSIZE && shrunk &&
               after == before);
}

static int
stress(void)
{
  enum { CHILDREN = 3, ROUNDS = 100 };
  int created = 0, completed = 0, own = valid();
  for (int i = 0; i < CHILDREN; i++) {
    int child = fork();
    if (child == 0) {
      for (int k = 0; k < ROUNDS; k++) {
        char *space = sbrk(PGSIZE);
        if (space == SBRK_ERROR || !valid() ||
            sbrk(-PGSIZE) == SBRK_ERROR)
          exit(1);
      }
      exit(0);
    }
    if (child > 0)
      created++;
  }
  for (int k = 0; k < ROUNDS; k++)
    if (!valid())
      own = 0;
  for (int i = 0; i < created; i++) {
    int status = -1;
    if (wait(&status) > 0 && status == 0)
      completed++;
  }
  printf("T07 creados=%d finalizados=%d rondas=%d\n",
         created, completed, ROUNDS);
  return own && created == CHILDREN && completed == CHILDREN;
}

static int
cross_page(void)
{
  char *raw = sbrk(3 * PGSIZE);
  if (raw == SBRK_ERROR)
    return 0;
  uint64 boundary = PGROUNDUP((uint64)raw + 1);
  struct psinfo *dst = (struct psinfo *)(boundary - 8);
  int n = getpsinfo(dst, 1);
  int ok = n == 1 && dst->pid > 0 &&
           dst->name[PS_NAME_LEN - 1] == 0;
  return sbrk(-3 * PGSIZE) != SBRK_ERROR && ok;
}

int
main(void)
{
  check("T01", valid());
  check("T02", bounds());
  check("T03", bad_address());
  lifecycle();
  size_change();
  check("T07", stress());
  check("T08", cross_page());
  printf("TOTAL %d/8 PASS\n", 8 - failures);
  exit(failures != 0);
}
