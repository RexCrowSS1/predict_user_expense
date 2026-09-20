#include "common/expense_data.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {

const array<string, DAYS> DAY_NAMES = {
    "Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday", "Saturday", "Sunday"
};

vector<string> split(const string& line) {
    vector<string> values;
    string value;
    stringstream stream(line);
    while (getline(stream, value, ',')) values.push_back(value);
    return values;
}

size_t dayIndex(const string& name) {
    auto found = find(DAY_NAMES.begin(), DAY_NAMES.end(), name);
    if (found == DAY_NAMES.end()) throw invalid_argument("Hari tidak valid.");
    return static_cast<size_t>(distance(DAY_NAMES.begin(), found));
}

} // namespace

double Record::total() const {
    return accumulate(expense.begin(), expense.end(), 0.0);
}

vector<Record> readCsv(const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("File " + filename + " tidak ditemukan.");

    vector<Record> data;
    string line;
    getline(file, line);
    const vector<string> header = split(line);
    const vector<string> expected = {
        "Dataset_Type", "Week", "Month", "Day", "Food", "Drinks",
        "Transportation", "Laundry"
    };
    if (header.size() < expected.size()
        || !equal(expected.begin(), expected.end(), header.begin())
        || find(header.begin(), header.end(), "Rent") != header.end())
        throw runtime_error("Header CSV harus memakai kategori tanpa Rent.");

    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> column = split(line);
        if (column.size() < 4 + CATEGORIES)
            throw runtime_error("Format CSV tidak valid.");

        Record row;
        row.week = stoi(column[1]);
        row.day = dayIndex(column[3]);
        for (size_t category = 0; category < CATEGORIES; category++) {
            row.expense[category] = stod(column[4 + category]);
            if (!isfinite(row.expense[category]) || row.expense[category] < 0)
                throw runtime_error("Nominal CSV tidak valid.");
        }
        if (row.week <= 0) throw runtime_error("Minggu CSV tidak valid.");
        data.push_back(row);
    }

    if (data.empty()) throw runtime_error("CSV tidak memiliki data.");
    sort(data.begin(), data.end(), [](const Record& a, const Record& b) {
        return a.week == b.week ? a.day < b.day : a.week < b.week;
    });
    // CSV records payments; the model uses an equal daily laundry allocation.
    for (size_t start = 0; start < data.size();) {
        size_t end = start;
        double weeklyLaundry = 0;
        while (end < data.size() && data[end].week == data[start].week) {
            weeklyLaundry += data[end].expense[LAUNDRY];
            end++;
        }
        if (end - start != DAYS)
            throw runtime_error("Data CSV harus memuat tujuh hari per minggu.");
        for (size_t day = 0; day < DAYS; day++) {
            if (data[start + day].day != day)
                throw runtime_error("Hari CSV duplikat atau tidak lengkap.");
            data[start + day].expense[LAUNDRY] = weeklyLaundry / DAYS;
        }
        start = end;
    }
    return data;
}
