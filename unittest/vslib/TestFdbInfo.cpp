#include "FdbInfo.h"

#include <linux/if.h>

#include <gtest/gtest.h>

using namespace saivs;

TEST(FdbInfo, getPortId)
{
    FdbInfo fdb;

    EXPECT_EQ(fdb.getPortId(), 0);
}

TEST(FdbInfo, getVlanId)
{
    FdbInfo fdb;

    EXPECT_EQ(fdb.getVlanId(), 0);
}

TEST(FdbInfo, getBridgePortId)
{
    FdbInfo fdb;

    EXPECT_EQ(fdb.getBridgePortId(), 0);
}

TEST(FdbInfo, getFdbEntry)
{
    FdbInfo fdb;

    auto entry = fdb.getFdbEntry();

    EXPECT_EQ(entry.switch_id, 0);
}

TEST(FdbInfo, getTimestamp)
{
    FdbInfo fdb;

    EXPECT_EQ(fdb.getTimestamp(), 0);
}

TEST(FdbInfo, serialize)
{
    FdbInfo fdb;

    auto str = fdb.serialize();

    EXPECT_EQ(str,
            "{\"bridge_port_id\":\"oid:0x0\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:00\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"0\"}");
}

TEST(FdbInfo, deserialize)
{
    std::string str =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:00\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"0\"}";

    auto fdb = FdbInfo::deserialize(str);

    EXPECT_EQ(fdb.getBridgePortId(), 1);
}

TEST(FdbInfo, setFdbEntry)
{
    FdbInfo fdb;

    sai_fdb_entry_t entry;

    entry.switch_id = 2;

    fdb.setFdbEntry(entry);

    auto en = fdb.getFdbEntry();

    EXPECT_EQ(en.switch_id, 2);
}

TEST(FdbInfo, setVlanId)
{
    FdbInfo info;

    info.setVlanId(7);

    EXPECT_EQ(info.getVlanId(), 7);
}

TEST(FdbInfo, setPortId)
{
    FdbInfo info;

    info.setPortId(7);

    EXPECT_EQ(info.getPortId(), 7);
}

TEST(FdbInfo, setBridgePortId)
{
    FdbInfo info;

    info.setBridgePortId(7);

    EXPECT_EQ(info.getBridgePortId(), 7);
}

TEST(FdbInfo, setTimestamp)
{
    FdbInfo info;

    info.setTimestamp(7);

    EXPECT_EQ(info.getTimestamp(), 7);
}

TEST(FdbInfo, operator_bracket)
{
    std::string strA =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:00\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"0\"}";

    std::string strB =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:01\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"0\"}";

    auto a = FdbInfo::deserialize(strA);
    auto b = FdbInfo::deserialize(strB);

    EXPECT_EQ(a.operator()(a,b), true);
    EXPECT_EQ(a.operator()(b,a), false);
}

// Y2038 timestamp overflow tests - verify 64-bit timestamp handling
TEST(FdbInfo, setTimestamp_Y2038_LargeValue)
{
    FdbInfo info;

    // Test value beyond 32-bit signed max (2^31 - 1 = 2147483647)
    // This is the Y2038 overflow point
    uint64_t y2038_overflow = 2147483648ULL;  // 2^31
    info.setTimestamp(y2038_overflow);

    EXPECT_EQ(info.getTimestamp(), y2038_overflow);
}

TEST(FdbInfo, setTimestamp_Y2038_MaxUint32)
{
    FdbInfo info;

    // Test value at 32-bit unsigned max (2^32 - 1 = 4294967295)
    uint64_t max_uint32 = 4294967295ULL;
    info.setTimestamp(max_uint32);

    EXPECT_EQ(info.getTimestamp(), max_uint32);
}

TEST(FdbInfo, setTimestamp_Y2038_Beyond32Bit)
{
    FdbInfo info;

    // Test value beyond 32-bit range (2^32 + 1000)
    uint64_t beyond_32bit = 4294968295ULL;
    info.setTimestamp(beyond_32bit);

    EXPECT_EQ(info.getTimestamp(), beyond_32bit);
}

TEST(FdbInfo, serialize_Y2038_LargeTimestamp)
{
    FdbInfo fdb;

    // Set timestamp beyond Y2038 overflow point
    uint64_t large_timestamp = 5000000000ULL;  // ~2128 year
    fdb.setTimestamp(large_timestamp);

    auto str = fdb.serialize();

    // Verify the serialized string contains the large timestamp
    EXPECT_NE(str.find("\"timestamp\":\"5000000000\""), std::string::npos);
}

TEST(FdbInfo, deserialize_Y2038_LargeTimestamp)
{
    // Test deserializing a timestamp beyond 32-bit range
    std::string str =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:00\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"5000000000\","
            "\"vlan_id\":\"0\"}";

    auto fdb = FdbInfo::deserialize(str);

    EXPECT_EQ(fdb.getTimestamp(), 5000000000ULL);
}

TEST(FdbInfo, roundtrip_Y2038_LargeTimestamp)
{
    FdbInfo original;

    // Set a timestamp far beyond Y2038
    uint64_t future_timestamp = 10000000000ULL;  // ~2286 year
    original.setTimestamp(future_timestamp);
    original.setBridgePortId(1);
    original.setVlanId(100);

    // Serialize and deserialize
    auto serialized = original.serialize();
    auto deserialized = FdbInfo::deserialize(serialized);

    // Verify timestamp survives round-trip
    EXPECT_EQ(deserialized.getTimestamp(), future_timestamp);
    EXPECT_EQ(deserialized.getBridgePortId(), 1);
    EXPECT_EQ(deserialized.getVlanId(), 100);
}

TEST(FdbInfo, operator_lt)
{
    std::string strA =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:00\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"0\"}";

    std::string strB =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:01\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"0\"}";

    auto a = FdbInfo::deserialize(strA);
    auto b = FdbInfo::deserialize(strB);

    EXPECT_EQ(a < b, true);
    EXPECT_EQ(b < a, false);
    EXPECT_EQ(a < a, false);
    EXPECT_EQ(b < b, false);

    std::string strC =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:00\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"1\"}";

    std::string strD =
            "{\"bridge_port_id\":\"oid:0x1\","
            "\"fdb_entry\":\"{\\\"bvid\\\":\\\"oid:0x0\\\",\\\"mac\\\":\\\"00:00:00:00:00:00\\\",\\\"switch_id\\\":\\\"oid:0x0\\\"}\","
            "\"port_id\":\"oid:0x0\","
            "\"timestamp\":\"0\","
            "\"vlan_id\":\"2\"}";

    auto c = FdbInfo::deserialize(strC);
    auto d = FdbInfo::deserialize(strD);

    EXPECT_EQ(c < d, true);
    EXPECT_EQ(d < c, false);
    EXPECT_EQ(c < c, false);
    EXPECT_EQ(d < d, false);

}
