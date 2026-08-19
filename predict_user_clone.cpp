#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int N = 7;
string hari[N] = {"Senin", "Selasa", "Rabu", "Kamis",
                  "Jumat", "Sabtu", "Minggu"};

double food[N], drinks[N], transport[N], laundry[N], rent[N];
double expense[N], forecast[N];
double total, averageExpense, weeklyForecast, monthlyForecast;
double dailyBudget, monthlyBudget;
int highestDay;

struct Data {
    double gap; // Total_Expense - Daily_Budget
    int label;  // 0 = WITHIN, 1 = OVER
};

vector<Data> readCsv(string filename) {
    vector<Data> dataset;
    ifstream file(filename);
    string line;

    getline(file, line); // Lewati header.
    while (getline(file, line)) {
        vector<string> column;
        string value;
        stringstream row(line);

        while (getline(row, value, ',')) {
            column.push_back(value);
        }

        if (column.size() >= 12) {
            double gap = stod(column[9]) - stod(column[10]);
            int label = column[11].find("OVER_BUDGET") != string::npos;
            dataset.push_back({gap, label});
        }
    }
    return dataset;
}

// KNN mencari 3 data dengan jarak paling dekat.
class KNN {
private:
    vector<Data> trainingData;

public:
    void train(vector<Data> data) {
        trainingData = data;
    }

    bool predict(double totalExpense, double budget) {
        vector<pair<double, int>> distances;
        double userGap = totalExpense - budget;

        for (Data data : trainingData) {
            distances.push_back({abs(userGap - data.gap), data.label});
        }

        sort(distances.begin(), distances.end());

        int overVotes = 0;
        for (int i = 0; i < 3; i++) {
            overVotes = overVotes + distances[i].second;
        }
        return overVotes >= 2;
    }
};

KNN model;

string status(bool over) {
    return over ? "OVER_BUDGET" : "WITHIN_BUDGET";
}

// Melatih dengan train.csv dan menguji dengan file test.
void trainAndTest() {
    vector<Data> train = readCsv("train.csv");
    vector<Data> test = readCsv("expense_test_fluctuated.csv");

    model.train(train);
    int correct = 0;

    for (Data data : test) {
        // budget dibuat 0 karena data.gap sudah berisi total - budget.
        if (model.predict(data.gap, 0) == data.label) {
            correct++;
        }
    }

    cout << "MODEL KNN (k = 3)\n";
    cout << "Training: " << train.size() << " data\n";
    cout << "Testing : " << test.size() << " data\n";
    cout << "Akurasi : " << 100.0 * correct / test.size() << "%\n";

    // Setelah testing, semua data dipakai untuk model user.
    train.insert(train.end(), test.begin(), test.end());
    model.train(train);
}

// 1. DAILY EXPENSE INPUT
void inputExpense() {
    cout << "\n1. DAILY EXPENSE INPUT\n";
    for (int i = 0; i < N; i++) {
        cout << "\n" << hari[i] << '\n';
        cout << "Food          : Rp "; cin >> food[i];
        cout << "Drinks        : Rp "; cin >> drinks[i];
        cout << "Transportation: Rp "; cin >> transport[i];
        cout << "Laundry       : Rp "; cin >> laundry[i];
        cout << "Rent          : Rp "; cin >> rent[i];

        expense[i] = food[i] + drinks[i] + transport[i]
                   + laundry[i] + rent[i];
    }
}

// 2. EXPENSE HISTORY
void showHistory() {
    cout << "\n2. EXPENSE HISTORY\n";
    for (int i = 0; i < N; i++) {
        cout << hari[i] << " | Food: " << food[i]
             << " | Drinks: " << drinks[i]
             << " | Transport: " << transport[i]
             << " | Laundry: " << laundry[i]
             << " | Rent: " << rent[i]
             << " | Total: " << expense[i] << '\n';
    }
}

// 3. EXPENSE STATISTICS
void showStatistics() {
    total = 0;
    highestDay = 0;

    for (int i = 0; i < N; i++) {
        total += expense[i];
        if (expense[i] > expense[highestDay]) highestDay = i;
    }

    averageExpense = total / N;
    cout << "\n3. EXPENSE STATISTICS\n";
    cout << "Total    : Rp " << total << '\n';
    cout << "Rata-rata: Rp " << averageExpense << '\n';
    cout << "Tertinggi: " << hari[highestDay] << '\n';
}

// 4. EXPENSE PREDICTION: Moving Average 3 hari.
void predictExpense() {
    double data[14];
    weeklyForecast = 0;

    for (int i = 0; i < N; i++) data[i] = expense[i];
    for (int i = 7; i < 14; i++) {
        data[i] = (data[i - 1] + data[i - 2] + data[i - 3]) / 3;
        forecast[i - 7] = data[i];
        weeklyForecast += data[i];
    }

    monthlyForecast = weeklyForecast * 4;
    cout << "\n4. EXPENSE PREDICTION\n";
    for (int i = 0; i < N; i++) {
        cout << hari[i] << ": Rp " << forecast[i] << '\n';
    }
    cout << "Prediksi bulanan: Rp " << monthlyForecast << '\n';
}

// 5. DAILY/MONTHLY BUDGET
void inputBudget() {
    cout << "\n5. DAILY/MONTHLY BUDGET\n";
    cout << "Budget harian : Rp "; cin >> dailyBudget;
    cout << "Budget bulanan: Rp "; cin >> monthlyBudget;
}

// 6. BUDGET MONITORING
void monitorBudget() {
    cout << "\n6. BUDGET MONITORING\n";
    for (int i = 0; i < N; i++) {
        cout << hari[i] << ": "
             << status(model.predict(expense[i], dailyBudget)) << '\n';
    }
    cout << "Sisa budget: Rp " << monthlyBudget - monthlyForecast << '\n';
}

// 7. BUDGET WARNING
void showWarning() {
    cout << "\n7. BUDGET WARNING\n";
    bool warning = false;

    for (int i = 0; i < N; i++) {
        if (model.predict(expense[i], dailyBudget)) {
            cout << hari[i] << " berisiko melewati budget.\n";
            warning = true;
        }
    }
    if (monthlyForecast > monthlyBudget) {
        cout << "Prediksi bulanan melewati budget.\n";
        warning = true;
    }
    if (!warning) cout << "Pengeluaran masih aman.\n";
}

// 8. DAILY REMINDER
void showReminder() {
    cout << "\n8. DAILY REMINDER\n";
    cout << "Jangan lupa mencatat pengeluaran setiap hari.\n";
}

// 9. SPENDING RECOMMENDATION
void showRecommendation() {
    cout << "\n9. SPENDING RECOMMENDATION\n";
    if (monthlyForecast > monthlyBudget)
        cout << "Kurangi pengeluaran yang tidak wajib.\n";
    else
        cout << "Pola pengeluaran masih sesuai budget.\n";

    cout << "Periksa pengeluaran tertinggi pada hari "
         << hari[highestDay] << ".\n";
}

int main() {
    cout << fixed << setprecision(0);

    trainAndTest();
    inputExpense();
    showHistory();
    showStatistics();
    predictExpense();
    inputBudget();
    monitorBudget();
    showWarning();
    showReminder();
    showRecommendation();

    return 0;
}
