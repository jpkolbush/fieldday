#include "Schedule.h"

#include <stack>
#include <numeric>
#include <set>
#include <iostream>

namespace FieldDay {
    Schedule::Schedule(size_t dimension)
    {
        _dimension = dimension;
        _data = std::vector<std::vector<std::pair<size_t,size_t>>>(dimension);
        // _unused_teams_by_col = std::vector<std::unordered_set<size_t>>(dimension);
        // _unused_teams_by_row = std::vector<std::unordered_set<size_t>>(dimension);
        _used_teams_by_row = std::vector<std::vector<bool>>(dimension);
        _used_teams_by_col = std::vector<std::vector<bool>>(dimension);
        _used_pairings = std::vector<std::unordered_set<size_t>>(2*dimension);

        std::vector<std::pair<size_t,size_t>> first_row(dimension);

        // Add the entries of the first row
        for(auto i = 0; i < dimension; i++)
        {
            auto pair = std::pair<size_t, size_t>(2*i , 2*i + 1);
            first_row[i] = pair;
            _used_pairings[pair.first] = std::unordered_set<size_t> { pair.second};
            _used_pairings[pair.second] = std::unordered_set<size_t> { };

            // Setup the unused teams by col sets
            // auto unused_teams_col = std::vector<size_t>(2*dimension - 2);
            _used_teams_by_col[i] = std::vector<bool>(2 * dimension, false);
            // std::cout << _used_teams_by_col.size() << std::endl;
            _used_teams_by_col[i][2*i] = true;
            _used_teams_by_col[i][2*i+1] = true;
        
            
        }
        _data[0] = first_row;

        // Setup the unused teams by row sets
        // _unused_teams_by_row.push_back(std::unordered_set<size_t>());
        _used_teams_by_row[0] = std::vector<bool>(2 * dimension, true);

        for (auto i = 1; i < dimension; i++)
        {
            _used_teams_by_row[i] = std::vector<bool>(2 * dimension, false);
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

    // std::set<size_t> intersect(const std::unordered_set<size_t>& a,
    //                                   const std::unordered_set<size_t>& b) {
    //     // Iterate over the smaller set, look up in the larger one.
    //     const auto& smaller = (a.size() <= b.size()) ? a : b;
    //     const auto& larger  = (a.size() <= b.size()) ? b : a;

    //     std::set<size_t> result;

    //     for (size_t val : smaller) {
    //         if (larger.find(val) != larger.end()) {
    //             result.insert(val);
    //         }
    //     }

    //     return result;
    // }

    std::vector<size_t> get_valid_teams(const std::vector<bool>& used_in_row,
                                      const std::vector<bool>& used_in_col) {
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

    std::vector<Schedule> Schedule::NextValidSchedules()
    {
        
        
        // Iterate the _complete_up_to
        auto next_complete = std::pair<size_t,size_t>(_complete_up_to.first, _complete_up_to.second + 1);
        if (next_complete.second == _dimension)
        {
            next_complete.first = next_complete.first + 1;
            next_complete.second = 0;
        }

        auto ret = std::vector<Schedule>();
        // std::cout << "Current index " << next_complete.first << " " << next_complete.second << std::endl;
        // std::cout << "Valid entries in row " << _unused_teams_by_row[next_complete.first].size() << std::endl;
        // std::cout << "Valid entries in col " << _unused_teams_by_col[next_complete.second].size() << std::endl;

        // Valid teams based on matchup, have not taken into account previously used matchups yet
        auto valid_teams = get_valid_teams(_used_teams_by_row[next_complete.first], _used_teams_by_col[next_complete.second]);
        // std::cout << "Valid teams count " << valid_teams.size() << std::endl;
        for (auto it1 = valid_teams.begin(); it1 != valid_teams.end(); ++it1) {

            for (auto it2 = std::next(it1); it2 != valid_teams.end(); ++it2) {
                auto matchup = std::pair<size_t, size_t>(*it1, *it2);
                if (_used_pairings[matchup.first].find(matchup.second) != _used_pairings[matchup.first].end())
                {
                    continue;
                }
                // Standard form, use the earlier entry first
                if (next_complete.first == 1 && matchup.first % 2 == 1 && 
                    !_used_teams_by_row[1][matchup.first - 1]
                )
                {
                    continue;
                }
                // Standard form, use the 
                if (next_complete.first == 1 && matchup.second % 2 == 1 && 
                    !_used_teams_by_row[1][matchup.second - 1])
                {
                    continue;
                }


                // New vaid pairing
                Schedule newSchedule = *this;
                newSchedule.update(next_complete, matchup);
                ret.push_back(newSchedule);
            }
        }

        
        
        // std::cout << "Number of next valid schdules " << ret.size() << std::endl;
        return ret;
    }

    void Schedule::update(std::pair<size_t,size_t> position, std::pair<size_t, size_t> matchup)
    {
        
        _data[position.first].push_back(matchup);
        _used_pairings[matchup.first].insert(matchup.second);
        _used_teams_by_row[position.first][matchup.first] = true;
        _used_teams_by_row[position.first][matchup.second] = true;
        _used_teams_by_col[position.second][matchup.first] = true;
        _used_teams_by_col[position.second][matchup.second] = true;
        _complete_up_to = position;
    }

    std::vector<Schedule> Schedule::GenerateAllSchedules(size_t dim)
    {

        auto scheduleStack = std::stack<Schedule>();
        scheduleStack.push(Schedule(dim));

        auto ret = std::vector<Schedule>();

        while (!scheduleStack.empty())
        {
            auto top = scheduleStack.top();
            scheduleStack.pop();
            auto nextSchedules = top.NextValidSchedules();
            for(size_t i = 0; i < nextSchedules.size(); i++)
            {
                if (nextSchedules[i].complete())
                {
                    ret.push_back(nextSchedules[i]);
                }
                else
                {
                    scheduleStack.push(nextSchedules[i]);
                }
            }
            
        }

        return ret;
    }

    size_t Schedule::GenerateAllSchedulesCount(size_t dim)
    {

        auto scheduleStack = std::stack<Schedule>();
        scheduleStack.push(Schedule(dim));

        auto ret = 0;

        while (!scheduleStack.empty())
        {
            auto top = scheduleStack.top();
            scheduleStack.pop();
            auto nextSchedules = top.NextValidSchedules();
            for(size_t i = 0; i < nextSchedules.size(); i++)
            {
                if (nextSchedules[i].complete())
                {
                    ret++;
                }
                else
                {
                    scheduleStack.push(nextSchedules[i]);
                }
            }
            
        }

        return ret;
    }



}