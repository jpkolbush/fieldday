#include <vector>
#include <cstdint>
#include <utility>

namespace FieldDay {
    class Schedule {
    private:
        size_t _dimension;
        std::vector<std::vector<std::pair<size_t, size_t>>> _data;
        std::pair<size_t, size_t> _complete_up_to;
        // Bit i set => team i has already appeared in this row/column.
        std::vector<uint64_t> _used_teams_by_row;
        std::vector<uint64_t> _used_teams_by_col;
        // Bit j set on _used_pairings[i] => teams i and j have already played.
        std::vector<uint64_t> _used_pairings;

        std::pair<size_t, size_t> next_position() const;

        void countCompletionsFrom(size_t& count);
        void collectCompletionsFrom(std::vector<Schedule>& out);

    public:
        Schedule(size_t dim);
        size_t dimension();

        bool complete();

        std::vector<std::vector<std::pair<size_t, size_t>>> data();

        void update(std::pair<size_t, size_t> position, std::pair<size_t, size_t> matchup);
        void undo(std::pair<size_t, size_t> position, std::pair<size_t, size_t> matchup, std::pair<size_t, size_t> prev_complete);

        static std::vector<Schedule> GenerateAllSchedules(size_t dim);
        static size_t GenerateAllSchedulesCount(size_t dim);
    };
}
