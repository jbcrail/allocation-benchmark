#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
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

int compare_samples(const void *a, const void *b)
{
  const uint64_t *arg1 = a;
  const uint64_t *arg2 = b;

  return *arg1 - *arg2;
}

#define ELAPSED(A, B) (1000000000lu * (uint64_t)(B.tv_sec - A.tv_sec) + B.tv_nsec - A.tv_nsec)

#define BENCH_SETUP() struct timespec tps, tpe;

#define BENCH_START(FD, NUM_TESTS) uint32_t ntests = NUM_TESTS; \
  uint64_t *samples = (uint64_t *)malloc(sizeof(uint64_t) * ntests); \
  for (int i = 0; i < NUM_TESTS; i++) { \
  lseek(FD, 0, SEEK_SET); \
  get_monotonic_clock(&tps)

#define BENCH_END(S, PTR) get_monotonic_clock(&tpe); \
  samples[i] = ELAPSED(tps, tpe); \
  free(PTR); \
  } \
  qsort(samples, ntests, sizeof(uint64_t), compare_samples); \
  printf("%-21s ", S); \
  printf("%" PRIu64 " ", samples[0]); \
  printf("%" PRIu64 " ", (samples[ntests/4]+samples[ntests/4+1])/2); \
  printf("%" PRIu64 " ", (samples[ntests/2-1]+samples[ntests/2])/2); \
  printf("%" PRIu64 " ", (samples[ntests-ntests/4-1]+samples[ntests-ntests/4])/2); \
  printf("%" PRIu64 "\n", samples[ntests-1]); \
  free(samples);

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

  // Round number of tests to nearest multiple of four, thus,
  // simplifying the quartile calculations
  uint32_t num_tests = (uint32_t)(ceil(atoi(argv[2]) / 4) * 4);
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
