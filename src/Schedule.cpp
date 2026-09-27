#include "Schedule.h"

namespace FieldDay {
    Schedule::Schedule(size_t dimension)
    {
        _dimension = dimension;
        _data = std::vector<std::vector<std::pair<size_t,size_t>>>(dimension);
        _used_teams_by_row = std::vector<std::vector<bool>>(dimension, std::vector<bool>(2 * dimension, false));
        _used_teams_by_col = std::vector<std::vector<bool>>(dimension, std::vector<bool>(2 * dimension, false));
        _used_pairings = std::vector<std::vector<bool>>(2 * dimension, std::vector<bool>(2 * dimension, false));

        std::vector<std::pair<size_t,size_t>> first_row(dimension);

        // Add the entries of the first row
        for (size_t i = 0; i < dimension; i++)
        {
            auto pair = std::pair<size_t, size_t>(2*i , 2*i + 1);
            first_row[i] = pair;
            _used_pairings[pair.first][pair.second] = true;

            _used_teams_by_col[i][pair.first] = true;
            _used_teams_by_col[i][pair.second] = true;
        }
        _data[0] = first_row;

        for (size_t i = 0; i < 2 * dimension; i++)
        {
            _used_teams_by_row[0][i] = true;
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
        _used_pairings[matchup.first][matchup.second] = true;

        _used_teams_by_row[position.first][matchup.first] = true;
        _used_teams_by_row[position.first][matchup.second] = true;
        _used_teams_by_col[position.second][matchup.first] = true;
        _used_teams_by_col[position.second][matchup.second] = true;
        _complete_up_to = position;
    }

    void Schedule::undo(std::pair<size_t, size_t> position, std::pair<size_t, size_t> matchup, std::pair<size_t, size_t> prev_complete)
    {
        _data[position.first].pop_back();
        _used_pairings[matchup.first][matchup.second] = false;

        _used_teams_by_row[position.first][matchup.first] = false;
        _used_teams_by_row[position.first][matchup.second] = false;
        _used_teams_by_col[position.second][matchup.first] = false;
        _used_teams_by_col[position.second][matchup.second] = false;
        _complete_up_to = prev_complete;
    }

    static std::vector<size_t> get_valid_teams(const std::vector<bool>& used_in_row,
                                                const std::vector<bool>& used_in_col)
    {
        std::vector<size_t> ret;
        for (size_t i = 0; i < used_in_row.size(); i++)
        {
            if (!used_in_row[i] && !used_in_col[i])
            {
                ret.push_back(i);
            }
        }
        return ret;
    }

    // Shared move-enumeration core used by both counting and collecting traversals.
    // `visit` is invoked with each legal (team, team) matchup for `next_complete`;
    // the caller applies update()/recurses/undo() around it.
    template <typename Visit>
    static void forEachNextMove(const std::vector<std::vector<bool>>& used_teams_by_row,
                                const std::vector<std::vector<bool>>& used_teams_by_col,
                                const std::vector<std::vector<bool>>& used_pairings,
                                std::pair<size_t, size_t> next_complete,
                                Visit&& visit)
    {
        auto valid_teams = get_valid_teams(used_teams_by_row[next_complete.first], used_teams_by_col[next_complete.second]);

        for (auto it1 = valid_teams.begin(); it1 != valid_teams.end(); ++it1)
        {
            for (auto it2 = std::next(it1); it2 != valid_teams.end(); ++it2)
            {
                auto matchup = std::pair<size_t, size_t>(*it1, *it2);
                if (used_pairings[matchup.first][matchup.second])
                {
                    continue;
                }

                // Standard form: within row 1, always place the lower-numbered
                // partner of a first-row pair before its partner.
                if (next_complete.first == 1 && matchup.first % 2 == 1 &&
                    !used_teams_by_row[1][matchup.first - 1])
                {
                    continue;
                }
                if (next_complete.first == 1 && matchup.second % 2 == 1 &&
                    !used_teams_by_row[1][matchup.second - 1])
                {
                    continue;
                }

                visit(matchup);
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
        forEachNextMove(_used_teams_by_row, _used_teams_by_col, _used_pairings, next_complete,
            [&](std::pair<size_t, size_t> matchup)
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
        forEachNextMove(_used_teams_by_row, _used_teams_by_col, _used_pairings, next_complete,
            [&](std::pair<size_t, size_t> matchup)
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
