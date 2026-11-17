#include <gtest/gtest.h>

#include <limits.h>

extern "C" {
#include "../src/helpers.h"
}

TEST(Helpers, ReadAllValidBitFields) {
  for (int pos = 0; pos < 8; pos++) {
    for (int num = 1; num <= 8 - pos; num++) {
      const unsigned divisor = 1u << pos;
      const unsigned range = 1u << num;
      for (unsigned byte = 0; byte <= 0xFF; byte++) {
        const unsigned expected = (byte / divisor) % range;
        ASSERT_EQ(lwmqtt_read_bits((uint8_t)byte, pos, num), expected)
            << "byte=" << byte << " pos=" << pos << " num=" << num;
      }
    }
  }
}

TEST(Helpers, WriteAllValidBitFields) {
  for (int pos = 0; pos < 8; pos++) {
    for (int num = 1; num <= 8 - pos; num++) {
      for (unsigned original = 0; original <= 0xFF; original++) {
        for (unsigned value = 0; value <= 0xFF; value++) {
          uint8_t expected = (uint8_t)original;
          for (int bit = 0; bit < num; bit++) {
            const unsigned bit_mask = 1u << (pos + bit);
            expected = (uint8_t)((expected & ~bit_mask) | (((value >> bit) & 1u) << (pos + bit)));
          }
          uint8_t actual = (uint8_t)original;
          lwmqtt_write_bits(&actual, (uint8_t)value, pos, num);
          ASSERT_EQ(actual, expected)
              << "original=" << original << " value=" << value << " pos=" << pos << " num=" << num;
        }
      }
    }
  }
}

TEST(Helpers, InvalidBitFields) {
  const int ranges[][2] = {{-1, 1},      {8, 1},       {9, 1},       {0, 0},       {0, -1},      {0, 9},
                           {7, 2},       {INT_MIN, 1}, {INT_MAX, 1}, {0, INT_MIN}, {0, INT_MAX}};

  for (size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); i++) {
    int pos = ranges[i][0];
    int num = ranges[i][1];
    uint8_t byte = 0xA5;

    EXPECT_EQ(lwmqtt_read_bits(byte, pos, num), 0) << "pos=" << pos << " num=" << num;

    lwmqtt_write_bits(&byte, 0xFF, pos, num);
    EXPECT_EQ(byte, 0xA5) << "pos=" << pos << " num=" << num;
  }
}

TEST(Helpers, WriteBitsNullByte) {
  lwmqtt_write_bits(nullptr, 0xFF, 0, 8);
  lwmqtt_write_bits(nullptr, 0xFF, INT_MIN, INT_MAX);
}

TEST(Helpers, VarNum1) {
  uint8_t buf[2] = {0};

  for (uint32_t i = 1; i < 128; i++) {
    int len;
    lwmqtt_err_t err = lwmqtt_varnum_length(i, &len);
    EXPECT_EQ(err, LWMQTT_SUCCESS);
    EXPECT_EQ(len, 1);

    uint8_t *ptr1 = buf;
    err = lwmqtt_write_varnum(&ptr1, buf + 1, i);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    uint8_t *ptr2 = buf;
    uint32_t num;
    err = lwmqtt_read_varnum(&ptr2, buf + 1, &num);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    EXPECT_EQ(num, i);
    EXPECT_EQ(buf[1], 0);
    EXPECT_EQ(ptr1 - buf, 1);
    EXPECT_EQ(ptr2 - buf, 1);
  }
}

TEST(Helpers, VarNum2) {
  uint8_t buf[3] = {0};

  for (uint32_t i = 1; i < 128; i++) {
    int len;
    lwmqtt_err_t err = lwmqtt_varnum_length(128 * i, &len);
    EXPECT_EQ(err, LWMQTT_SUCCESS);
    EXPECT_EQ(len, 2);

    uint8_t *ptr1 = buf;
    err = lwmqtt_write_varnum(&ptr1, buf + 2, 128 * i);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    uint8_t *ptr2 = buf;
    uint32_t num;
    err = lwmqtt_read_varnum(&ptr2, buf + 2, &num);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    EXPECT_EQ(num, 128 * i);
    EXPECT_EQ(buf[2], 0);
    EXPECT_EQ(ptr1 - buf, 2);
    EXPECT_EQ(ptr2 - buf, 2);
  }
}

TEST(Helpers, VarNum3) {
  uint8_t buf[4] = {0};

  for (uint32_t i = 1; i < 128; i++) {
    int len;
    lwmqtt_err_t err = lwmqtt_varnum_length(128 * 128 * i, &len);
    EXPECT_EQ(err, LWMQTT_SUCCESS);
    EXPECT_EQ(len, 3);

    uint8_t *ptr1 = buf;
    err = lwmqtt_write_varnum(&ptr1, buf + 3, 128 * 128 * i);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    uint8_t *ptr2 = buf;
    uint32_t num;
    err = lwmqtt_read_varnum(&ptr2, buf + 3, &num);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    EXPECT_EQ(num, 128 * 128 * i);
    EXPECT_EQ(buf[3], 0);
    EXPECT_EQ(ptr1 - buf, 3);
    EXPECT_EQ(ptr2 - buf, 3);
  }
}

TEST(Helpers, VarNum4) {
  uint8_t buf[5] = {0};

  for (uint32_t i = 1; i < 128; i++) {
    int len;
    lwmqtt_err_t err = lwmqtt_varnum_length(128 * 128 * 128 * i, &len);
    EXPECT_EQ(err, LWMQTT_SUCCESS);
    EXPECT_EQ(len, 4);

    uint8_t *ptr1 = buf;
    err = lwmqtt_write_varnum(&ptr1, buf + 4, 128 * 128 * 128 * i);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    uint8_t *ptr2 = buf;
    uint32_t num;
    err = lwmqtt_read_varnum(&ptr2, buf + 4, &num);
    EXPECT_EQ(err, LWMQTT_SUCCESS);

    EXPECT_EQ(num, 128 * 128 * 128 * i);
    EXPECT_EQ(buf[4], 0);
    EXPECT_EQ(ptr1 - buf, 4);
    EXPECT_EQ(ptr2 - buf, 4);
  }
}

TEST(Helpers, VarNumMax) {
  uint8_t buf[4] = {0xFF, 0xFF, 0xFF, 0x7F};

  uint8_t *ptr = buf;
  uint32_t num;
  lwmqtt_err_t err = lwmqtt_read_varnum(&ptr, buf + 4, &num);
  EXPECT_EQ(err, LWMQTT_SUCCESS);

  EXPECT_EQ(num, (uint32_t)268435455);
}

TEST(Helpers, VarNumLengthBoundaries) {
  int len = 0;

  EXPECT_EQ(lwmqtt_varnum_length(2097151, &len), LWMQTT_SUCCESS);
  EXPECT_EQ(len, 3);

  EXPECT_EQ(lwmqtt_varnum_length(268435455, &len), LWMQTT_SUCCESS);
  EXPECT_EQ(len, 4);
}
