#include "models/knn/knn_model.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace {

const Record* findRecord(const vector<Record>& data, int week, size_t day) {
    for (const Record& row : data)
        if (row.week == week && row.day == day) return &row;
    return nullptr;
}

} // namespace

KnnModel::KnnModel(size_t k)
    : neighbors(k), modelName("KNN Regression (k=" + to_string(k) + ")") {
    if (k == 0) throw invalid_argument("Nilai k harus lebih dari nol.");
}

const char* KnnModel::name() const {
    return modelName.c_str();
}

KnnModel::Feature KnnModel::makeFeature(int week, size_t day,
                                       double lag) const {
    Feature feature{};
    feature[day] = 2.0;
    const size_t phase = static_cast<size_t>(
        (week - 1) % static_cast<int>(PHASES));
    feature[DAYS + phase] = 1.0;
    feature[DAYS + PHASES] = (week - meanWeek) / scaleWeek;
    feature[DAYS + PHASES + 1] = (lag - meanLag) / scaleLag;
    return feature;
}

void KnnModel::train(const vector<Record>& data) {
    struct RawSample {
        const Record* row;
        double lag;
    };

    fitted = false;
    history = data;
    samples.clear();
    latestWeek = 0;
    vector<RawSample> raw;

    for (const Record& row : history) {
        const double total = row.total();
        if (row.week <= 0 || row.day >= DAYS || !isfinite(total) || total < 0)
            throw invalid_argument("Data training tidak valid.");
        latestWeek = max(latestWeek, row.week);
        if (const Record* lag = findRecord(history, row.week - 4, row.day))
            raw.push_back({&row, lag->total()});
    }
    if (raw.size() < neighbors)
        throw invalid_argument("Data training tidak cukup untuk nilai k.");

    meanWeek = meanLag = 0;
    for (const RawSample& item : raw) {
        meanWeek += item.row->week;
        meanLag += item.lag;
    }
    const double count = static_cast<double>(raw.size());
    meanWeek /= count;
    meanLag /= count;

    double weekVariance = 0, lagVariance = 0;
    for (const RawSample& item : raw) {
        weekVariance += pow(item.row->week - meanWeek, 2);
        lagVariance += pow(item.lag - meanLag, 2);
    }
    scaleWeek = sqrt(weekVariance / count);
    scaleLag = sqrt(lagVariance / count);
    if (scaleWeek < 1e-12) scaleWeek = 1;
    if (scaleLag < 1e-12) scaleLag = 1;

    for (const RawSample& item : raw)
        samples.push_back({makeFeature(item.row->week, item.row->day, item.lag),
                           item.row->total(), item.row->week, item.row->day});
    fitted = true;
}

double KnnModel::predict(int week, size_t day) const {
    if (!fitted) throw logic_error("Model belum dilatih.");
    if (week <= latestWeek || day >= DAYS)
        throw invalid_argument("Prediksi hanya untuk minggu masa depan.");

    const Record* lag = findRecord(history, week - 4, day);
    if (!lag) throw invalid_argument("Data lag empat minggu tidak tersedia.");
    const Feature query = makeFeature(week, day, lag->total());

    struct Neighbor {
        double distance, target;
        int week;
        size_t day;
    };
    vector<Neighbor> nearest;
    nearest.reserve(samples.size());
    for (const Sample& sample : samples) {
        double distance = 0;
        for (size_t feature = 0; feature < FEATURES; feature++)
            distance += pow(query[feature] - sample.feature[feature], 2);
        nearest.push_back({distance, sample.target, sample.week, sample.day});
    }
    sort(nearest.begin(), nearest.end(), [](const Neighbor& a,
                                             const Neighbor& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        if (a.week != b.week) return a.week > b.week;
        if (a.day != b.day) return a.day < b.day;
        return a.target < b.target;
    });

    double prediction = 0;
    for (size_t index = 0; index < neighbors; index++)
        prediction += nearest[index].target;
    return max(0.0, prediction / static_cast<double>(neighbors));
}

int KnnModel::lastWeek() const {
    if (!fitted) throw logic_error("Model belum dilatih.");
    return latestWeek;
}

size_t KnnModel::k() const {
    return neighbors;
}
