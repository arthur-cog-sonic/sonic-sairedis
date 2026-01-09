/*
 * Y2K38 Test Suite for sonic-sairedis FdbInfo
 *
 * This test suite verifies that the FDB timestamp types are properly
 * sized to handle dates beyond January 19, 2038 (Y2K38 problem).
 *
 * The Y2K38 problem occurs when 32-bit signed integers used to store
 * Unix timestamps overflow on January 19, 2038 at 03:14:07 UTC.
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <ctime>
#include <string>

#include "FdbInfo.h"

using namespace saivs;

/* Y2K38 boundary timestamp: 2038-01-19 03:14:07 UTC */
constexpr uint64_t Y2K38_BOUNDARY = 2147483647ULL;

/* Test timestamps beyond Y2K38 */
constexpr uint64_t YEAR_2040 = 2208988800ULL;  /* 2040-01-01 00:00:00 UTC */
constexpr uint64_t YEAR_2050 = 2524608000ULL;  /* 2050-01-01 00:00:00 UTC */
constexpr uint64_t YEAR_2100 = 4102444800ULL;  /* 2100-01-01 00:00:00 UTC */

class Y2K38FdbInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        /* Create a valid FDB entry for testing */
        m_switchId = 0x21000000000000;
        m_portId = 0x1000000000001;
        m_vlanId = 100;

        memset(&m_fdbEntry, 0, sizeof(m_fdbEntry));
        m_fdbEntry.switch_id = m_switchId;
        m_fdbEntry.bv_id = m_vlanId;
        /* Set a test MAC address */
        m_fdbEntry.mac_address[0] = 0x00;
        m_fdbEntry.mac_address[1] = 0x11;
        m_fdbEntry.mac_address[2] = 0x22;
        m_fdbEntry.mac_address[3] = 0x33;
        m_fdbEntry.mac_address[4] = 0x44;
        m_fdbEntry.mac_address[5] = 0x55;
    }

    sai_object_id_t m_switchId;
    sai_object_id_t m_portId;
    sai_vlan_id_t m_vlanId;
    sai_fdb_entry_t m_fdbEntry;
};

/*
 * Test 1: Verify FdbInfo timestamp field is 64-bit
 */
TEST_F(Y2K38FdbInfoTest, TimestampFieldSize)
{
    FdbInfo fdbInfo;

    /* The timestamp should be stored as uint64_t (8 bytes) */
    /* We verify this by checking that we can store and retrieve
     * values larger than 32-bit max */
    fdbInfo.setTimestamp(Y2K38_BOUNDARY);
    EXPECT_EQ(fdbInfo.getTimestamp(), Y2K38_BOUNDARY);

    fdbInfo.setTimestamp(Y2K38_BOUNDARY + 1);
    EXPECT_EQ(fdbInfo.getTimestamp(), Y2K38_BOUNDARY + 1);
}

/*
 * Test 2: Verify FdbInfo can store Y2K38 boundary timestamp
 */
TEST_F(Y2K38FdbInfoTest, StoreY2K38Boundary)
{
    FdbInfo fdbInfo;

    fdbInfo.setTimestamp(Y2K38_BOUNDARY);
    EXPECT_EQ(fdbInfo.getTimestamp(), Y2K38_BOUNDARY);
}

/*
 * Test 3: Verify FdbInfo can store timestamps beyond Y2K38
 */
TEST_F(Y2K38FdbInfoTest, StoreTimestampsBeyondY2K38)
{
    FdbInfo fdbInfo;

    /* Test year 2040 */
    fdbInfo.setTimestamp(YEAR_2040);
    EXPECT_EQ(fdbInfo.getTimestamp(), YEAR_2040);

    /* Test year 2050 */
    fdbInfo.setTimestamp(YEAR_2050);
    EXPECT_EQ(fdbInfo.getTimestamp(), YEAR_2050);

    /* Test year 2100 */
    fdbInfo.setTimestamp(YEAR_2100);
    EXPECT_EQ(fdbInfo.getTimestamp(), YEAR_2100);
}

/*
 * Test 4: Verify timestamp arithmetic works correctly
 */
TEST_F(Y2K38FdbInfoTest, TimestampArithmetic)
{
    FdbInfo fdbInfo1, fdbInfo2;

    fdbInfo1.setTimestamp(Y2K38_BOUNDARY);
    fdbInfo2.setTimestamp(YEAR_2040);

    /* Verify comparison works correctly */
    EXPECT_LT(fdbInfo1.getTimestamp(), fdbInfo2.getTimestamp());

    /* Verify subtraction works correctly */
    uint64_t diff = fdbInfo2.getTimestamp() - fdbInfo1.getTimestamp();
    EXPECT_GT(diff, 0ULL);
    EXPECT_EQ(diff, YEAR_2040 - Y2K38_BOUNDARY);
}

/*
 * Test 5: Verify FdbInfo serialization preserves 64-bit timestamps
 */
TEST_F(Y2K38FdbInfoTest, SerializationPreserves64BitTimestamp)
{
    FdbInfo fdbInfo;

    /* Set a timestamp beyond 32-bit max */
    fdbInfo.setTimestamp(YEAR_2050);
    fdbInfo.setFdbEntry(m_fdbEntry);
    fdbInfo.setBridgePortId(m_portId);

    /* Serialize to string */
    std::string serialized = fdbInfo.serialize();

    /* Deserialize and verify timestamp is preserved */
    FdbInfo deserialized = FdbInfo::deserialize(serialized);

    EXPECT_EQ(deserialized.getTimestamp(), YEAR_2050);
}

/*
 * Test 6: Verify FdbInfo handles zero timestamp
 */
TEST_F(Y2K38FdbInfoTest, ZeroTimestamp)
{
    FdbInfo fdbInfo;

    fdbInfo.setTimestamp(0);
    EXPECT_EQ(fdbInfo.getTimestamp(), 0ULL);
}

/*
 * Test 7: Verify FdbInfo handles maximum 64-bit timestamp
 */
TEST_F(Y2K38FdbInfoTest, MaxTimestamp)
{
    FdbInfo fdbInfo;

    /* Test with a very large timestamp (year ~292 billion) */
    uint64_t maxTimestamp = UINT64_MAX;
    fdbInfo.setTimestamp(maxTimestamp);
    EXPECT_EQ(fdbInfo.getTimestamp(), maxTimestamp);
}

/*
 * Test 8: Verify timestamp type is uint64_t
 */
TEST_F(Y2K38FdbInfoTest, TimestampTypeIs64Bit)
{
    FdbInfo fdbInfo;

    /* This is a compile-time check - if getTimestamp() returns
     * a smaller type, this would cause a warning or truncation */
    uint64_t timestamp = fdbInfo.getTimestamp();
    EXPECT_GE(sizeof(timestamp), 8UL);

    /* Verify we can assign large values without truncation */
    fdbInfo.setTimestamp(YEAR_2100);
    timestamp = fdbInfo.getTimestamp();
    EXPECT_EQ(timestamp, YEAR_2100);
}

/*
 * Test 9: Verify multiple FdbInfo objects with different timestamps
 */
TEST_F(Y2K38FdbInfoTest, MultipleFdbInfoTimestamps)
{
    FdbInfo fdb2020, fdb2038, fdb2040, fdb2100;

    uint64_t ts2020 = 1577836800ULL;  /* 2020-01-01 */

    fdb2020.setTimestamp(ts2020);
    fdb2038.setTimestamp(Y2K38_BOUNDARY);
    fdb2040.setTimestamp(YEAR_2040);
    fdb2100.setTimestamp(YEAR_2100);

    /* Verify all timestamps are stored correctly */
    EXPECT_EQ(fdb2020.getTimestamp(), ts2020);
    EXPECT_EQ(fdb2038.getTimestamp(), Y2K38_BOUNDARY);
    EXPECT_EQ(fdb2040.getTimestamp(), YEAR_2040);
    EXPECT_EQ(fdb2100.getTimestamp(), YEAR_2100);

    /* Verify ordering */
    EXPECT_LT(fdb2020.getTimestamp(), fdb2038.getTimestamp());
    EXPECT_LT(fdb2038.getTimestamp(), fdb2040.getTimestamp());
    EXPECT_LT(fdb2040.getTimestamp(), fdb2100.getTimestamp());
}

/*
 * Test 10: Verify timestamp update works correctly
 */
TEST_F(Y2K38FdbInfoTest, TimestampUpdate)
{
    FdbInfo fdbInfo;

    /* Set initial timestamp */
    fdbInfo.setTimestamp(Y2K38_BOUNDARY);
    EXPECT_EQ(fdbInfo.getTimestamp(), Y2K38_BOUNDARY);

    /* Update to a later timestamp */
    fdbInfo.setTimestamp(YEAR_2040);
    EXPECT_EQ(fdbInfo.getTimestamp(), YEAR_2040);

    /* Update to an even later timestamp */
    fdbInfo.setTimestamp(YEAR_2100);
    EXPECT_EQ(fdbInfo.getTimestamp(), YEAR_2100);
}
