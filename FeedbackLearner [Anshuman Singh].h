#pragma once                                    // Prevents file from being included twice
#include "Config.h"
#include "Ranker.h"                             // To update document boosts and word weights
#include "Tokenizer.h"                          // To split search queries into words
#include <algorithm>                            // For std::max and std::min
#include <string>
#include <unordered_map>
#include <vector>

namespace rar {

// Stores ONE user feedback rating
struct FeedbackEvent {
    int documentId{0};                          // Rated document ID
    bool useful{false};                         // true = useful, false = not useful
    std::string query, timestamp;               // Search query & time of rating
    FeedbackEvent() = default;                  // Default empty constructor
    FeedbackEvent(int id, bool u, const std::string& q, const std::string& ts)
        : documentId(id), useful(u), query(q), timestamp(ts) {} // 4-value constructor
};

// Adjusts Ranker scores based on user feedback ("Reflect" step)
class FeedbackLearner {
private:
    Ranker& ranker_;                            // Reference to Ranker
    const Tokenizer& tokenizer_;                // Reference to Tokenizer
    std::unordered_map<int, double> documentRelevance_; // Saved boost per doc ID
    std::vector<FeedbackEvent> events_;         // List of all feedback events
    double learningRate_{0.05};                 // Speed of word weight change

public:
    FeedbackLearner(Ranker& r, const Tokenizer& t) : ranker_(r), tokenizer_(t) {} // Constructor

    // Set/get learning rate (kept between 0.001 and 0.5)
    void setLearningRate(double v) { learningRate_ = std::max(0.001, std::min(v, 0.5)); }
    double learningRate() const { return learningRate_; }

    // Main function: updates doc boost, updates word weights, and saves the event
    void record(int id, const std::string& q, bool useful, const std::string& ts) {
        documentRelevance_[id] += useful ? 1.0 : -0.5;          // +1.0 if useful, -0.5 if not
        ranker_.setDocumentBoost(id, documentRelevance_[id]);   // Send new boost to Ranker

        double delta = useful ? learningRate_ : -learningRate_ * 0.5; // Word weight change
        for (const auto& w : tokenizer_.tokenize(q, true))      // Loop each query word
            ranker_.setTermWeight(w, ranker_.termWeight(w) + delta); // Update word weight

        events_.push_back({id, useful, q, ts});                 // Save to history
    }

    // Getters used when saving to disk
    const std::unordered_map<int, double>& relevanceMap() const { return documentRelevance_; }
    const std::vector<FeedbackEvent>& events() const { return events_; }

    // Restorers used when loading from disk at startup
    void restoreDocumentRelevance(int id, double v) { documentRelevance_[id] = v; ranker_.setDocumentBoost(id, v); }
    void restoreEvent(const FeedbackEvent& e) { events_.push_back(e); }
};

} // namespace rar
