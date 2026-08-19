#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Data {
    int week;
    string day;
    double expense;
    double budget;
    bool overBudget;
};

// Memisahkan setiap kolom CSV berdasarkan tanda koma.
vector<string> split(string line) {
    vector<string> columns;
    string value;
    stringstream stream(line);

    while (getline(stream, value, ',')) {
        columns.push_back(value);
    }

    return columns;
}

// Membaca dataset CSV.
vector<Data> readCsv(string filename) {
    vector<Data> dataset;
    ifstream file(filename);
    string line;

    if (!file) {
        cout << "File " << filename << " tidak ditemukan.\n";
        return dataset;
    }

    getline(file, line); // Lewati header.

    while (getline(file, line)) {
        vector<string> column = split(line);

        if (column.size() >= 12) {
            Data data;
            data.week = stoi(column[1]);
            data.day = column[3];
            data.expense = stod(column[9]);
            data.budget = stod(column[10]);
            data.overBudget = column[11].find("OVER_BUDGET") != string::npos;
            dataset.push_back(data);
        }
    }

    return dataset;
}

// Melatih Decision Stump dengan mencari batas antara dua status.
double trainModel(vector<Data> trainingData) {
    double biggestWithin = -1000000000;
    double smallestOver = 1000000000;

    for (Data data : trainingData) {
        double difference = data.expense - data.budget;

        if (data.overBudget && difference < smallestOver) {
            smallestOver = difference;
        }

        if (!data.overBudget && difference > biggestWithin) {
            biggestWithin = difference;
        }
    }

    return (biggestWithin + smallestOver) / 2;
}

// Model hanya menggunakan satu aturan keputusan.
bool predictStatus(Data data, double threshold) {
    return data.expense - data.budget > threshold;
}

string status(bool overBudget) {
    if (overBudget) {
        return "OVER_BUDGET";
    }

    return "WITHIN_BUDGET";
}

// Menghitung prediksi dari rata-rata hari dan posisi minggu yang sama.
double predictExpense(vector<Data> history, int weekPosition, string day) {
    double total = 0;
    int count = 0;

    for (Data data : history) {
        if ((data.week - 1) % 4 == weekPosition && data.day == day) {
            total = total + data.expense;
            count++;
        }
    }

    if (count == 0) {
        return 0;
    }

    return total / count;
}

int main() {
    cout << fixed << setprecision(0);

    // TAHAP 1: Membaca data training dan data test.
    vector<Data> trainingData = readCsv("train.csv");
    vector<Data> testData = readCsv("expense_test_fluctuated.csv");

    if (trainingData.empty() || testData.empty()) {
        return 1;
    }

    // TAHAP 2: Melatih model Decision Stump.
    double threshold = trainModel(trainingData);
    cout << "Batas model: " << threshold << "\n";

    // TAHAP 3: Menguji model dengan data bulan ke-4.
    int correct = 0;

    cout << "\nHASIL PENGUJIAN BULAN KE-4\n";
    for (Data data : testData) {
        bool prediction = predictStatus(data, threshold);

        cout << "Minggu " << data.week << " - " << data.day
             << ": " << status(prediction) << '\n';

        if (prediction == data.overBudget) {
            correct++;
        }
    }

    double accuracy = 100.0 * correct / testData.size();
    cout << "Akurasi: " << setprecision(2) << accuracy << "%\n";

    // TAHAP 4: Menggabungkan data bulan 1 sampai bulan 4.
    vector<Data> history = trainingData;

    for (Data data : testData) {
        history.push_back(data);
    }

    string days[7] = {
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"
    };

    // TAHAP 5: Memprediksi pengeluaran bulan ke-5.
    double dailyBudget = 150000;
    double monthlyPrediction = 0;

    cout << "\nPREDIKSI PENGELUARAN BULAN KE-5\n";

    for (int weekPosition = 0; weekPosition < 4; weekPosition++) {
        int futureWeek = 17 + weekPosition;

        for (int day = 0; day < 7; day++) {
            double expense = predictExpense(history, weekPosition, days[day]);

            Data futureData;
            futureData.week = futureWeek;
            futureData.day = days[day];
            futureData.expense = expense;
            futureData.budget = dailyBudget;
            futureData.overBudget = false;

            monthlyPrediction = monthlyPrediction + expense;

            cout << "Minggu " << futureWeek << " - " << days[day]
                 << ": Rp " << setprecision(0) << expense
                 << " (" << status(predictStatus(futureData, threshold)) << ")\n";
        }
    }

    // TAHAP 6: Menampilkan total prediksi.
    cout << "\nTotal prediksi bulan ke-5: Rp "
         << monthlyPrediction << '\n';

    return 0;
}
