#include <vector>
#include <unordered_set>

namespace FieldDay {
    class Schedule {
    private:
        size_t _dimension;
        std::vector<std::vector<std::pair<size_t, size_t>>> _data;
        std::pair<size_t, size_t> _complete_up_to;
        // std::vector<std::unordered_set<size_t>> _unused_teams_by_row;
        std::vector<std::vector<bool>> _used_teams_by_row;
        // std::vector<std::unordered_set<size_t>> _unused_teams_by_col;
        std::vector<std::vector<bool>> _used_teams_by_col;
        std::vector<std::unordered_set<size_t>> _used_pairings;

    public:
        Schedule(size_t dim);
        size_t dimension();

        bool complete();

        std::vector<std::vector<std::pair<size_t, size_t>>> data();

        std::vector<Schedule> NextValidSchedules();

        void update(std::pair<size_t,size_t> position, std::pair<size_t, size_t> matchup);

        static std::vector<Schedule> GenerateAllSchedules(size_t dim);
        static size_t GenerateAllSchedulesCount(size_t dim);

        

        

    };
}