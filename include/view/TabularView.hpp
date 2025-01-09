//
// Created by aryam on 21-04-2024.
//

#ifndef ECE141DB_TABULARVIEW_HPP
#define ECE141DB_TABULARVIEW_HPP

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>

class TabularView {
private:
    std::vector<std::string> databases;
    std::ostream& stream;

public:
    TabularView(std::ostream& os, const std::vector<std::string>& dbNames) : stream(os), databases(dbNames) {}

    void show() const {
        auto start = std::chrono::high_resolution_clock::now();
        int width = 20;
        int totalWidth = width + 2;

        stream << '+' << std::string(totalWidth - 2, '-') << '+' << std::endl;
        stream << "| " << std::left << std::setw(width - 2) << "Database" << " |" << std::endl;
        stream << '+' << std::string(totalWidth - 2, '-') << '+' << std::endl;

        for (const auto& dbName : databases) {
            stream << "| " << std::left << std::setw(width - 2) << dbName << " |" << std::endl;
        }

        stream << '+' << std::string(totalWidth - 2, '-') << '+' << std::endl;

        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = finish - start;

        stream << "Query Ok, " << databases.size() << " rows affected (" << elapsed.count() / 1000 << " sec)\n";
    }
};

#endif //ECE141DB_TABULARVIEW_HPP
