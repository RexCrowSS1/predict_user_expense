CXX ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -I.

BUILD := build
APP := $(BUILD)/expense_tracker
LINEAR_TEST := $(BUILD)/linear_model_test
KNN_TEST := $(BUILD)/knn_model_test
DATA_TEST := $(BUILD)/expense_data_test
PYTHON ?= python3
COMMON := common/expense_data.cpp common/evaluation.cpp
LINEAR := models/linear_regression/linear_model.cpp
KNN := models/knn/knn_model.cpp
HEADERS := common/expense_data.h common/evaluation.h common/forecast_model.h \
           models/linear_regression/linear_model.h models/knn/knn_model.h

.PHONY: all run run-linear run-knn test test-linear test-knn test-data test-app clean

all: $(APP)

$(BUILD):
	mkdir -p $(BUILD)

$(APP): app/main.cpp $(COMMON) $(LINEAR) $(KNN) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) app/main.cpp $(COMMON) $(LINEAR) $(KNN) -o $@

$(LINEAR_TEST): tests/linear_model_test.cpp $(COMMON) $(LINEAR) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) tests/linear_model_test.cpp $(COMMON) $(LINEAR) -o $@

$(KNN_TEST): tests/knn_model_test.cpp $(COMMON) $(KNN) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) tests/knn_model_test.cpp $(COMMON) $(KNN) -o $@

$(DATA_TEST): tests/expense_data_test.cpp common/expense_data.cpp common/expense_data.h | $(BUILD)
	$(CXX) $(CXXFLAGS) tests/expense_data_test.cpp common/expense_data.cpp -o $@

run: run-linear

run-linear: $(APP)
	./$(APP) linear

run-knn: $(APP)
	./$(APP) knn

test: test-linear test-knn test-data test-app

test-linear: $(LINEAR_TEST)
	./$(LINEAR_TEST)

test-knn: $(KNN_TEST)
	./$(KNN_TEST)

test-data: $(DATA_TEST)
	./$(DATA_TEST)

test-app: $(APP)
	$(PYTHON) tests/app_test.py

clean:
	rm -f $(APP) $(LINEAR_TEST) $(KNN_TEST) $(DATA_TEST)
	rmdir $(BUILD) 2>/dev/null || true
