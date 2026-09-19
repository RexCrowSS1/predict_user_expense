#ifndef EVALUATION_H
#define EVALUATION_H

#include "common/forecast_model.h"

struct Metrics {
    std::size_t samples = 0;
    double mae = 0;
    double rmse = 0;
    double wape = 0;
    double baselineMae = 0;
};

Metrics evaluate(const ForecastModel& model,
                 const std::vector<Record>& training,
                 const std::vector<Record>& test);

#endif
