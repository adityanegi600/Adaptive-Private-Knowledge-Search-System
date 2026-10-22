#pragma once
#include "Config [Arshveer Singh].h"
#include "Ranker [Anshuman Singh].h"
#include "Tokenizer [Aryan Panwar].h"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace rar {

struct FeedbackEvent {
    int documentId{0};
    bool useful{false};
    std::string query;
    std::string timestamp;
    FeedbackEvent() : documentId(0), useful(false) {}
    FeedbackEvent(int id, bool isUseful, const std::string& q, const std::string& ts)
        : documentId(id), useful(isUseful), query(q), timestamp(ts) {}
};

// Lightweight online ranking adjustment. This is heuristic feedback, not model training.
class FeedbackLearner {
private:
    Ranker& ranker_;
    const Tokenizer& tokenizer_;
    std::unordered_map<int, double> documentRelevance_;
    std::vector<FeedbackEvent> events_;
    double learningRate_{0.05};

public:
    FeedbackLearner(Ranker& ranker, const Tokenizer& tokenizer)
        : ranker_(ranker), tokenizer_(tokenizer) {}

    void setLearningRate(double value) {
        learningRate_ = std::max(0.001, std::min(value, 0.5));
    }
    double learningRate() const { return learningRate_; }

    void record(int documentId, const std::string& query, bool useful,
                const std::string& timestamp) {
        const double delta = useful ? 1.0 : -0.5;
        documentRelevance_[documentId] += delta;
        ranker_.setDocumentBoost(documentId, documentRelevance_[documentId]);
        const double termDelta = useful ? learningRate_ : -learningRate_ * 0.5;
        for (const auto& term : tokenizer_.tokenize(query, true)) {
            const double next = ranker_.termWeight(term) + termDelta;
            ranker_.setTermWeight(term, next);
        }
        events_.push_back({documentId, useful, query, timestamp});
    }

    const std::unordered_map<int, double>& relevanceMap() const { return documentRelevance_; }
    const std::vector<FeedbackEvent>& events() const { return events_; }

    void restoreDocumentRelevance(int id, double value) {
        documentRelevance_[id] = value;
        ranker_.setDocumentBoost(id, value);
    }
    void restoreEvent(const FeedbackEvent& event) { events_.push_back(event); }
};

} // namespace rar
