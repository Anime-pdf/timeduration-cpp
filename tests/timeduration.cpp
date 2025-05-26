#include <gtest/gtest.h>
#include <timeduration/timeduration.hpp>
#include <chrono>

using namespace timeduration;

class CTimePeriodTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ========== Scanner Tests ==========

class CScannerTest : public ::testing::Test {
protected:
    static CTimePeriod::TokenHolder GetDefaultTokens() {
        return {
            {"s", 1L}, {"seconds", 1L},
            {"m", 60L}, {"minutes", 60L},
            {"h", 3600L}, {"hours", 3600L},
            {"d", 86400L}, {"days", 86400L},
            {"mo", 2419200L}, {"months", 2419200L},
            {"y", 31536000L}, {"years", 31536000L},
        };
    }
};

TEST_F(CScannerTest, ParsesSingleUnit) {
    CTimePeriod::CScanner scanner("5s", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[1], 5); // 5 seconds
}

TEST_F(CScannerTest, ParsesMultipleUnits) {
    CTimePeriod::CScanner scanner("2h 30m 15s", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[3600], 2);  // 2 hours
    EXPECT_EQ(result[60], 30);   // 30 minutes
    EXPECT_EQ(result[1], 15);    // 15 seconds
}

TEST_F(CScannerTest, ParsesLongFormUnits) {
    CTimePeriod::CScanner scanner("1 hours 30 minutes 45 seconds", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[3600], 1);  // 1 hour
    EXPECT_EQ(result[60], 30);   // 30 minutes
    EXPECT_EQ(result[1], 45);    // 45 seconds
}

TEST_F(CScannerTest, ParsesDaysAndLargerUnits) {
    CTimePeriod::CScanner scanner("1y 2mo 3d", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[31536000], 1);  // 1 year
    EXPECT_EQ(result[2419200], 2);   // 2 months
    EXPECT_EQ(result[86400], 3);     // 3 days
}

TEST_F(CScannerTest, HandlesLargeNumbers) {
    CTimePeriod::CScanner scanner("999h 123456s", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[3600], 999);    // 999 hours
    EXPECT_EQ(result[1], 123456);    // 123456 seconds
}

TEST_F(CScannerTest, HandlesDuplicateUnits) {
    CTimePeriod::CScanner scanner("5m 10m", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[60], 15);  // 5 + 10 = 15 minutes
}

TEST_F(CScannerTest, HandlesEmptyString) {
    CTimePeriod::CScanner scanner("", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    EXPECT_EQ(result.size(), 0);
}

TEST_F(CScannerTest, HandlesNumberWithoutUnit) {
    CTimePeriod::CScanner scanner("120", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[60], 120);  // Defaults to minutes
}

TEST_F(CScannerTest, HandlesMixedFormats) {
    CTimePeriod::CScanner scanner("1h 90 30s", GetDefaultTokens());
    auto result = scanner.ScanTokens();

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[3600], 1);  // 1 hour
    EXPECT_EQ(result[60], 90);   // 90 minutes (number without unit)
    EXPECT_EQ(result[1], 30);    // 30 seconds
}

// ========== Parser Tests ==========

TEST_F(CTimePeriodTest, ParsesBasicTimeFormats) {
    EXPECT_EQ(CTimePeriod::Parse("1s").count(), 1);
    EXPECT_EQ(CTimePeriod::Parse("1m").count(), 60);
    EXPECT_EQ(CTimePeriod::Parse("1h").count(), 3600);
    EXPECT_EQ(CTimePeriod::Parse("1d").count(), 86400);
}

TEST_F(CTimePeriodTest, ParsesComplexTimeFormats) {
    auto duration = CTimePeriod::Parse("2h 30m 15s");
    EXPECT_EQ(duration.count(), 2 * 3600 + 30 * 60 + 15);
}

TEST_F(CTimePeriodTest, ParsesLongFormUnits) {
    auto duration = CTimePeriod::Parse("1 hours 30 minutes 45 seconds");
    EXPECT_EQ(duration.count(), 1 * 3600 + 30 * 60 + 45);
}

TEST_F(CTimePeriodTest, ParsesLargerUnits) {
    EXPECT_EQ(CTimePeriod::Parse("1mo").count(), 2419200);  // 28 days
    EXPECT_EQ(CTimePeriod::Parse("1y").count(), 31536000);  // 365 days
}

TEST_F(CTimePeriodTest, ParsesZeroDuration) {
    EXPECT_EQ(CTimePeriod::Parse("0s").count(), 0);
    EXPECT_EQ(CTimePeriod::Parse("").count(), 0);
}

TEST_F(CTimePeriodTest, HandlesLargeNumbers) {
    auto duration = CTimePeriod::Parse("999h");
    EXPECT_EQ(duration.count(), 999 * 3600);
}

// ========== Constructor Tests ==========

TEST_F(CTimePeriodTest, ConstructorFromComponents) {
    CTimePeriod period(15, 30, 2, 1);  // 1d 2h 30m 15s

    EXPECT_EQ(period.days(), 1);
    EXPECT_EQ(period.hours(), 2);
    EXPECT_EQ(period.minutes(), 30);
    EXPECT_EQ(period.seconds(), 15);
    EXPECT_EQ(period.duration().count(), 86400 + 7200 + 1800 + 15);
}

TEST_F(CTimePeriodTest, ConstructorFromString) {
    CTimePeriod period("2h 30m 15s");

    EXPECT_EQ(period.hours(), 2);
    EXPECT_EQ(period.minutes(), 30);
    EXPECT_EQ(period.seconds(), 15);
    EXPECT_EQ(period.duration().count(), 2 * 3600 + 30 * 60 + 15);
}

TEST_F(CTimePeriodTest, ConstructorFromChronoSeconds) {
    CTimePeriod period(std::chrono::seconds(3661));  // 1h 1m 1s

    EXPECT_EQ(period.hours(), 1);
    EXPECT_EQ(period.minutes(), 1);
    EXPECT_EQ(period.seconds(), 1);
    EXPECT_EQ(period.duration().count(), 3661);
}

TEST_F(CTimePeriodTest, DefaultConstructor) {
    CTimePeriod period;

    EXPECT_EQ(period.days(), 0);
    EXPECT_EQ(period.hours(), 0);
    EXPECT_EQ(period.minutes(), 0);
    EXPECT_EQ(period.seconds(), 0);
    EXPECT_EQ(period.duration().count(), 0);
    EXPECT_TRUE(period.isZero());
}

// ========== Normalization Tests ==========

TEST_F(CTimePeriodTest, NormalizesExcessSeconds) {
    CTimePeriod period(75);  // 75 seconds = 1m 15s

    EXPECT_EQ(period.minutes(), 1);
    EXPECT_EQ(period.seconds(), 15);
}

TEST_F(CTimePeriodTest, NormalizesExcessMinutes) {
    CTimePeriod period(0, 90);  // 90 minutes = 1h 30m

    EXPECT_EQ(period.hours(), 1);
    EXPECT_EQ(period.minutes(), 30);
}

TEST_F(CTimePeriodTest, NormalizesExcessHours) {
    CTimePeriod period(0, 0, 25);  // 25 hours = 1d 1h

    EXPECT_EQ(period.days(), 1);
    EXPECT_EQ(period.hours(), 1);
}

TEST_F(CTimePeriodTest, NormalizesComplexDuration) {
    CTimePeriod period(3725, 90, 25, 0);  // Complex normalization

    // 3725s = 1h 2m 5s
    // 90m = 1h 30m
    // 25h = 1d 1h
    // Total: 1d + (1+1+1)h + (2+30)m + 5s = 1d 3h 32m 5s

    EXPECT_EQ(period.days(), 1);
    EXPECT_EQ(period.hours(), 3);
    EXPECT_EQ(period.minutes(), 32);
    EXPECT_EQ(period.seconds(), 5);
}

// ========== Formatting Tests ==========

TEST_F(CTimePeriodTest, ToStringFormat) {
    EXPECT_EQ(CTimePeriod("2h 30m 15s").toString(), "2h 30m 15s");
    EXPECT_EQ(CTimePeriod("1d 5h").toString(), "1d 5h 0s");
    EXPECT_EQ(CTimePeriod("30m").toString(), "30m 0s");
    EXPECT_EQ(CTimePeriod("0s").toString(), "0s");
}

TEST_F(CTimePeriodTest, SqlIntervalFormat) {
    EXPECT_EQ(CTimePeriod("1h").asSqlInterval(), "interval 3600 second");
    EXPECT_EQ(CTimePeriod("2h 30m").asSqlInterval(), "interval 9000 second");
}

// ========== Comparison Tests ==========

TEST_F(CTimePeriodTest, EqualityComparison) {
    CTimePeriod period1("1h 30m");
    CTimePeriod period2("90m");
    CTimePeriod period3("1h 31m");

    EXPECT_TRUE(period1 == period2);
    EXPECT_FALSE(period1 == period3);
    EXPECT_FALSE(period1 != period2);
    EXPECT_TRUE(period1 != period3);
}

TEST_F(CTimePeriodTest, RelationalComparison) {
    CTimePeriod small("30m");
    CTimePeriod large("1h");

    EXPECT_TRUE(small < large);
    EXPECT_TRUE(small <= large);
    EXPECT_FALSE(small > large);
    EXPECT_FALSE(small >= large);

    EXPECT_TRUE(large > small);
    EXPECT_TRUE(large >= small);
    EXPECT_FALSE(large < small);
    EXPECT_FALSE(large <= small);
}

TEST_F(CTimePeriodTest, SelfComparison) {
    CTimePeriod period("1h 30m");

    EXPECT_TRUE(period == period);
    EXPECT_FALSE(period != period);
    EXPECT_TRUE(period <= period);
    EXPECT_TRUE(period >= period);
    EXPECT_FALSE(period < period);
    EXPECT_FALSE(period > period);
}

// ========== Edge Cases Tests ==========

TEST_F(CTimePeriodTest, HandlesZeroValues) {
    CTimePeriod period("0h 0m 0s");

    EXPECT_EQ(period.days(), 0);
    EXPECT_EQ(period.hours(), 0);
    EXPECT_EQ(period.minutes(), 0);
    EXPECT_EQ(period.seconds(), 0);
    EXPECT_TRUE(period.isZero());
}

TEST_F(CTimePeriodTest, HandlesVeryLargeDurations) {
    CTimePeriod period("999d 23h 59m 59s");

    EXPECT_EQ(period.days(), 999);
    EXPECT_EQ(period.hours(), 23);
    EXPECT_EQ(period.minutes(), 59);
    EXPECT_EQ(period.seconds(), 59);
}

TEST_F(CTimePeriodTest, ParseFactoryMethod) {
    auto period = CTimePeriod::ParseFactory("2h 30m");

    EXPECT_EQ(period.hours(), 2);
    EXPECT_EQ(period.minutes(), 30);
    EXPECT_EQ(period.duration().count(), 2 * 3600 + 30 * 60);
}

// ========== Stress Tests ==========

TEST_F(CTimePeriodTest, StressTestLargeValues) {
    CTimePeriod period(59, 59, 23, 365);  // Almost 1 year + 1 day

    EXPECT_GT(period.days(), 365);  // Should normalize
    EXPECT_GE(period.duration().count(), 365 * 86400);
}

TEST_F(CTimePeriodTest, StressTestComplexParsing) {
    std::string complex = "5y 11mo 29d 23h 59m 59s";
    CTimePeriod period(complex);

    EXPECT_GT(period.duration().count(), 0);
    EXPECT_FALSE(period.toString().empty());
}

// ========== Integration Tests ==========

TEST_F(CTimePeriodTest, RoundTripStringConversion) {
    std::vector<std::string> testCases = {
        "1h", "30m", "45s", "1d 2h", "2h 30m 15s",
        "1d 0h 0m 0s", "0d 5h 30m", "100h"
    };

    for (const auto& testCase : testCases) {
        CTimePeriod period(testCase);
        auto duration = period.duration().count();
        const auto chronoDuration = std::chrono::seconds(duration);

        CTimePeriod roundTrip(chronoDuration);

        EXPECT_EQ(period.duration().count(), roundTrip.duration().count())
            << "Failed round trip for: " << testCase;
    }
}
