#ifndef FORECAST_MODEL_H
#define FORECAST_MODEL_H

#include "common/expense_data.h"

class ForecastModel {
public:
    virtual ~ForecastModel() = default;
    virtual const char* name() const = 0;
    virtual void train(const std::vector<Record>& data) = 0;
    virtual double predict(int week, std::size_t day) const = 0;
    virtual int lastWeek() const = 0;
};

#endif
