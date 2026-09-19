#include "common/evaluation.h"

#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

double seasonalBaseline(const vector<Record>& data, int week, size_t day) {
    for (const Record& row : data)
        if (row.week == week - 4 && row.day == day) return row.total();
    throw invalid_argument("Baseline musiman tidak tersedia.");
}

} // namespace

Metrics evaluate(const ForecastModel& model,
                 const vector<Record>& training,
                 const vector<Record>& test) {
    if (test.empty()) throw invalid_argument("Data test kosong.");

    Metrics result;
    double absoluteError = 0, squaredError = 0, actualTotal = 0;
    double baselineError = 0;

    for (const Record& row : test) {
        const double actual = row.total();
        const double error = model.predict(row.week, row.day) - actual;
        absoluteError += abs(error);
        squaredError += error * error;
        actualTotal += actual;
        baselineError += abs(seasonalBaseline(training, row.week, row.day)
                             - actual);
    }

    result.samples = test.size();
    const double count = static_cast<double>(result.samples);
    result.mae = absoluteError / count;
    result.rmse = sqrt(squaredError / count);
    result.wape = actualTotal > 0 ? 100.0 * absoluteError / actualTotal : 0;
    result.baselineMae = baselineError / count;
    return result;
}
