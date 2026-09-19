#ifndef LINEAR_MODEL_H
#define LINEAR_MODEL_H

#include "common/forecast_model.h"

class LinearRegressionModel final : public ForecastModel {
    struct Line {
        double intercept = 0;
        double slope = 0;
    };

    std::array<std::array<Line, PHASES>, DAYS> lines{};
    bool fitted = false;
    int latestWeek = 0;

public:
    const char* name() const override;
    void train(const std::vector<Record>& data) override;
    double predict(int week, std::size_t day) const override;
    int lastWeek() const override;
};

#endif
