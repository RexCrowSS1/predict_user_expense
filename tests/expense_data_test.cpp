#include "common/expense_data.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

using namespace std;

int main() {
    try {
        for (const string path : {"data/train.csv", "data/expense_test_fluctuated.csv"}) {
            ifstream file(path);
            string line;
            getline(file, line);
            map<int, double> laundry, totals, loadedTotals;
            while (getline(file, line)) {
                if (line.empty()) continue;
                stringstream stream(line);
                vector<string> columns;
                string value;
                while (getline(stream, value, ',')) columns.push_back(value);
                const int week = stoi(columns.at(1));
                laundry[week] += stod(columns.at(7));
                totals[week] += stod(columns.at(8));
            }
            for (const Record& row : readCsv(path)) {
                if (abs(row.expense[LAUNDRY] - laundry.at(row.week) / DAYS) > 1e-6)
                    throw runtime_error("Laundry historis tidak dibagi rata per minggu.");
                loadedTotals[row.week] += row.total();
            }
            if (loadedTotals.size() != totals.size())
                throw runtime_error("Jumlah minggu berubah saat membaca CSV.");
            for (const auto& entry : totals)
                if (abs(loadedTotals.at(entry.first) - entry.second) > 1e-6)
                    throw runtime_error("Alokasi Laundry mengubah total pengeluaran mingguan.");
        }
        cout << "Data tanpa Rent dan alokasi Laundry OK\n";
    } catch (const exception& error) {
        cerr << "TEST GAGAL: " << error.what() << '\n';
        return 1;
    }
}
