#include "common/evaluation.h"
#include "models/linear_regression/linear_model.h"

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
        vector<Record> test = readCsv("data/expense_test_fluctuated.csv");
        LinearRegressionModel model;
        model.train(training);
        Metrics result = evaluate(model, training, test);
        check(isfinite(result.mae) && result.mae < result.baselineMae,
              "Linear Regression tidak mengalahkan baseline.");

        training.insert(training.end(), test.begin(), test.end());
        model.train(training);
        for (int offset = 1; offset <= 4; offset++)
            for (size_t day = 0; day < DAYS; day++)
                check(model.predict(model.lastWeek() + offset, day) >= 0,
                      "Prediksi Linear Regression tidak valid.");

        cout << "Linear Regression OK | MAE: Rp " << result.mae
             << " | baseline: Rp " << result.baselineMae << '\n';
    } catch (const exception& error) {
        cerr << "TEST GAGAL: " << error.what() << '\n';
        return 1;
    }
}
