#include <timeduration/timeduration.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace timeduration;

void print_header(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

// Example 1: Configuration File Parsing
void demonstrate_config_parsing() {
    print_header("CONFIGURATION FILE PARSING");

    // Simulate reading from a config file
    std::vector<std::string> config_lines = {
        "# Application timeouts",
        "connection_timeout=30s",
        "read_timeout=5m",
        "session_timeout=2h",
        "backup_interval=1d",
        "log_rotation=7d",
        "cache_expiry=1h 30m",
        "# Database settings",
        "db_connection_pool_timeout=10s",
        "query_timeout=2m 30s"
    };

    std::map<std::string, CTimePeriod> timeouts;

    std::cout << "Parsing configuration:\n" << std::endl;

    for (const auto& line : config_lines) {
        if (line.empty() || line[0] == '#') {
            std::cout << line << std::endl;
            continue;
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            try {
                CTimePeriod duration(value);
                timeouts[key] = duration;

                std::cout << std::left << std::setw(30) << key + ":"
                          << std::setw(15) << value
                          << "-> " << duration.toString()
                          << " (" << duration.duration().count() << " seconds)"
                          << std::endl;
            } catch (...) {
                std::cout << "Error parsing: " << line << std::endl;
            }
        }
    }

    std::cout << "\nUsing parsed timeouts:" << std::endl;
    if (timeouts.find("connection_timeout") != timeouts.end()) {
        auto timeout = timeouts["connection_timeout"];
        std::cout << "Connection timeout set to " << timeout.duration().count() << " seconds" << std::endl;
    }
}

// Example 2: Duration Arithmetic and Accumulation
void demonstrate_duration_arithmetic() {
    print_header("DURATION ARITHMETIC & ACCUMULATION");

    // Simulate task durations
    std::vector<std::pair<std::string, std::string>> tasks = {
        {"Database backup", "45m"},
        {"Log analysis", "1h 20m"},
        {"System cleanup", "30m"},
        {"Report generation", "2h 15m"},
        {"File compression", "1h 5m"},
        {"Data validation", "40m"}
    };

    std::cout << "Task Duration Analysis:\n" << std::endl;
    std::cout << std::left << std::setw(25) << "Task"
              << std::setw(15) << "Duration"
              << std::setw(15) << "Seconds"
              << "Formatted" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    CTimePeriod total_duration;

    for (const auto& [task_name, duration_str] : tasks) {
        CTimePeriod task_duration(duration_str);

        std::cout << std::setw(25) << task_name
                  << std::setw(15) << duration_str
                  << std::setw(15) << task_duration.duration().count()
                  << task_duration.toString() << std::endl;

        // Accumulate total duration
        auto new_total_seconds = total_duration.duration() + task_duration.duration();
        total_duration = CTimePeriod(new_total_seconds);
    }

    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::setw(25) << "TOTAL:"
              << std::setw(15) << ""
              << std::setw(15) << total_duration.duration().count()
              << total_duration.toString() << std::endl;

    // Find longest and shortest tasks
    auto longest = std::max_element(tasks.begin(), tasks.end(),
        [](const auto& a, const auto& b) {
            return CTimePeriod(a.second).duration() < CTimePeriod(b.second).duration();
        });

    auto shortest = std::min_element(tasks.begin(), tasks.end(),
        [](const auto& a, const auto& b) {
            return CTimePeriod(a.second).duration() < CTimePeriod(b.second).duration();
        });

    std::cout << "\nAnalysis:" << std::endl;
    std::cout << "  Longest task: " << longest->first << " (" << longest->second << ")" << std::endl;
    std::cout << "  Shortest task: " << shortest->first << " (" << shortest->second << ")" << std::endl;
    std::cout << "  Average duration: " << CTimePeriod(std::chrono::seconds(
        total_duration.duration().count() / tasks.size())).toString() << std::endl;
}

// Example 3: Sorting and Ranking
void demonstrate_sorting_ranking() {
    print_header("SORTING & RANKING BY DURATION");

    struct Process {
        std::string name;
        CTimePeriod runtime;
        int priority;

        Process(std::string n, const std::string& duration, int p)
            : name(std::move(n)), runtime(duration), priority(p) {}
    };

    std::vector<Process> processes = {
        {"WebServer", "5h 30m", 1},
        {"DatabaseCleanup", "2h 45m", 3},
        {"LogRotation", "15m", 2},
        {"BackupService", "1h 20m", 2},
        {"MonitoringAgent", "30s", 1},
        {"DataSync", "3h 15m", 3},
        {"CacheWarming", "45m", 2}
    };

    std::cout << "Original process list:" << std::endl;
    std::cout << std::left << std::setw(20) << "Process"
              << std::setw(15) << "Runtime"
              << std::setw(10) << "Priority"
              << "Total Seconds" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const auto& proc : processes) {
        std::cout << std::setw(20) << proc.name
                  << std::setw(15) << proc.runtime.toString()
                  << std::setw(10) << proc.priority
                  << proc.runtime.duration().count() << std::endl;
    }

    // Sort by runtime (longest first)
    std::sort(processes.begin(), processes.end(),
        [](const Process& a, const Process& b) {
            return a.runtime > b.runtime;
        });

    std::cout << "\nSorted by runtime (longest first):" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (size_t i = 0; i < processes.size(); ++i) {
        const auto& proc = processes[i];
        std::cout << "#" << (i + 1) << " " << std::setw(18) << proc.name
                  << std::setw(15) << proc.runtime.toString()
                  << std::setw(10) << proc.priority
                  << proc.runtime.duration().count() << std::endl;
    }

    // Group by priority and show totals
    std::map<int, std::vector<Process*>> by_priority;
    for (auto& proc : processes) {
        by_priority[proc.priority].push_back(&proc);
    }

    std::cout << "\nGrouped by priority:" << std::endl;
    for (const auto& [priority, procs] : by_priority) {
        CTimePeriod total_for_priority;

        std::cout << "\nPriority " << priority << ":" << std::endl;
        for (const auto* proc : procs) {
            std::cout << "  " << std::setw(20) << proc->name
                      << proc->runtime.toString() << std::endl;

            auto new_total = total_for_priority.duration() + proc->runtime.duration();
            total_for_priority = CTimePeriod(new_total);
        }
        std::cout << "  Total: " << total_for_priority.toString()
                  << " (" << procs.size() << " processes)" << std::endl;
    }
}

// Example 4: Integration with std::chrono and threading
void demonstrate_threading_integration() {
    print_header("THREADING & std::chrono INTEGRATION");

    struct Task {
        std::string name;
        CTimePeriod delay;

        Task(std::string n, const std::string& d) : name(std::move(n)), delay(d) {}
    };

    std::vector<Task> tasks = {
        {"Initialize system", "2s"},
        {"Load configuration", "1s"},
        {"Connect to database", "3s"},
        {"Start monitoring", "1s"},
        {"Ready for requests", "500ms"}  // Note: ms not supported, will be 0
    };

    std::cout << "Simulating startup sequence with delays:\n" << std::endl;

    auto start_time = std::chrono::steady_clock::now();

    for (const auto& task : tasks) {
        std::cout << "Executing: " << task.name
                  << " (delay: " << task.delay.toString() << ")" << std::flush;

        // Use the duration directly with std::this_thread::sleep_for
        if (!task.delay.isZero()) {
            std::this_thread::sleep_for(task.delay.duration());
        }

        std::cout << " ✓" << std::endl;
    }

    auto end_time = std::chrono::steady_clock::now();
    auto total_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        end_time - start_time);

    std::cout << "\nStartup completed in " << total_elapsed.count() << " seconds" << std::endl;

    // Calculate expected vs actual time
    CTimePeriod expected_total;
    for (const auto& task : tasks) {
        auto new_total = expected_total.duration() + task.delay.duration();
        expected_total = CTimePeriod(new_total);
    }

    std::cout << "Expected total delay: " << expected_total.toString() << std::endl;
    std::cout << "Actual elapsed time: " << total_elapsed.count() << "s" << std::endl;
}

// Example 5: SQL Query Generation
void demonstrate_sql_integration() {
    print_header("SQL QUERY GENERATION");

    struct QueryTemplate {
        std::string description;
        std::string base_query;
        CTimePeriod time_range;
    };

    std::vector<QueryTemplate> queries = {
        {
            "Delete old logs",
            "DELETE FROM system_logs WHERE created_at < NOW() - ",
            CTimePeriod("30d")
        },
        {
            "Archive old sessions",
            "UPDATE user_sessions SET archived = true WHERE last_activity < NOW() - ",
            CTimePeriod("7d")
        },
        {
            "Clean temporary files",
            "DELETE FROM temp_files WHERE created_at < NOW() - ",
            CTimePeriod("2h")
        },
        {
            "Remove expired cache entries",
            "DELETE FROM cache_entries WHERE expires_at < NOW() - ",
            CTimePeriod("1h 30m")
        }
    };

    std::cout << "Generated SQL queries with time intervals:\n" << std::endl;

    for (const auto& query : queries) {
        std::cout << "-- " << query.description << std::endl;
        std::cout << query.base_query << query.time_range.asSqlInterval() << ";" << std::endl;
        std::cout << "-- Duration: " << query.time_range.toString()
                  << " (" << query.time_range.duration().count() << " seconds)" << std::endl;
        std::cout << std::endl;
    }
}

// Example 6: Performance Monitoring
void demonstrate_performance_monitoring() {
    print_header("PERFORMANCE MONITORING");

    struct PerformanceMetric {
        std::string operation;
        CTimePeriod target_time;
        CTimePeriod actual_time;

        [[nodiscard]] double performance_ratio() const {
            if (target_time.isZero()) return 0.0;
            return static_cast<double>(actual_time.duration().count()) /
                   static_cast<double>(target_time.duration().count());
        }

        [[nodiscard]] std::string status() const {
            double ratio = performance_ratio();
            if (ratio <= 0.8) return "EXCELLENT";
            else if (ratio <= 1.0) return "GOOD";
            else if (ratio <= 1.5) return "ACCEPTABLE";
            else return "NEEDS ATTENTION";
        }
    };

    std::vector<PerformanceMetric> metrics = {
        {"Database Query", CTimePeriod("2s"), CTimePeriod("1s 500ms")},  // 500ms will be 0
        {"File Upload", CTimePeriod("30s"), CTimePeriod("25s")},
        {"Data Processing", CTimePeriod("5m"), CTimePeriod("7m 30s")},
        {"Report Generation", CTimePeriod("2m"), CTimePeriod("1m 45s")},
        {"Cache Refresh", CTimePeriod("10s"), CTimePeriod("15s")},
        {"Backup Operation", CTimePeriod("1h"), CTimePeriod("45m")}
    };

    std::cout << "Performance Analysis Report:\n" << std::endl;
    std::cout << std::left << std::setw(20) << "Operation"
              << std::setw(12) << "Target"
              << std::setw(12) << "Actual"
              << std::setw(8) << "Ratio"
              << "Status" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto& metric : metrics) {
        std::cout << std::setw(20) << metric.operation
                  << std::setw(12) << metric.target_time.toString()
                  << std::setw(12) << metric.actual_time.toString()
                  << std::setw(8) << std::fixed << std::setprecision(2)
                  << metric.performance_ratio()
                  << metric.status() << std::endl;
    }

    // Summary statistics
    double avg_ratio = 0.0;
    int good_performance = 0;

    for (const auto& metric : metrics) {
        avg_ratio += metric.performance_ratio();
        if (metric.performance_ratio() <= 1.0) {
            good_performance++;
        }
    }
    avg_ratio /= static_cast<double>(metrics.size());

    std::cout << std::string(70, '-') << std::endl;
    std::cout << "Summary:" << std::endl;
    std::cout << "  Average performance ratio: " << std::fixed << std::setprecision(2)
              << avg_ratio << std::endl;
    std::cout << "  Operations meeting target: " << good_performance
              << "/" << metrics.size() << std::endl;
    std::cout << "  Success rate: " << std::fixed << std::setprecision(1)
              << (static_cast<double>(good_performance) / static_cast<double>(metrics.size()) * 100) << "%" << std::endl;
}

int main() {
    std::cout << "TimeDuration Library - Advanced Usage Examples" << std::endl;
    std::cout << "=============================================" << std::endl;

    demonstrate_config_parsing();
    demonstrate_duration_arithmetic();
    demonstrate_sorting_ranking();
    demonstrate_threading_integration();
    demonstrate_sql_integration();
    demonstrate_performance_monitoring();

    return 0;
}