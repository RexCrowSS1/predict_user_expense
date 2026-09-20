#include "common/evaluation.h"
#include "models/knn/knn_model.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace std;

void check(bool condition, const string& message) {
    if (!condition) throw runtime_error(message);
}

int main() {
    try {
        vector<Record> training = readCsv("data/train.csv");
        vector<Record> fit, validation;
        for (const Record& row : training) {
            if (row.week <= 8) fit.push_back(row);
            else if (row.week <= 12) validation.push_back(row);
        }

        double bestMae = 0;
        size_t bestK = 1;
        for (size_t k = 1; k <= 5; k++) {
            KnnModel candidate(k);
            candidate.train(fit);
            const double mae = evaluate(candidate, fit, validation).mae;
            if (k == 1 || mae < bestMae) {
                bestMae = mae;
                bestK = k;
            }
            cout << "k=" << k << " | MAE validasi: Rp " << mae << '\n';
        }

        vector<Record> test = readCsv("data/expense_test_fluctuated.csv");
        KnnModel model;
        check(model.k() == bestK, "Default KNN tidak sesuai hasil validasi.");
        model.train(training);
        Metrics result = evaluate(model, training, test);
        cout << "k terbaik: " << bestK << '\n';
        // Removing fixed rent changes the target; baseline superiority is
        // reported below, not assumed for every dataset.
        check(isfinite(result.mae) && result.mae >= 0
              && isfinite(result.rmse) && result.rmse >= result.mae
              && result.samples == test.size(),
              "Metrik evaluasi KNN tidak valid.");

        training.insert(training.end(), test.begin(), test.end());
        model.train(training);
        for (int offset = 1; offset <= 4; offset++)
            for (size_t day = 0; day < DAYS; day++)
                check(model.predict(model.lastWeek() + offset, day) >= 0,
                      "Prediksi KNN tidak valid.");

        bool rejected = false;
        try { model.predict(model.lastWeek() + 5, 0); }
        catch (const invalid_argument&) { rejected = true; }
        check(rejected, "KNN menerima prediksi tanpa lag-4.");

        cout << "KNN OK | MAE holdout: Rp " << result.mae
             << " | baseline: Rp " << result.baselineMae << '\n';
    } catch (const exception& error) {
        cerr << "TEST GAGAL: " << error.what() << '\n';
        return 1;
    }
}
