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
    "Food", "Drinks", "Transportation", "Laundry", "Rent"
};

vector<Record> trainData, testData, historyData;
array<Record, DAYS> currentWeek;
array<double, DAYS * 4> forecast{};
array<double, CATEGORIES> categoryTotal{};
ForecastModel* model = nullptr;
Metrics metrics;
int currentWeekNumber;
double weeklyTotal, weeklyForecast, monthlyForecast;
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
    currentWeekNumber = testData.back().week + 1;
}

void inputExpense() {
    cout << "1. DAILY EXPENSE INPUT\n";
    for (size_t day = 0; day < DAYS; day++) {
        currentWeek[day].week = currentWeekNumber;
        currentWeek[day].day = day;
        cout << "\n" << HARI[day] << '\n';
        for (size_t category = 0; category < CATEGORIES; category++)
            currentWeek[day].expense[category] = inputNumber(
                "  " + KATEGORI[category] + ": Rp ");
    }
}

void showHistory() {
    cout << "\n2. EXPENSE HISTORY\n";
    for (const Record& row : currentWeek) {
        cout << HARI[row.day];
        for (size_t category = 0; category < CATEGORIES; category++)
            cout << " | " << KATEGORI[category] << ": "
                 << row.expense[category];
        cout << " | Total: " << row.total() << '\n';
    }
}

void showStatistics() {
    weeklyTotal = 0;
    categoryTotal.fill(0);
    highestDay = lowestDay = 0;

    for (size_t day = 0; day < DAYS; day++) {
        weeklyTotal += currentWeek[day].total();
        if (currentWeek[day].total() > currentWeek[highestDay].total())
            highestDay = day;
        if (currentWeek[day].total() < currentWeek[lowestDay].total())
            lowestDay = day;
        for (size_t category = 0; category < CATEGORIES; category++)
            categoryTotal[category] += currentWeek[day].expense[category];
    }

    largestCategory = 0;
    for (size_t category = 1; category < CATEGORIES - 1; category++)
        if (categoryTotal[category] > categoryTotal[largestCategory])
            largestCategory = category;

    cout << "\n3. EXPENSE STATISTICS\n"
         << "Total mingguan  : Rp " << weeklyTotal << '\n'
         << "Rata-rata harian: Rp "
         << weeklyTotal / static_cast<double>(DAYS) << '\n'
         << "Tertinggi       : " << HARI[highestDay] << '\n'
         << "Terendah        : " << HARI[lowestDay] << '\n'
         << "Kategori fleksibel terbesar: "
         << KATEGORI[largestCategory] << '\n';
}

void predictExpense() {
    vector<Record> allData = historyData;
    allData.insert(allData.end(), currentWeek.begin(), currentWeek.end());
    model->train(allData);

    for (size_t week = 0; week < 4; week++)
        for (size_t day = 0; day < DAYS; day++)
            forecast[week * DAYS + day] =
                model->predict(currentWeekNumber + 1
                               + static_cast<int>(week), day);

    weeklyForecast = accumulate(forecast.begin(), forecast.begin() + DAYS, 0.0);
    monthlyForecast = accumulate(forecast.begin(), forecast.end(), 0.0);

    cout << "\n4. EXPENSE PREDICTION\n"
         << "Model: " << model->name() << '\n'
         << "MAE test      : Rp " << metrics.mae << '\n'
         << "RMSE test     : Rp " << metrics.rmse << '\n'
         << setprecision(1) << "WAPE test     : " << metrics.wape << "%\n"
         << setprecision(0) << "MAE baseline  : Rp " << metrics.baselineMae
         << "\nPrediksi minggu ke-" << currentWeekNumber + 1 << ":\n";
    for (size_t day = 0; day < DAYS; day++)
        cout << HARI[day] << ": Rp " << forecast[day] << '\n';
    cout << "Total minggu depan: Rp " << weeklyForecast
         << "\nTotal 4 minggu    : Rp " << monthlyForecast << '\n';
}

void inputBudget() {
    cout << "\n5. DAILY/MONTHLY BUDGET\n";
    dailyBudget = inputNumber("Budget harian: Rp ", true);
    monthlyBudget = inputNumber("Budget 4 minggu: Rp ", true);
}

void monitorBudget() {
    cout << "\n6. BUDGET MONITORING\n";
    for (size_t day = 0; day < DAYS; day++)
        cout << HARI[day] << " | Aktual: " << currentWeek[day].total()
             << " (" << status(currentWeek[day].total()) << ")"
             << " | Forecast: " << forecast[day]
             << " (" << status(forecast[day]) << ")\n";
    cout << "Sisa budget 4 minggu: Rp "
         << monthlyBudget - monthlyForecast << '\n';
}

void showWarning() {
    cout << "\n7. BUDGET WARNING\n"
         << "Heuristik berdasarkan RMSE test:\n";
    for (size_t day = 0; day < DAYS; day++) {
        string risk = forecast[day] - metrics.rmse > dailyBudget
                    ? "RISIKO TINGGI"
                    : forecast[day] + metrics.rmse > dailyBudget
                    ? "WASPADA" : "AMAN";
        cout << HARI[day] << ": " << risk << '\n';
    }
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
