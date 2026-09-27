#include "Schedule.h"

namespace FieldDay {
    Schedule::Schedule(size_t dimension)
    {
        _dimension = dimension;
        _data = std::vector<std::vector<std::pair<size_t,size_t>>>(dimension);
        _used_teams_by_row = std::vector<uint64_t>(dimension, 0);
        _used_teams_by_col = std::vector<uint64_t>(dimension, 0);
        _used_pairings = std::vector<uint64_t>(2 * dimension, 0);

        std::vector<std::pair<size_t,size_t>> first_row(dimension);

        // Add the entries of the first row
        for (size_t i = 0; i < dimension; i++)
        {
            auto pair = std::pair<size_t, size_t>(2*i , 2*i + 1);
            first_row[i] = pair;
            _used_pairings[pair.first] |= (1ULL << pair.second);

            _used_teams_by_col[i] |= (1ULL << pair.first) | (1ULL << pair.second);
        }
        _data[0] = first_row;

        for (size_t i = 0; i < 2 * dimension; i++)
        {
            _used_teams_by_row[0] |= (1ULL << i);
        }

        _complete_up_to = std::pair<size_t,size_t>(0, dimension-1);
    }

    size_t Schedule::dimension()
    {
        return _dimension;
    }

    bool Schedule::complete()
    {
        return _complete_up_to.first ==  _dimension -1
            &&  _complete_up_to.second ==  _dimension -1;
    }

    std::vector<std::vector<std::pair<size_t, size_t>>> Schedule::data()
    {
        return _data;
    }

    std::pair<size_t, size_t> Schedule::next_position() const
    {
        auto next_complete = std::pair<size_t,size_t>(_complete_up_to.first, _complete_up_to.second + 1);
        if (next_complete.second == _dimension)
        {
            next_complete.first = next_complete.first + 1;
            next_complete.second = 0;
        }
        return next_complete;
    }

    void Schedule::update(std::pair<size_t,size_t> position, std::pair<size_t, size_t> matchup)
    {
        _data[position.first].push_back(matchup);
        _used_pairings[matchup.first] |= (1ULL << matchup.second);

        uint64_t bits = (1ULL << matchup.first) | (1ULL << matchup.second);
        _used_teams_by_row[position.first] |= bits;
        _used_teams_by_col[position.second] |= bits;
        _complete_up_to = position;
    }

    void Schedule::undo(std::pair<size_t, size_t> position, std::pair<size_t, size_t> matchup, std::pair<size_t, size_t> prev_complete)
    {
        _data[position.first].pop_back();
        _used_pairings[matchup.first] &= ~(1ULL << matchup.second);

        uint64_t bits = (1ULL << matchup.first) | (1ULL << matchup.second);
        _used_teams_by_row[position.first] &= ~bits;
        _used_teams_by_col[position.second] &= ~bits;
        _complete_up_to = prev_complete;
    }

    // Shared move-enumeration core used by both counting and collecting traversals.
    // `Visit` is called with (position, matchup) already applied via update(); it must
    // recurse (or record) and the caller here handles the matching undo().
    template <typename Visit>
    static void forEachNextMove(Schedule& s, size_t dimension,
                                const std::vector<uint64_t>& used_teams_by_row,
                                const std::vector<uint64_t>& used_teams_by_col,
                                const std::vector<uint64_t>& used_pairings,
                                std::pair<size_t, size_t> next_complete,
                                Visit&& visit)
    {
        uint64_t full_mask = (dimension * 2 >= 64) ? ~0ULL : ((1ULL << (2 * dimension)) - 1);
        uint64_t avail = full_mask & ~(used_teams_by_row[next_complete.first] | used_teams_by_col[next_complete.second]);

        for (size_t t1 = 0; t1 < 2 * dimension; t1++)
        {
            if (!((avail >> t1) & 1ULL)) continue;
            for (size_t t2 = t1 + 1; t2 < 2 * dimension; t2++)
            {
                if (!((avail >> t2) & 1ULL)) continue;
                if ((used_pairings[t1] >> t2) & 1ULL) continue;

                // Standard form: within row 1, always place the lower-numbered
                // partner of a first-row pair before its partner.
                if (next_complete.first == 1 && t1 % 2 == 1 &&
                    !((used_teams_by_row[1] >> (t1 - 1)) & 1ULL))
                {
                    continue;
                }
                if (next_complete.first == 1 && t2 % 2 == 1 &&
                    !((used_teams_by_row[1] >> (t2 - 1)) & 1ULL))
                {
                    continue;
                }

                visit(std::pair<size_t, size_t>(t1, t2));
            }
        }
    }

    void Schedule::countCompletionsFrom(size_t& count)
    {
        if (complete())
        {
            count++;
            return;
        }

        auto next_complete = next_position();
        forEachNextMove(*this, _dimension, _used_teams_by_row, _used_teams_by_col, _used_pairings,
            next_complete, [&](std::pair<size_t, size_t> matchup)
        {
            auto prev_complete = _complete_up_to;
            update(next_complete, matchup);
            countCompletionsFrom(count);
            undo(next_complete, matchup, prev_complete);
        });
    }

    void Schedule::collectCompletionsFrom(std::vector<Schedule>& out)
    {
        if (complete())
        {
            out.push_back(*this);
            return;
        }

        auto next_complete = next_position();
        forEachNextMove(*this, _dimension, _used_teams_by_row, _used_teams_by_col, _used_pairings,
            next_complete, [&](std::pair<size_t, size_t> matchup)
        {
            auto prev_complete = _complete_up_to;
            update(next_complete, matchup);
            collectCompletionsFrom(out);
            undo(next_complete, matchup, prev_complete);
        });
    }

    std::vector<Schedule> Schedule::GenerateAllSchedules(size_t dim)
    {
        Schedule root(dim);
        std::vector<Schedule> ret;
        root.collectCompletionsFrom(ret);
        return ret;
    }

    size_t Schedule::GenerateAllSchedulesCount(size_t dim)
    {
        Schedule root(dim);
        size_t count = 0;
        root.countCompletionsFrom(count);
        return count;
    }
}
