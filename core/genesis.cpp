
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// GENESIS V0.3
// Controlled synthetic research benchmark.
// Candidate representations are predefined, not open-ended discovery.

struct DataSet {
    std::vector<double> x;
    std::vector<double> y;
};

static double hidden_law(double x) {
    return 0.7 * x * x
         + 1.4 * std::sin(1.7 * x)
         + 0.3 * x
         + 2.0;
}

static DataSet make_data(int count, std::mt19937& rng) {
    std::uniform_real_distribution<double> x_dist(-3.0, 3.0);
    std::normal_distribution<double> noise(0.0, 0.08);

    DataSet data;

    for (int i = 0; i < count; ++i) {
        double x = x_dist(rng);
        data.x.push_back(x);
        data.y.push_back(hidden_law(x) + noise(rng));
    }

    return data;
}

static void append_data(
    const DataSet& source,
    std::vector<double>& x,
    std::vector<double>& y
) {
    x.insert(x.end(), source.x.begin(), source.x.end());
    y.insert(y.end(), source.y.begin(), source.y.end());
}

static double rmse(
    const std::vector<double>& actual,
    const std::vector<double>& predicted
) {
    if (actual.empty() || actual.size() != predicted.size()) {
        return 1e9;
    }

    double sum = 0.0;

    for (size_t i = 0; i < actual.size(); ++i) {
        double error = actual[i] - predicted[i];
        sum += error * error;
    }

    return std::sqrt(sum / actual.size());
}

static double feature(int id, double x) {
    switch (id) {
        case 0: return 1.0;
        case 1: return x;
        case 2: return x * x;
        case 3: return std::sin(1.7 * x);
        default: return std::cos(1.7 * x);
    }
}

static std::vector<double> fit(
    const std::vector<double>& xs,
    const std::vector<double>& ys,
    const std::vector<int>& features
) {
    int n = static_cast<int>(features.size());

    std::vector<std::vector<double>> a(
        n, std::vector<double>(n + 1, 0.0)
    );

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (size_t k = 0; k < xs.size(); ++k) {
                a[i][j] += feature(features[i], xs[k])
                         * feature(features[j], xs[k]);
            }
        }

        for (size_t k = 0; k < xs.size(); ++k) {
            a[i][n] += feature(features[i], xs[k]) * ys[k];
        }

        a[i][i] += 1e-9;
    }

    for (int i = 0; i < n; ++i) {
        int pivot = i;

        for (int r = i + 1; r < n; ++r) {
            if (std::abs(a[r][i]) > std::abs(a[pivot][i])) {
                pivot = r;
            }
        }

        std::swap(a[i], a[pivot]);

        double divisor = a[i][i];

        if (std::abs(divisor) < 1e-12) {
            return std::vector<double>(n, 0.0);
        }

        for (int c = i; c <= n; ++c) {
            a[i][c] /= divisor;
        }

        for (int r = 0; r < n; ++r) {
            if (r == i) continue;

            double factor = a[r][i];

            for (int c = i; c <= n; ++c) {
                a[r][c] -= factor * a[i][c];
            }
        }
    }

    std::vector<double> weights(n);

    for (int i = 0; i < n; ++i) {
        weights[i] = a[i][n];
    }

    return weights;
}

static double predict(
    double x,
    const std::vector<int>& features,
    const std::vector<double>& weights
) {
    double result = 0.0;

    for (size_t i = 0; i < features.size(); ++i) {
        result += weights[i] * feature(features[i], x);
    }

    return result;
}

static double evaluate(
    const DataSet& data,
    const std::vector<int>& features,
    const std::vector<double>& weights
) {
    std::vector<double> predictions;

    for (double x : data.x) {
        predictions.push_back(predict(x, features, weights));
    }

    return rmse(data.y, predictions);
}

static double mean(const std::vector<double>& values) {
    double sum = 0.0;

    for (double value : values) {
        sum += value;
    }

    return values.empty() ? 0.0 : sum / values.size();
}

static double standard_deviation(const std::vector<double>& values) {
    if (values.size() < 2) return 0.0;

    double average = mean(values);
    double sum = 0.0;

    for (double value : values) {
        double difference = value - average;
        sum += difference * difference;
    }

    return std::sqrt(sum / (values.size() - 1));
}

extern "C" {

const char* genesis_version() {
    return "GENESIS V0.3 · C++20";
}

const char* genesis_run() {
    static std::string output;

    const std::vector<std::vector<int>> candidates = {
        {0, 1},
        {0, 1, 2},
        {0, 1, 3},
        {0, 1, 2, 3},
        {0, 1, 2, 3, 4}
    };

    std::vector<double> baseline_scores;
    std::vector<double> genesis_scores;
    std::vector<int> first_selected_features;

    const int experiments = 20;

    for (int experiment = 0; experiment < experiments; ++experiment) {
        std::mt19937 rng(
            20261109u + static_cast<unsigned int>(experiment * 7919)
        );

        DataSet training = make_data(100, rng);
        DataSet validation = make_data(100, rng);
        DataSet test = make_data(200, rng);

        // Select the model using validation data only.
        double best_validation_error = 1e99;
        std::vector<int> best_features;

        for (const auto& candidate : candidates) {
            auto weights = fit(
                training.x,
                training.y,
                candidate
            );

            double error = evaluate(
                validation,
                candidate,
                weights
            );

            if (error < best_validation_error) {
                best_validation_error = error;
                best_features = candidate;
            }
        }

        // Refit the selected model using training + validation.
        std::vector<double> combined_x;
        std::vector<double> combined_y;

        append_data(training, combined_x, combined_y);
        append_data(validation, combined_x, combined_y);

        const std::vector<int> baseline_features = {0, 1};

        auto baseline_weights = fit(
            combined_x,
            combined_y,
            baseline_features
        );

        auto genesis_weights = fit(
            combined_x,
            combined_y,
            best_features
        );

        // Final test data is used only for evaluation.
        double baseline_error = evaluate(
            test,
            baseline_features,
            baseline_weights
        );

        double genesis_error = evaluate(
            test,
            best_features,
            genesis_weights
        );

        baseline_scores.push_back(baseline_error);
        genesis_scores.push_back(genesis_error);

        if (experiment == 0) {
            first_selected_features = best_features;
        }
    }

    double baseline_average = mean(baseline_scores);
    double genesis_average = mean(genesis_scores);

    double improvement = baseline_average > 0.0
        ? (1.0 - genesis_average / baseline_average) * 100.0
        : 0.0;

    bool passed =
        genesis_average < 0.15 &&
        improvement > 50.0;

    const char* names[] = {
        "constant",
        "x",
        "x_squared",
        "sin_1.7x",
        "cos_1.7x"
    };

    std::ostringstream result;
    result << std::fixed << std::setprecision(6);

    result << "{";
    result << "\"version\":\"GENESIS V0.3 · C++20\",";
    result << "\"experiments\":" << experiments << ",";
    result << "\"training_readings_per_experiment\":100,";
    result << "\"validation_readings_per_experiment\":100,";
    result << "\"test_readings_per_experiment\":200,";

    result << "\"baseline_mean_rmse\":" << baseline_average << ",";
    result << "\"genesis_mean_rmse\":" << genesis_average << ",";
    result << "\"baseline_rmse_stddev\":"
           << standard_deviation(baseline_scores) << ",";
    result << "\"genesis_rmse_stddev\":"
           << standard_deviation(genesis_scores) << ",";

    result << "\"improvement_percent\":" << improvement << ",";

    result << "\"first_experiment_selected_features\":[";

    for (size_t i = 0; i < first_selected_features.size(); ++i) {
        if (i > 0) result << ",";
        result << "\"" << names[first_selected_features[i]] << "\"";
    }

    result << "],";

    result << "\"benchmark_pass\":"
           << (passed ? "true" : "false") << ",";

    result << "\"scientific_note\":"
           << "\"Synthetic benchmark with independent random noise. "
           << "Models are selected on validation data and evaluated on "
           << "untouched test data. Candidate features are predefined; "
           << "this does not demonstrate unrestricted scientific discovery "
           << "or new physics.\"";

    result << "}";

    output = result.str();
    return output.c_str();
}

}
