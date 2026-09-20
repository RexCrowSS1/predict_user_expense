#ifndef EXPENSE_DATA_H
#define EXPENSE_DATA_H

#include <array>
#include <cstddef>
#include <string>
#include <vector>

constexpr std::size_t DAYS = 7;
constexpr std::size_t CATEGORIES = 4;
constexpr std::size_t LAUNDRY = 3;
constexpr std::size_t WEEKS = 4;
constexpr std::size_t PHASES = 4;

struct Record {
    int week = 0;
    std::size_t day = 0;
    std::array<double, CATEGORIES> expense{};
    double total() const;
};

std::vector<Record> readCsv(const std::string& filename);

#endif
