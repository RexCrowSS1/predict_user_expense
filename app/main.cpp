#include "common/evaluation.h"
#include "models/knn/knn_model.h"
#include "models/linear_regression/linear_model.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

const array<string, DAYS> HARI = {
    "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu"
};
const array<string, CATEGORIES> KATEGORI = {
    "Food", "Drinks", "Transportation", "Laundry"
};

vector<Record> trainData, testData, historyData;
array<Record, DAYS * WEEKS> currentMonth;
array<double, WEEKS> laundry{}, weeklyTotal{}, weeklyForecast{};
array<double, DAYS * WEEKS> forecast{};
array<double, CATEGORIES> categoryTotal{};
ForecastModel* model = nullptr;
Metrics metrics;
int firstWeekNumber;
double monthlyTotal, monthlyForecast, monthlyRent;
double dailyBudget, monthlyBudget;
size_t highestDay, lowestDay, largestCategory;

double inputNumber(const string& prompt, bool positive = false) {
    while (true) {
        cout << prompt;
        double value;
        if (cin >> value) {
            if (isfinite(value) && (positive ? value > 0 : value >= 0))
                return value;
        } else if (cin.eof()) {
            throw runtime_error("Input dihentikan.");
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Masukkan angka yang valid.\n";
    }
}

string status(double value) {
    return value > dailyBudget ? "OVER_BUDGET" : "WITHIN_BUDGET";
}

void prepare() {
    trainData = readCsv("data/train.csv");
    testData = readCsv("data/expense_test_fluctuated.csv");
    model->train(trainData);
    metrics = evaluate(*model, trainData, testData);
    historyData = trainData;
    historyData.insert(historyData.end(), testData.begin(), testData.end());
    firstWeekNumber = testData.back().week + 1;
}

void inputExpense() {
    cout << "1. EXPENSE INPUT (4 MINGGU / 28 HARI)\n";
    monthlyRent = inputNumber("Rent 1 bulan (4 minggu): Rp ");
    cout << "Rent per minggu: Rp " << monthlyRent / WEEKS
         << " (tidak masuk budget harian)\n";
    for (size_t week = 0; week < WEEKS; week++) {
        cout << "\nMinggu " << week + 1 << '\n';
        laundry[week] = inputNumber("Laundry minggu ini: Rp ");
        cout << "Alokasi Laundry per hari: Rp " << setprecision(2)
             << laundry[week] / DAYS << setprecision(0) << '\n';
    }
    cout << "\nInput Food, Drinks, dan Transportation untuk satu minggu.\n"
         << "Nominal setiap hari digunakan kembali pada minggu 2-4.\n";
    for (size_t day = 0; day < DAYS; day++) {
        cout << HARI[day] << '\n';
        for (size_t category = 0; category < LAUNDRY; category++)
            currentMonth[day].expense[category] = inputNumber(
                "  " + KATEGORI[category] + ": Rp ");
    }
    for (size_t week = 0; week < WEEKS; week++) {
        for (size_t day = 0; day < DAYS; day++) {
            Record& row = currentMonth[week * DAYS + day];
            row.week = firstWeekNumber + static_cast<int>(week);
            row.day = day;
            row.expense[LAUNDRY] = laundry[week] / DAYS;
            for (size_t category = 0; category < LAUNDRY; category++)
                row.expense[category] = currentMonth[day].expense[category];
        }
    }
}

void showHistory() {
    cout << "\n2. EXPENSE HISTORY\n"
         << "Food, Drinks, dan Transportation minggu 2-4 memakai input minggu pertama.\n"
         << setprecision(2);
    for (size_t week = 0; week < WEEKS; week++) {
        cout << "Minggu " << week + 1 << " | Laundry: Rp " << laundry[week]
             << " | Rent: Rp " << monthlyRent / WEEKS << '\n';
        for (size_t day = 0; day < DAYS; day++) {
            const Record& row = currentMonth[week * DAYS + day];
            cout << HARI[row.day];
            for (size_t category = 0; category < CATEGORIES; category++)
                cout << " | " << KATEGORI[category] << ": "
                     << row.expense[category];
            cout << " | Total harian tanpa Rent: " << row.total() << '\n';
        }
    }
    cout << setprecision(0);
}

void showStatistics() {
    monthlyTotal = 0;
    weeklyTotal.fill(monthlyRent / WEEKS);
    categoryTotal.fill(0);
    highestDay = lowestDay = 0;
    for (size_t index = 0; index < currentMonth.size(); index++) {
        const Record& row = currentMonth[index];
        weeklyTotal[index / DAYS] += row.total();
        if (row.total() > currentMonth[highestDay].total()) highestDay = index;
        if (row.total() < currentMonth[lowestDay].total()) lowestDay = index;
        for (size_t category = 0; category < CATEGORIES; category++)
            categoryTotal[category] += row.expense[category];
    }
    monthlyTotal = accumulate(weeklyTotal.begin(), weeklyTotal.end(), 0.0);
    largestCategory = static_cast<size_t>(distance(categoryTotal.begin(),
        max_element(categoryTotal.begin(), categoryTotal.end())));

    cout << "\n3. EXPENSE STATISTICS\n";
    for (size_t week = 0; week < WEEKS; week++)
        cout << "Total minggu " << week + 1 << " (termasuk Rent): Rp "
             << weeklyTotal[week] << '\n';
    cout << "Total 4 minggu (termasuk Rent): Rp " << monthlyTotal << '\n'
         << "Rata-rata harian tanpa Rent: Rp "
         << (monthlyTotal - monthlyRent) / currentMonth.size() << '\n'
         << "Tertinggi: Minggu " << highestDay / DAYS + 1 << ", "
         << HARI[highestDay % DAYS] << '\n'
         << "Terendah : Minggu " << lowestDay / DAYS + 1 << ", "
         << HARI[lowestDay % DAYS] << '\n'
         << "Kategori fleksibel terbesar: " << KATEGORI[largestCategory] << '\n';
}

void predictExpense() {
    vector<Record> allData = historyData;
    allData.insert(allData.end(), currentMonth.begin(), currentMonth.end());
    model->train(allData);

    cout << "\n4. EXPENSE PREDICTION\n"
         << "Model: " << model->name() << '\n'
         << "MAE test      : Rp " << metrics.mae << '\n'
         << "RMSE test     : Rp " << metrics.rmse << '\n'
         << setprecision(1) << "WAPE test     : " << metrics.wape << "%\n"
         << setprecision(0) << "MAE baseline  : Rp " << metrics.baselineMae
         << "\nPrediksi harian termasuk Laundry, tanpa Rent.\n"
         << "Rent untuk 4 minggu berikutnya diasumsikan tetap: Rp "
         << monthlyRent << '\n';
    for (size_t week = 0; week < WEEKS; week++) {
        weeklyForecast[week] = monthlyRent / WEEKS;
        const int targetWeek = firstWeekNumber + static_cast<int>(WEEKS + week);
        cout << "Prediksi minggu ke-" << targetWeek << ":\n";
        for (size_t day = 0; day < DAYS; day++) {
            const size_t index = week * DAYS + day;
            forecast[index] = model->predict(targetWeek, day);
            weeklyForecast[week] += forecast[index];
            cout << HARI[day] << ": Rp " << forecast[index] << '\n';
        }
        cout << "Total prediksi minggu " << week + 1
             << " (termasuk Rent): Rp " << weeklyForecast[week] << '\n';
    }
    monthlyForecast = accumulate(weeklyForecast.begin(), weeklyForecast.end(), 0.0);
    cout << "Total prediksi 4 minggu (termasuk Rent): Rp " << monthlyForecast << '\n';
}

void inputBudget() {
    cout << "\n5. DAILY/MONTHLY BUDGET\n";
    dailyBudget = inputNumber("Budget harian (termasuk Laundry, tanpa Rent): Rp ", true);
    monthlyBudget = inputNumber("Budget 4 minggu (termasuk Rent): Rp ", true);
}

void monitorBudget() {
    cout << "\n6. BUDGET MONITORING\n"
         << "Status harian termasuk Laundry, tanpa Rent.\n";
    for (size_t week = 0; week < WEEKS; week++) {
        cout << "Minggu " << week + 1 << " input / minggu " << week + 1
             << " periode berikutnya:\n";
        for (size_t day = 0; day < DAYS; day++) {
            const size_t index = week * DAYS + day;
            const double actual = currentMonth[index].total();
            cout << HARI[day] << " | Aktual: " << actual
                 << " (" << status(actual) << ")"
                 << " | Forecast: " << forecast[index]
                 << " (" << status(forecast[index]) << ")\n";
        }
    }
    cout << "Sisa budget aktual 4 minggu: Rp " << monthlyBudget - monthlyTotal
         << "\nSisa budget prediksi 4 minggu: Rp " << monthlyBudget - monthlyForecast << '\n';
}

void showWarning() {
    cout << "\n7. BUDGET WARNING\n"
         << "Heuristik harian tanpa Rent berdasarkan RMSE test:\n";
    for (size_t index = 0; index < forecast.size(); index++) {
        string risk = forecast[index] - metrics.rmse > dailyBudget
                    ? "RISIKO TINGGI"
                    : forecast[index] + metrics.rmse > dailyBudget
                    ? "WASPADA" : "AMAN";
        cout << "Minggu prediksi " << index / DAYS + 1 << " "
             << HARI[index % DAYS] << ": " << risk << '\n';
    }
    if (monthlyTotal > monthlyBudget)
        cout << "Pengeluaran aktual 4 minggu melewati budget.\n";
    if (monthlyForecast > monthlyBudget)
        cout << "Forecast 4 minggu melewati budget.\n";
}

void showReminder() {
    cout << "\n8. DAILY REMINDER\n"
         << "Jangan lupa mencatat pengeluaran setiap hari.\n";
}

void showRecommendation() {
    cout << "\n9. SPENDING RECOMMENDATION\n";
    if (monthlyForecast > monthlyBudget)
        cout << "Kurangi sekitar Rp "
             << (monthlyForecast - monthlyBudget) / 28.0
             << " per hari, terutama kategori "
             << KATEGORI[largestCategory] << ".\n";
    else
        cout << "Forecast masih sesuai budget. Pantau kategori "
             << KATEGORI[largestCategory] << ".\n";
    cout << setprecision(1) << "Error relatif model (WAPE): "
         << metrics.wape << "%\n" << setprecision(0);
}

int main(int argc, char* argv[]) {
    cout << fixed << setprecision(0);
    LinearRegressionModel linear;
    KnnModel knn;
    const string choice = argc == 1 ? "linear" : argc == 2 ? argv[1] : "";
    if (choice == "linear") model = &linear;
    else if (choice == "knn") model = &knn;
    else {
        cerr << "Gunakan: " << argv[0] << " [linear|knn]\n";
        return 1;
    }
    try {
        prepare();
        inputExpense();
        showHistory();
        showStatistics();
        predictExpense();
        inputBudget();
        monitorBudget();
        showWarning();
        showReminder();
        showRecommendation();
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
