#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

const int JUMLAH_HARI = 7;

string hari[JUMLAH_HARI] = {
    "Senin", "Selasa", "Rabu", "Kamis",
    "Jumat", "Sabtu", "Minggu"
};

// Variabel sesuai kolom pengeluaran pada train.csv.
double food[JUMLAH_HARI];
double drinks[JUMLAH_HARI];
double transportation[JUMLAH_HARI];
double laundry[JUMLAH_HARI];
double rent[JUMLAH_HARI];
double totalExpense[JUMLAH_HARI];

double totalMingguan;
double rataRata;
double prediksiHarian[JUMLAH_HARI];
double prediksiMingguan;
double prediksiBulanan;
double budgetHarian;
double budgetBulanan;
int indexTerbesar;
int indexTerkecil;

// Model Decision Stump sederhana dengan satu aturan.
string prediksiStatus(double pengeluaran) {
    if (pengeluaran > budgetHarian) {
        return "OVER_BUDGET";
    }

    return "WITHIN_BUDGET";
}

// 1. DAILY EXPENSE INPUT
void dailyExpenseInput() {
    cout << "1. DAILY EXPENSE INPUT\n";

    for (int i = 0; i < JUMLAH_HARI; i++) {
        cout << "\nInput pengeluaran " << hari[i] << "\n";
        cout << "Food          : Rp ";
        cin >> food[i];
        cout << "Drinks        : Rp ";
        cin >> drinks[i];
        cout << "Transportation: Rp ";
        cin >> transportation[i];
        cout << "Laundry       : Rp ";
        cin >> laundry[i];
        cout << "Rent          : Rp ";
        cin >> rent[i];

        totalExpense[i] = food[i] + drinks[i] + transportation[i]
                        + laundry[i] + rent[i];
    }
}

// 2. EXPENSE HISTORY
void expenseHistory() {
    cout << "\n2. EXPENSE HISTORY\n";
    cout << left << setw(10) << "Hari"
         << setw(12) << "Food"
         << setw(12) << "Drinks"
         << setw(17) << "Transportation"
         << setw(12) << "Laundry"
         << setw(12) << "Rent"
         << "Total\n";

    for (int i = 0; i < JUMLAH_HARI; i++) {
        cout << left << setw(10) << hari[i]
             << setw(12) << food[i]
             << setw(12) << drinks[i]
             << setw(17) << transportation[i]
             << setw(12) << laundry[i]
             << setw(12) << rent[i]
             << totalExpense[i] << '\n';
    }
}

// 3. EXPENSE STATISTICS
void expenseStatistics() {
    totalMingguan = 0;
    indexTerbesar = 0;
    indexTerkecil = 0;

    for (int i = 0; i < JUMLAH_HARI; i++) {
        totalMingguan = totalMingguan + totalExpense[i];

        if (totalExpense[i] > totalExpense[indexTerbesar]) {
            indexTerbesar = i;
        }

        else if (totalExpense[i] < totalExpense[indexTerkecil]) {
            indexTerkecil = i;
        }
    }

    rataRata = totalMingguan / JUMLAH_HARI;

    cout << "\n3. EXPENSE STATISTICS\n";
    cout << "Total mingguan  : Rp " << totalMingguan << '\n';
    cout << "Rata-rata harian: Rp " << rataRata << '\n';
    cout << "Terbesar        : " << hari[indexTerbesar]
         << " (Rp " << totalExpense[indexTerbesar] << ")\n";
    cout << "Terkecil        : " << hari[indexTerkecil]
         << " (Rp " << totalExpense[indexTerkecil] << ")\n";
}

// 4. EXPENSE PREDICTION
void expensePrediction() {
    // Model data science sederhana: Moving Average 3 Hari.
    // Setiap prediksi dihitung dari rata-rata tiga data sebelumnya.
    double data[14];

    for (int i = 0; i < JUMLAH_HARI; i++) {
        data[i] = totalExpense[i];
    }

    prediksiMingguan = 0;

    for (int i = 7; i < 14; i++) {
        data[i] = (data[i - 1] + data[i - 2] + data[i - 3]) / 3;
        prediksiHarian[i - 7] = data[i];
        prediksiMingguan = prediksiMingguan + data[i];
    }

    prediksiBulanan = prediksiMingguan * 4;

    cout << "\n4. EXPENSE PREDICTION\n";
    cout << "Model: Moving Average 3 Hari\n";

    for (int i = 0; i < JUMLAH_HARI; i++) {
        cout << "Prediksi " << hari[i] << ": Rp "
             << prediksiHarian[i] << '\n';
    }

    cout << "Total prediksi minggu depan: Rp " << prediksiMingguan << '\n';
    cout << "Total prediksi satu bulan  : Rp " << prediksiBulanan << '\n';
}

// 5. DAILY/MONTHLY BUDGET
void dailyMonthlyBudget() {
    cout << "\n5. DAILY/MONTHLY BUDGET\n";
    cout << "Masukkan budget harian : Rp ";
    cin >> budgetHarian;
    cout << "Masukkan budget bulanan: Rp ";
    cin >> budgetBulanan;
}

// 6. BUDGET MONITORING
void budgetMonitoring() {
    cout << "\n6. BUDGET MONITORING\n";

    for (int i = 0; i < JUMLAH_HARI; i++) {
        cout << hari[i] << ": " << prediksiStatus(totalExpense[i]) << '\n';
    }

    cout << "Sisa budget bulanan: Rp "
         << budgetBulanan - prediksiBulanan << '\n';
}

// 7. BUDGET WARNING
void budgetWarning() {
    bool adaPeringatan = false;

    cout << "\n7. BUDGET WARNING\n";

    for (int i = 0; i < JUMLAH_HARI; i++) {
        if (totalExpense[i] > budgetHarian) {
            cout << "Peringatan: " << hari[i]
                 << " melewati budget harian.\n";
            adaPeringatan = true;
        }
    }

    if (prediksiBulanan > budgetBulanan) {
        cout << "Peringatan: prediksi bulanan melewati budget.\n";
        adaPeringatan = true;
    }

    if (adaPeringatan == false) {
        cout << "Pengeluaran masih aman.\n";
    }
}

// 8. DAILY REMINDER
void dailyReminder() {
    cout << "\n8. DAILY REMINDER\n";
    cout << "Jangan lupa mencatat pengeluaran setiap hari.\n";
}

// 9. SPENDING RECOMMENDATION
void spendingRecommendation() {
    cout << "\n9. SPENDING RECOMMENDATION\n";

    if (prediksiBulanan > budgetBulanan) {
        cout << "Kurangi pengeluaran harian agar sesuai budget.\n";
    } else {
        cout << "Pertahankan pola pengeluaran saat ini.\n";
    }

    cout << "Hari dengan pengeluaran terbesar adalah "
         << hari[indexTerbesar] << ".\n";
}

int main() {
    cout << fixed << setprecision(0);

    dailyExpenseInput();        // Tahap 1
    expenseHistory();           // Tahap 2
    expenseStatistics();        // Tahap 3
    expensePrediction();        // Tahap 4
    dailyMonthlyBudget();       // Tahap 5
    budgetMonitoring();         // Tahap 6
    budgetWarning();            // Tahap 7
    dailyReminder();            // Tahap 8
    spendingRecommendation();   // Tahap 9

    return 0;
}
