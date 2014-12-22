// Linux/OS X: gcc -g -std=gnu99 -Wall -Werror -o bench_initialize bench_initialize.c -lrt
//
// Generate test data using `dd if=/dev/urandom bs=$N count=1 >output` for $N bytes

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef __MACH__
#include <mach/mach_time.h>

static double _timebase = 0.0;
static uint64_t _timestart = 0;

void get_monotonic_clock(struct timespec *t)
{
  if (!_timestart) {
    mach_timebase_info_data_t tb = { 0 };
    mach_timebase_info(&tb);
    _timebase = tb.numer;
    _timebase /= tb.denom;
    _timestart = mach_absolute_time();
  }
  double diff = (mach_absolute_time() - _timestart) * _timebase;
  t->tv_sec = diff / 1000000000lu;
  t->tv_nsec = diff - (t->tv_sec * 1000000000lu);
}
#else
void get_monotonic_clock(struct timespec *t)
{
  clock_gettime(CLOCK_MONOTONIC, t);
}
#endif

#define ELAPSED(A, B) (1000000000lu * (uint64_t)(B.tv_sec - A.tv_sec) + B.tv_nsec - A.tv_nsec)

#define BENCH_SETUP() struct timespec tps, tpe; \
  uint64_t cumulative = 0

#define BENCH_START(FD, NUM_TESTS) uint32_t ntests = NUM_TESTS; \
  for (int i = 0; i < NUM_TESTS; i++) { \
  lseek(FD, 0, SEEK_SET); \
  get_monotonic_clock(&tps)

#define BENCH_END(S, PTR) get_monotonic_clock(&tpe); \
  cumulative += ELAPSED(tps, tpe); \
  free(PTR); \
  } \
  printf("%-21s %" PRIu64 " ns/op\n", S, cumulative / ntests)

void bench_no_initialization(int fd, uint32_t num_tests, size_t bytes, char *ptr)
{
  BENCH_SETUP();
  BENCH_START(fd, num_tests);

  ptr = malloc(bytes+1);
  assert(ptr != NULL);
  read(fd, ptr, bytes);
  ptr[bytes] = '\0';

  BENCH_END("no initialization:", ptr);
}

void bench_initialize_with_memset(int fd, uint32_t num_tests, size_t bytes, char *ptr)
{
  BENCH_SETUP();
  BENCH_START(fd, num_tests);

  ptr = malloc(bytes+1);
  assert(ptr != NULL);
  memset(ptr, 0, bytes+1);
  read(fd, ptr, bytes);

  BENCH_END("initialize w/ memset:", ptr);
}

void bench_initialize_with_calloc(int fd, uint32_t num_tests, size_t bytes, char *ptr)
{
  BENCH_SETUP();
  BENCH_START(fd, num_tests);

  ptr = calloc(bytes+1, 1);
  assert(ptr != NULL);
  read(fd, ptr, bytes);

  BENCH_END("initialize w/ calloc:", ptr);
}

int main(int argc, char *argv[])
{
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <benchmark> <# tests> <filename>\n\n", argv[0]);
    fprintf(stderr, "Available benchmarks:\n\n");
    fprintf(stderr, "\t0\tAll benchmarks are executed\n");
    fprintf(stderr, "\t1\tNo initialization\n");
    fprintf(stderr, "\t2\tInitialization with memset\n");
    fprintf(stderr, "\t3\tInitialization with calloc\n");
    exit(EXIT_FAILURE);
  }

  uint32_t num_tests = (uint32_t)atoi(argv[2]);
  int fd = open(argv[3], O_RDONLY);
  int bytes = lseek(fd, 0, SEEK_END) + 1;
  char *ptr = NULL;

  switch (atoi(argv[1])) {
    case 1:
      bench_no_initialization(fd, num_tests, bytes, ptr);
      break;

    case 2:
      bench_initialize_with_memset(fd, num_tests, bytes, ptr);
      break;

    case 3:
      bench_initialize_with_calloc(fd, num_tests, bytes, ptr);
      break;

    case 0:
    default:
      bench_no_initialization(fd, num_tests, bytes, ptr);
      bench_initialize_with_memset(fd, num_tests, bytes, ptr);
      bench_initialize_with_calloc(fd, num_tests, bytes, ptr);
      break;
  }

  close(fd);
  return 0;
}
