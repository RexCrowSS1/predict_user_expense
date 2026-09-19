#include "models/linear_regression/linear_model.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

const char* LinearRegressionModel::name() const {
    return "Seasonal Linear Regression";
}

void LinearRegressionModel::train(const vector<Record>& data) {
    struct Sum {
        double n = 0, x = 0, y = 0, xx = 0, xy = 0;
    };
    array<array<Sum, PHASES>, DAYS> sums{};
    fitted = false;
    latestWeek = 0;

    for (const Record& row : data) {
        const double y = row.total();
        if (row.week <= 0 || row.day >= DAYS || !isfinite(y) || y < 0)
            throw invalid_argument("Data training tidak valid.");

        const size_t phase = static_cast<size_t>(
            (row.week - 1) % static_cast<int>(PHASES));
        Sum& s = sums[row.day][phase];
        const double x = row.week;
        s.n++; s.x += x; s.y += y; s.xx += x * x; s.xy += x * y;
        latestWeek = max(latestWeek, row.week);
    }

    for (size_t day = 0; day < DAYS; day++) {
        for (size_t phase = 0; phase < PHASES; phase++) {
            const Sum& s = sums[day][phase];
            if (s.n == 0) throw invalid_argument("Data training belum lengkap.");

            const double denominator = s.n * s.xx - s.x * s.x;
            Line& line = lines[day][phase];
            line.slope = abs(denominator) < 1e-12
                       ? 0
                       : (s.n * s.xy - s.x * s.y) / denominator;
            line.intercept = (s.y - line.slope * s.x) / s.n;
        }
    }
    fitted = true;
}

double LinearRegressionModel::predict(int week, size_t day) const {
    if (!fitted) throw logic_error("Model belum dilatih.");
    if (week <= latestWeek || day >= DAYS)
        throw invalid_argument("Prediksi hanya untuk minggu masa depan.");

    const size_t phase = static_cast<size_t>(
        (week - 1) % static_cast<int>(PHASES));
    const Line& line = lines[day][phase];
    return max(0.0, line.intercept + line.slope * week);
}

int LinearRegressionModel::lastWeek() const {
    if (!fitted) throw logic_error("Model belum dilatih.");
    return latestWeek;
}
