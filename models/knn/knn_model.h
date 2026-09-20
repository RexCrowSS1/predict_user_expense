#ifndef KNN_MODEL_H
#define KNN_MODEL_H

#include "common/forecast_model.h"

class KnnModel final : public ForecastModel {
    static constexpr std::size_t FEATURES = DAYS + PHASES + 2;
    using Feature = std::array<double, FEATURES>;

    struct Sample {
        Feature feature{};
        double target = 0;
        int week = 0;
        std::size_t day = 0;
    };

    std::vector<Record> history;
    std::vector<Sample> samples;
    std::size_t neighbors;
    std::string modelName;
    double meanWeek = 0, scaleWeek = 1;
    double meanLag = 0, scaleLag = 1;
    int latestWeek = 0;
    bool fitted = false;

    Feature makeFeature(int week, std::size_t day, double lag) const;

public:
    explicit KnnModel(std::size_t k = 3);
    const char* name() const override;
    void train(const std::vector<Record>& data) override;
    double predict(int week, std::size_t day) const override;
    int lastWeek() const override;
    std::size_t k() const;
};

#endif
