#ifndef TEST_AGGREGATE_TEST_H_
#define TEST_AGGREGATE_TEST_H_

#include "aggregate.h"
#include "gtest/gtest.h"

using namespace ::dialog;

class AggregateTest : public testing::Test {
};

TEST_F(AggregateTest, SumTest) {
  aggregate_list agg(INT_TYPE, aggregate_id::D_SUM);
  ASSERT_TRUE(numeric(limits::int_zero) == agg.get(0));

  int sum[11];
  sum[0] = limits::int_zero;
  for (int i = 1; i <= 10; i++) {
    sum[i] = sum[i - 1] + i;
  }

  for (int i = 1; i <= 10; i++) {
    numeric value(i);
    agg.update(value, i * 2);
    for (int j = 0; j <= i; j++)
      ASSERT_TRUE(numeric(sum[j]) == agg.get(j * 2));
  }

  for (int j = 0; j <= 20; j++) {
    ASSERT_TRUE(numeric(sum[j / 2]) == agg.get(j));
  }
}

TEST_F(AggregateTest, MinTest) {
  aggregate_list agg(INT_TYPE, aggregate_id::D_MIN);
  ASSERT_TRUE(numeric(limits::int_max) == agg.get(0));

  int min[11];
  min[0] = limits::int_max;
  for (int i = 1; i <= 10; i++) {
    min[i] = std::min(min[i - 1], 10 - i);
  }

  for (int i = 1; i <= 10; i++) {
    numeric value(10 - i);
    agg.update(value, i * 2);
    for (int j = 0; j <= i; j++)
      ASSERT_TRUE(numeric(min[j]) == agg.get(j * 2));
  }

  for (int j = 0; j <= 20; j++) {
    ASSERT_TRUE(numeric(min[j / 2]) == agg.get(j));
  }
}

TEST_F(AggregateTest, MaxTest) {
  aggregate_list agg(INT_TYPE, aggregate_id::D_MAX);
  ASSERT_EQ(numeric(limits::int_min), agg.get(0));

  int max[11];
  max[0] = limits::int_min;
  for (int i = 1; i <= 10; i++) {
    max[i] = std::max(max[i - 1], i);
  }

  for (int i = 1; i <= 10; i++) {
    numeric value(i);
    agg.update(value, i * 2);
    for (int j = 0; j <= i; j++)
      ASSERT_TRUE(numeric(max[j]) == agg.get(j * 2));
  }

  for (int j = 0; j <= 20; j++) {
    ASSERT_TRUE(numeric(max[j / 2]) == agg.get(j));
  }
}

TEST_F(AggregateTest, CountTest) {
  aggregate_list agg(INT_TYPE, aggregate_id::D_CNT);
  ASSERT_TRUE(numeric(limits::int_zero) == agg.get(0));

  int count[11];
  count[0] = limits::int_zero;
  for (int i = 1; i <= 10; i++) {
    count[i] = i;
  }

  for (int i = 1; i <= 10; i++) {
    agg.update(numeric(1), i * 2);
    for (int j = 0; j <= i; j++)
      ASSERT_TRUE(numeric(count[j]) == agg.get(j * 2));
  }

  for (int j = 0; j <= 20; j++) {
    ASSERT_TRUE(numeric(count[j / 2]) == agg.get(j));
  }
}

#endif /* TEST_AGGREGATE_TEST_H_ */
