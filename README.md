# timeduration-cpp

A modern C++17 header-only library for parsing and manipulating human-readable time durations.

[![Build and Test](https://github.com/anime-pdf/timeduration-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/anime-pdf/timeduration-cpp/actions/workflows/ci.yml)

## Features

- **Header-only**: Easy integration into any C++ project
- **Modern C++17**: Uses standard library chrono types
- **Flexible parsing**: Supports multiple time formats and units
- **Type-safe**: Built on `std::chrono::seconds` for accuracy
- **Zero dependencies**: No external libraries required for the core functionality
- **Cross-platform**: Works on Windows, Linux, and macOS
- **SQL integration**: Generate SQL-compatible interval strings

## Quick Start

```cpp
#include <timeduration/timeduration.hpp>
#include <iostream>

int main() {
    using namespace timeduration;
    
    // Parse a time duration string
    CTimePeriod duration("2h 30m 15s");
    
    // Access individual components
    std::cout << "Hours: " << duration.hours() << std::endl;     // 2
    std::cout << "Minutes: " << duration.minutes() << std::endl; // 30
    std::cout << "Seconds: " << duration.seconds() << std::endl; // 15
    
    // Get total duration in seconds
    std::cout << "Total: " << duration.duration().count() << " seconds" << std::endl; // 9015
    
    // Format back to string
    std::cout << "Formatted: " << duration.toString() << std::endl; // "2h 30m 15s"
    
    return 0;
}
```

## Installation

### Using CMake (Recommended)

1. **Clone the repository:**
   ```bash
   git clone https://github.com/anime-pdf/timeduration-cpp.git
   cd timeduration-cpp
   ```

2. **Build and install:**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build .
   sudo cmake --install .
   ```

3. **Use in your CMake project:**
   ```cmake
   find_package(timeduration REQUIRED)
   target_link_libraries(your_target PRIVATE timeduration::timeduration)
   ```

### Header-only Integration

Simply copy `include/timeduration/timeduration.hpp` to your project and include it:

```cpp
#include "timeduration.hpp"
```

## Supported Time Units

The library supports both short and long forms of time units:

| Unit | Short Form | Long Form | Seconds |
|------|------------|-----------|---------|
| Seconds | `s` | `seconds` | 1 |
| Minutes | `m` | `minutes` | 60 |
| Hours | `h` | `hours` | 3,600 |
| Days | `d` | `days` | 86,400 |
| Months | `mo` | `months` | 2,419,200 (28 days) |
| Years | `y` | `years` | 31,536,000 (365 days) |

## Usage Examples

### Basic Parsing

```cpp
using namespace timeduration;

// Different formats
CTimePeriod p1("5s");              // 5 seconds
CTimePeriod p2("10m");             // 10 minutes  
CTimePeriod p3("2h");              // 2 hours
CTimePeriod p4("1d");              // 1 day

// Combined formats
CTimePeriod p5("1h 30m");          // 1 hour 30 minutes
CTimePeriod p6("2d 5h 30m 15s");   // 2 days 5 hours 30 minutes 15 seconds

// Long form
CTimePeriod p7("1 hours 30 minutes 45 seconds");

// Large units
CTimePeriod p8("1y 6mo 15d");      // 1 year 6 months 15 days
```

### Construction Methods

```cpp
// From string
CTimePeriod duration1("2h 30m");

// From components (seconds, minutes, hours, days)
CTimePeriod duration2(15, 30, 2, 1);  // 1d 2h 30m 15s

// From std::chrono::seconds
CTimePeriod duration3(std::chrono::seconds(3661));  // 1h 1m 1s

// Using factory method
auto duration4 = CTimePeriod::ParseFactory("5h 15m");

// Default constructor (zero duration)
CTimePeriod duration5;  // 0s
```

### Accessing Components

```cpp
CTimePeriod duration("1d 5h 30m 45s");

std::cout << "Days: " << duration.days() << std::endl;       // 1
std::cout << "Hours: " << duration.hours() << std::endl;     // 5
std::cout << "Minutes: " << duration.minutes() << std::endl; // 30
std::cout << "Seconds: " << duration.seconds() << std::endl; // 45

// Total duration as std::chrono::seconds
auto total = duration.duration();
std::cout << "Total seconds: " << total.count() << std::endl; // 106245

// Check if zero
if (duration.isZero()) {
    std::cout << "Duration is zero" << std::endl;
}
```

### Formatting and Output

```cpp
CTimePeriod duration("2h 30m 15s");

// Human-readable string
std::cout << duration.toString() << std::endl;      // "2h 30m 15s"

// SQL interval format
std::cout << duration.asSqlInterval() << std::endl; // "interval 9015 second"
```

### Comparisons

```cpp
CTimePeriod short_duration("30m");
CTimePeriod long_duration("1h");

// Equality
if (short_duration == long_duration) { /* ... */ }
if (short_duration != long_duration) { /* ... */ }

// Relational
if (short_duration < long_duration) { /* true */ }
if (short_duration > long_duration) { /* false */ }
if (short_duration <= long_duration) { /* true */ }
if (short_duration >= long_duration) { /* false */ }
```

### Advanced Usage

```cpp
// Handle normalization automatically
CTimePeriod duration(75);  // 75 seconds becomes 1m 15s
std::cout << duration.toString() << std::endl;  // "1m 15s"

// Accumulating durations
CTimePeriod total;
std::vector<std::string> durations = {"1h", "30m", "45s"};
for (const auto& d : durations) {
    CTimePeriod current(d);
    total = CTimePeriod(total.duration() + current.duration());
}
std::cout << total.toString() << std::endl;  // "1h 30m 45s"

// Parse large numbers
CTimePeriod huge("999h 123456s");
std::cout << huge.toString() << std::endl;  // Normalized output
```

## Parser Architecture

### Scanner (Tokenizer)

The `CScanner` class handles the low-level tokenization of input strings:

- **Token Recognition**: Identifies numbers and unit suffixes
- **Flexible Parsing**: Handles both "5m" and "5 minutes" formats
- **Accumulation**: Combines multiple instances of the same unit (e.g., "5m 10m" = "15m")
- **Default Units**: Numbers without units default to minutes

#### Scanner Behavior

```cpp
// Internal scanner usage (normally automatic)
CTimePeriod::TokenHolder tokens = {
    {"s", 1L}, {"seconds", 1L},
    {"m", 60L}, {"minutes", 60L},
    {"h", 3600L}, {"hours", 3600L},
    // ... more units
};

CTimePeriod::CScanner scanner("2h 30m 15s", tokens);
auto result = scanner.ScanTokens();
// result contains: {3600: 2, 60: 30, 1: 15}
```

### Parser (Duration Calculator)

The parser converts scanner results into `std::chrono::seconds`:

- **Multiplier Application**: Applies time unit multipliers to values
- **Accumulation**: Sums all components into total seconds
- **Normalization**: Breaks down total seconds into days, hours, minutes, seconds

#### Parse Process

1. **Input**: `"2h 30m 15s"`
2. **Tokenization**: `{3600: 2, 60: 30, 1: 15}`
3. **Calculation**: `2×3600 + 30×60 + 15×1 = 9015 seconds`
4. **Normalization**: `9015s → 0d 2h 30m 15s`

## Building from Source

### Prerequisites

- C++17 compatible compiler
- CMake 3.14 or later
- Git

### Build Options

```bash
# Configure with options
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DTIMEDURATION_BUILD_TESTS=ON \
  -DTIMEDURATION_BUILD_EXAMPLES=ON

# Build
cmake --build build --config Release

# Run tests
cd build && ctest --output-on-failure

# Install
cmake --install build --prefix /usr/local
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `TIMEDURATION_BUILD_TESTS` | `OFF` | Build unit tests |
| `TIMEDURATION_BUILD_EXAMPLES` | `OFF` | Build example programs |
| `TIMEDURATION_DOWNLOAD_GTEST` | `ON` | Auto-download Google Test if not found |

## Testing

The library includes comprehensive unit tests covering:

- **Scanner functionality**: Tokenization and parsing logic
- **Parser accuracy**: Duration calculation and normalization
- **Edge cases**: Zero durations, large numbers, malformed input
- **API completeness**: All public methods and operators
- **Integration**: Round-trip conversions and complex scenarios

Run tests with:

```bash
# Build with tests enabled
cmake -B build -DTIMEDURATION_BUILD_TESTS=ON
cmake --build build

# Run all tests
cd build && ctest --output-on-failure

# Run specific test patterns
cd build && ctest -R "Scanner" --verbose
```

## Performance Considerations

- **Header-only**: No runtime linking overhead
- **Parse caching**: Consider caching parsed durations for repeated use
- **Memory efficient**: Uses standard integers and chrono types
- **Stack allocated**: No dynamic memory allocation during normal operation

## Error Handling

The library follows a simple error handling strategy:

- **Malformed input**: Returns zero duration
- **Unknown units**: Ignored during parsing
- **Overflow**: Undefined behavior for extremely large values (same as standard integers)

```cpp
// These all result in zero or partial parsing
CTimePeriod invalid1("");           // Empty string → 0s
CTimePeriod invalid2("invalid");    // No numbers → 0s
CTimePeriod partial("5h invalid");  // Partial parse → 5h 0m 0s
```

## Integration Examples

### With std::chrono

```cpp
#include <chrono>
#include <thread>

CTimePeriod delay("5s");
std::this_thread::sleep_for(delay.duration());
```

### With Database Queries

```cpp
CTimePeriod retention("30d");
std::string query = "DELETE FROM logs WHERE created_at < NOW() - " + 
                   retention.asSqlInterval();
```

### Configuration Files

```cpp
#include <fstream>
#include <sstream>

std::ifstream config("config.txt");
std::string line;
while (std::getline(config, line)) {
    if (line.starts_with("timeout=")) {
        std::string duration_str = line.substr(8);
        CTimePeriod timeout(duration_str);
        // Use timeout.duration() for network operations
    }
}
```

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes and add tests
4. Ensure all tests pass: `cmake --build build && cd build && ctest`
5. Submit a pull request

### Code Style

- Follow the existing code style
- Use meaningful variable names
- Add unit tests for new functionality
- Update documentation for API changes

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Changelog

## FAQ

**Q: Why use this instead of std::chrono parsing?**
A: std::chrono doesn't provide built-in parsing for human-readable formats like "2h 30m". This library bridges that gap.

**Q: Are months and years exact?**
A: For simplicity, months are 28 days and years are 365 days. For precise calendar arithmetic, use a dedicated date/time library.

**Q: Can I extend the supported units?**
A: Currently, units are hardcoded. Future versions may support custom unit definitions.

**Q: What about negative durations?**
A: Negative durations are not currently supported. All parsed values are treated as positive.

**Q: Is it thread-safe?**
A: Yes, the library is thread-safe for read operations. Multiple threads can safely parse durations simultaneously.