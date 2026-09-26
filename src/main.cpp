#include <iostream>
#include <Schedule.h>
#include <chrono>

int main() {
    

    auto start = std::chrono::high_resolution_clock::now();
    // auto all = FieldDay::Schedule::GenerateAllSchedules(5);
    auto all = FieldDay::Schedule::GenerateAllSchedulesCount(5);
    auto end = std::chrono::high_resolution_clock::now();
    
    // std::cout << all.size() << std::endl;
    std::cout << all << std::endl;
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds"<< std::endl;;
    return 0;
}
