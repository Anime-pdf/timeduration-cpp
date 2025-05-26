#include <timeduration/timeduration.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace timeduration;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void demonstrate_parsing() {
    print_separator("PARSING DEMONSTRATIONS");

    // Different formats
    std::vector<std::string> formats = {
        "5s",
        "10m",
        "2h",
        "1d",
        "1h 30m",
        "2h 30m 15s",
        "1d 5h 30m 45s",
        "1 hours 30 minutes 45 seconds",
        "90m",  // Will normalize to 1h 30m
        "3661s", // Will normalize to 1h 1m 1s
        "1y 6mo 15d 2h 30m",
        "999h"
    };

    std::cout << std::left << std::setw(25) << "Input"
              << std::setw(15) << "Total Seconds"
              << std::setw(20) << "Formatted"
              << "Breakdown" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    for (const auto& format : formats) {
        CTimePeriod duration(format);

        std::cout << std::setw(25) << format
                  << std::setw(15) << duration.duration().count()
                  << std::setw(20) << duration.toString()
                  << duration.days() << "d "
                  << duration.hours() << "h "
                  << duration.minutes() << "m "
                  << duration.seconds() << "s"
                  << std::endl;
    }
}

void demonstrate_construction() {
    print_separator("CONSTRUCTION METHODS");

    // Different ways to create CTimePeriod objects
    std::cout << "1. From string: ";
    CTimePeriod from_string("2h 30m 15s");
    std::cout << from_string.toString() << " (" << from_string.duration().count() << "s)" << std::endl;

    std::cout << "2. From components (s, m, h, d): ";
    CTimePeriod from_components(15, 30, 2, 0);  // 15s, 30m, 2h, 0d
    std::cout << from_components.toString() << " (" << from_components.duration().count() << "s)" << std::endl;

    std::cout << "3. From chrono::seconds: ";
    CTimePeriod from_chrono(std::chrono::seconds(3661));
    std::cout << from_chrono.toString() << " (" << from_chrono.duration().count() << "s)" << std::endl;

    std::cout << "4. Using ParseFactory: ";
    auto from_factory = CTimePeriod::ParseFactory("1d 12h");
    std::cout << from_factory.toString() << " (" << from_factory.duration().count() << "s)" << std::endl;

    std::cout << "5. Default constructor: ";
    CTimePeriod default_constructed;
    std::cout << default_constructed.toString() << " (zero: "
              << (default_constructed.isZero() ? "true" : "false") << ")" << std::endl;
}

void demonstrate_comparisons() {
    print_separator("COMPARISON OPERATIONS");

    CTimePeriod duration1("1h 30m");
    CTimePeriod duration2("90m");        // Same as 1h 30m
    CTimePeriod duration3("2h");
    CTimePeriod duration4("45m");

    std::cout << "duration1 = " << duration1.toString() << " (" << duration1.duration().count() << "s)" << std::endl;
    std::cout << "duration2 = " << duration2.toString() << " (" << duration2.duration().count() << "s)" << std::endl;
    std::cout << "duration3 = " << duration3.toString() << " (" << duration3.duration().count() << "s)" << std::endl;
    std::cout << "duration4 = " << duration4.toString() << " (" << duration4.duration().count() << "s)" << std::endl;
    std::cout << std::endl;

    // Equality tests
    std::cout << "Equality tests:" << std::endl;
    std::cout << "  duration1 == duration2: " << (duration1 == duration2 ? "true" : "false") << std::endl;
    std::cout << "  duration1 != duration3: " << (duration1 != duration3 ? "true" : "false") << std::endl;

    // Relational tests
    std::cout << "\nRelational tests:" << std::endl;
    std::cout << "  duration4 < duration1: " << (duration4 < duration1 ? "true" : "false") << std::endl;
    std::cout << "  duration3 > duration1: " << (duration3 > duration1 ? "true" : "false") << std::endl;
    std::cout << "  duration1 <= duration2: " << (duration1 <= duration2 ? "true" : "false") << std::endl;
    std::cout << "  duration3 >= duration1: " << (duration3 >= duration1 ? "true" : "false") << std::endl;
}

void demonstrate_normalization() {
    print_separator("AUTOMATIC NORMALIZATION");

    std::cout << "The library automatically normalizes overflow values:\n" << std::endl;

    struct TestCase {
        std::string description;
        int64_t seconds, minutes, hours, days;
    };

    std::vector<TestCase> test_cases = {
        {"75 seconds", 75, 0, 0, 0},
        {"90 minutes", 0, 90, 0, 0},
        {"25 hours", 0, 0, 25, 0},
        {"Complex overflow", 3725, 90, 25, 0}
    };

    for (const auto& test : test_cases) {
        CTimePeriod duration(test.seconds, test.minutes, test.hours, test.days);
        std::cout << test.description << ":" << std::endl;
        std::cout << "  Input: " << test.seconds << "s " << test.minutes << "m "
                  << test.hours << "h " << test.days << "d" << std::endl;
        std::cout << "  Result: " << duration.toString() << std::endl;
        std::cout << "  Breakdown: " << duration.days() << "d "
                  << duration.hours() << "h " << duration.minutes() << "m "
                  << duration.seconds() << "s" << std::endl << std::endl;
    }
}

void demonstrate_formatting() {
    print_separator("OUTPUT FORMATTING");

    std::vector<std::string> durations = {
        "2h 30m 15s",
        "1d 5h",
        "30m",
        "45s",
        "0s",
        "1y 2mo 3d 4h 5m 6s"
    };

    std::cout << std::left << std::setw(20) << "Duration"
              << std::setw(25) << "toString()"
              << "asSqlInterval()" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto& dur_str : durations) {
        CTimePeriod duration(dur_str);
        std::cout << std::setw(20) << dur_str
                  << std::setw(25) << duration.toString()
                  << duration.asSqlInterval() << std::endl;
    }
}

int main() {
    std::cout << "TimeDuration Library - Basic Usage Examples" << std::endl;
    std::cout << "===========================================" << std::endl;

    demonstrate_parsing();
    demonstrate_construction();
    demonstrate_comparisons();
    demonstrate_normalization();
    demonstrate_formatting();

    return 0;
}