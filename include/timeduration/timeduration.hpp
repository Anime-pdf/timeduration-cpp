#ifndef TIMEDURATION_HPP
#define TIMEDURATION_HPP

#include <chrono>
#include <map>
#include <string>

namespace timeduration {

/**
 * @brief CTimePeriod class represents a time duration with parsing capabilities
 *
 * This class allows parsing of human-readable time durations (like "5h 30m") into
 * chrono::seconds and provides methods to access the parsed components.
 */
class CTimePeriod final {
public:
    using TokenHolder = std::map<std::string, int64_t>; // <literal, multiplier> (e.g. <"minutes", 60>)
    using ResultHolder = std::map<int64_t, int64_t>; // <multiplier, value> (e.g. <60, 15>)

    /**
     * @brief Scanner class that handles the tokenization and parsing of time duration strings
     */
    class CScanner final {
        const std::string m_Source;
        TokenHolder m_Tokens;
        ResultHolder m_Result;

        int m_Start = 0;
        int m_Current = 0;

        [[nodiscard]] bool AtEnd() const {
            return m_Current >= m_Source.length();
        }

        char Advance() {
            return m_Source[m_Current++];
        }

        [[nodiscard]] char Peek() const {
            if (AtEnd()) return '\0';
            return m_Source[m_Current];
        }

        void ScanToken() {
            if (const char c = Advance(); isdigit(c)) {
                while (isdigit(Peek())) Advance();
                const std::string Value = m_Source.substr(m_Start, m_Current - m_Start);
                const int Offset = m_Current;

                while (isalpha(Peek())) Advance();
                const std::string Literal = m_Source.substr(Offset, m_Current - Offset);

                if (Literal.empty())
                    AddValue(60, std::stoll(Value));
                else
                    AddValue(Literal, std::stoll(Value));
            }
        }

        void AddValue(const std::string_view Literal, const int64_t Value) {
            if (const auto TokIt = m_Tokens.find(Literal.data()); TokIt != m_Tokens.end())
                AddValue(TokIt->second, Value);
        }

        void AddValue(int64_t Multiplier, int64_t Value) {
            if (const auto ResIt = m_Result.find(Multiplier); ResIt != m_Result.end())
                ResIt->second += Value;
            else
                m_Result.emplace(Multiplier, Value);
        }

    public:
        explicit CScanner(const std::string_view Source, TokenHolder Multipliers) : m_Source(Source),
            m_Tokens(std::move(Multipliers)) {
        }

        [[nodiscard]] ResultHolder ScanTokens() {
            while (!AtEnd()) {
                m_Start = m_Current;
                ScanToken();
            }
            return m_Result;
        }
    };

private:
    typedef std::chrono::duration<int64_t, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>> chrono_day; // added in C++20

    std::chrono::seconds m_TotalDuration{0};
    int64_t m_Days{0};
    int64_t m_Hours{0};
    int64_t m_Minutes{0};
    int64_t m_Seconds{0};

    void Validate() {
        // Store the normalized values
        auto TotalSeconds = m_TotalDuration.count();

        m_Days = TotalSeconds / 86400L;
        TotalSeconds %= 86400L;

        m_Hours = TotalSeconds / 3600L;
        TotalSeconds %= 3600L;

        m_Minutes = TotalSeconds / 60L;
        m_Seconds = TotalSeconds % 60L;
    }

public:
    /**
     * @brief Construct a CTimePeriod with explicit duration components
     *
     * @param seconds Number of seconds
     * @param minutes Number of minutes
     * @param hours Number of hours
     * @param days Number of days
     */
    explicit CTimePeriod(const int64_t seconds = 0, const int64_t minutes = 0, const int64_t hours = 0,
                         const int64_t days = 0) {
        m_TotalDuration = std::chrono::seconds{seconds} +
                          std::chrono::minutes{minutes} +
                          std::chrono::hours{hours} +
                          chrono_day{days};
        Validate();
    }

    /**
     * @brief Construct a CTimePeriod by parsing a string
     *
     * @param from String representation of time duration (e.g., "5h 30m 10s")
     */
    explicit CTimePeriod(const std::string_view from) {
        m_TotalDuration = Parse(from);
        Validate();
    }

    /**
     * @brief Construct a CTimePeriod from std::chrono::seconds
     *
     * @param duration Duration in seconds
     */
    explicit CTimePeriod(const std::chrono::seconds duration) {
        m_TotalDuration = duration;
        Validate();
    }

    /**
     * @brief Parse a string into chrono::seconds
     *
     * @param from String representation of time duration
     * @return std::chrono::seconds Parsed duration in seconds
     */
    [[nodiscard]] static std::chrono::seconds Parse(const std::string_view from) {
        std::chrono::seconds TotalDuration{0};
        CScanner Scanner(from,
                         {
                             {"s", 1L}, {"seconds", 1L},
                             {"m", 60L}, {"minutes", 60L},
                             {"h", 3600L}, {"hours", 3600L},
                             {"d", 86400L}, {"days", 86400L},
                             {"mo", 2419200L}, {"months", 2419200L},
                             {"y", 31536000L}, {"years", 31536000L},
                         });

        auto Result = Scanner.ScanTokens();
        for (auto [Multiplier, Value]: Result)
            TotalDuration += std::chrono::seconds(Multiplier * Value);

        return TotalDuration;
    }

    /**
     * @brief Factory method to create a CTimePeriod from a string
     *
     * @param from String representation of time duration
     * @return CTimePeriod object
     */
    [[nodiscard]] static CTimePeriod ParseFactory(const std::string_view from) {
        return CTimePeriod(Parse(from));
    }

    /**
     * @brief Generate SQL interval string representation
     *
     * @return std::string SQL compatible interval string
     */
    [[nodiscard]] std::string asSqlInterval() const {
        return "interval " + std::to_string(m_TotalDuration.count()) + " second";
    }

    [[nodiscard]] constexpr std::chrono::seconds duration() const noexcept { return m_TotalDuration; }
    [[nodiscard]] constexpr int64_t days() const noexcept { return m_Days; }
    [[nodiscard]] constexpr int64_t hours() const noexcept { return m_Hours; }
    [[nodiscard]] constexpr int64_t minutes() const noexcept { return m_Minutes; }
    [[nodiscard]] constexpr int64_t seconds() const noexcept { return m_Seconds; }

    /**
     * @brief Format as human-readable string
     *
     * @return std::string Formatted string (e.g., "2d 5h 30m 15s")
     */
    [[nodiscard]] std::string toString() const {
        std::string result;
        if (m_Days > 0) result += std::to_string(m_Days) + "d ";
        if (m_Hours > 0) result += std::to_string(m_Hours) + "h ";
        if (m_Minutes > 0) result += std::to_string(m_Minutes) + "m ";
        if (m_Seconds > 0 || result.empty()) result += std::to_string(m_Seconds) + "s";
        if (result.ends_with(' '))
            result.erase(result.end() - 1);
        return result;
    }

    /**
     * @brief Check if duration is zero
     *
     * @return true if duration is zero
     */
    [[nodiscard]] constexpr bool isZero() const noexcept {
        return m_TotalDuration.count() == 0;
    }

    // Comparison operators

    friend bool operator==(const CTimePeriod& Lhs, const CTimePeriod& Rhs)
    {
        return Lhs.m_TotalDuration == Rhs.m_TotalDuration;
    }

    friend bool operator!=(const CTimePeriod& Lhs, const CTimePeriod& Rhs)
    {
        return !(Lhs == Rhs);
    }

    friend bool operator<(const CTimePeriod& Lhs, const CTimePeriod& Rhs)
    {
        return Lhs.m_TotalDuration < Rhs.m_TotalDuration;
    }

    friend bool operator<=(const CTimePeriod& Lhs, const CTimePeriod& Rhs)
    {
        return Lhs.m_TotalDuration <= Rhs.m_TotalDuration;
    }

    friend bool operator>(const CTimePeriod& Lhs, const CTimePeriod& Rhs)
    {
        return Lhs.m_TotalDuration > Rhs.m_TotalDuration;
    }

    friend bool operator>=(const CTimePeriod& Lhs, const CTimePeriod& Rhs)
    {
        return Lhs.m_TotalDuration >= Rhs.m_TotalDuration;
    }
};

} // namespace timeduration

#endif // TIMEDURATION_HPP
