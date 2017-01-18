#include "../util/range_tree.h"
#include <stdio.h>
#include "test_util.h"
#include "time_sample.h"
#include "../index.h"

int testRangeTree() {
  RangeTree *t = NewRangeTree();
  ASSERT(t != NULL);

  srand(1337);
  for (int i = 0; i < 50000; i++) {

    RangeTree_Add(t, i + 1, (double)(1 + rand() % 5000));
  }
  ASSERT_EQUAL_INT(t->numRanges, 16);
  ASSERT_EQUAL_INT(t->numEntries, 50000);

  struct {
    double min;
    double max;
  } rngs[] = {{0, 100}, {10, 1000}, {2500, 3500}, {0, 5000}, {4999, 4999}, {0, 0}};

  for (int r = 0; rngs[r].min || rngs[r].max; r++) {

    Vector *v = RangeTree_Find(t, rngs[r].min, rngs[r].max);
    ASSERT(Vector_Size(v) > 0);
    // printf("Got %d ranges for %f..%f...\n", Vector_Size(v), rngs[r].min, rngs[r].max);
    for (int i = 0; i < Vector_Size(v); i++) {
      NumericRange *l;
      Vector_Get(v, i, &l);
      ASSERT(l);
      // printf("%f...%f\n", l->minVal, l->maxVal);
      ASSERT(!(l->minVal > rngs[r].max));
      ASSERT(!(l->maxVal < rngs[r].min));
    }
    Vector_Free(v);
  }
  RangeTree_Free(t);
  return 0;
}

int testRangeIterator() {
  RangeTree *t = NewRangeTree();
  ASSERT(t != NULL);

  srand(1337);
  int N = 300;
  for (int i = 0; i < N; i++) {

    RangeTree_Add(t, i + 1, (double)(1 + rand() % 1000));
  }
  ASSERT_EQUAL_INT(t->numRanges, 1);
  ASSERT_EQUAL_INT(t->numEntries, N);

  IndexIterator *it = NewNumericRangeIterator(&t->root->range);
  ASSERT(it->HasNext(it->ctx));
  ASSERT(it->Len(it->ctx) == N);
  int count = 0;

  while (it->HasNext(it->ctx)) {
    IndexResult res = NewIndexResult();
    int rc = it->Read(it->ctx, &res);
    ASSERT_EQUAL_INT(INDEXREAD_OK, rc);
    ASSERT(res.docId > 0);
    ASSERT(res.flags = 0xff);
    ASSERT(res.numRecords == 1)
    count++;
  }
  ASSERT_EQUAL_INT(N, count);
  return 0;
}

int benchmarkRangeTree() {
  RangeTree *t = NewRangeTree();
  int count = 1;
  for (int i = 0; i < 1000000; i++) {

    count += RangeTree_Add(t, i, (double)(rand() % 100000));
  }
  printf("created %d range leaves\n", count);
  Vector *v;
  TIME_SAMPLE_RUN_LOOP(10000, {
    v = RangeTree_Find(t, 10000, 20000);
    // printf("%d\n", v->top);
    Vector_Free(v);
  });

  RangeTree_Free(t);
  return 0;
}

int main(int argc, char **argv) {
  TESTFUNC(testRangeTree);
  TESTFUNC(testRangeIterator);
  // benchmarkRangeTree();
  return 0;
}
