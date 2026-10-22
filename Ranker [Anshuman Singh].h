#pragma once
#include "Document [Aditya Negi].h"
#include "SearchIndex [Aryan Panwar].h"
#include "Tokenizer [Aryan Panwar].h"
#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rar {

struct RankedDocument {
    int documentId{0};
    double score{0.0};
    std::vector<std::string> matchedTerms;
};

class Ranker {
private:
    const Tokenizer& tokenizer_;
    const SearchIndex& index_;
    std::unordered_map<std::string, double> termWeights_;
    std::unordered_map<int, double> documentBoosts_;

public:
    Ranker(const Tokenizer& tokenizer, const SearchIndex& index)
        : tokenizer_(tokenizer), index_(index) {}

    void setTermWeight(const std::string& term, double weight) {
        termWeights_[term] = std::max(0.20, std::min(weight, 3.00));
    }
    double termWeight(const std::string& term) const {
        auto it = termWeights_.find(term);
        return it == termWeights_.end() ? 1.0 : it->second;
    }
    const std::unordered_map<std::string, double>& termWeights() const { return termWeights_; }

    void setDocumentBoost(int id, double boost) { documentBoosts_[id] = boost; }
    double documentBoost(int id) const {
        auto it = documentBoosts_.find(id);
        return it == documentBoosts_.end() ? 0.0 : it->second;
    }
    const std::unordered_map<int, double>& documentBoosts() const { return documentBoosts_; }

    double score(const Document& doc, const std::vector<std::string>& queryTerms,
                 bool phraseMatch = false) const {
        double total = 0.0;
        std::unordered_set<std::string> uniqueQuery(queryTerms.begin(), queryTerms.end());
        const auto titleTerms = tokenizer_.tokenize(doc.title(), true);
        const auto bodyTerms = tokenizer_.tokenize(doc.content(), true);
        std::unordered_map<std::string, int> titleCounts, bodyCounts;
        for (const auto& t : titleTerms) ++titleCounts[t];
        for (const auto& t : bodyTerms) ++bodyCounts[t];

        for (const auto& term : uniqueQuery) {
            const int tfTitle = titleCounts.count(term) ? titleCounts.at(term) : 0;
            const int tfBody = bodyCounts.count(term) ? bodyCounts.at(term) : 0;
            const double tf = (3.0 * tfTitle) + tfBody;
            if (tf > 0.0)
                total += tf * index_.inverseDocumentFrequency(term) * termWeight(term);
        }
        total += documentBoost(doc.id());
        if (phraseMatch && !queryTerms.empty()) total += 2.0;
        return total;
    }

    std::vector<RankedDocument> rank(const std::vector<Document>& documents,
                                     const std::vector<std::string>& queryTerms,
                                     bool phraseMatch = false,
                                     std::size_t limit = 20) const {
        std::vector<RankedDocument> results;
        const auto candidates = index_.candidates(queryTerms, false);
        for (const auto& doc : documents) {
            if (!candidates.count(doc.id())) continue;
            RankedDocument item;
            item.documentId = doc.id();
            item.score = score(doc, queryTerms, phraseMatch);
            for (const auto& term : queryTerms) {
                if (doc.title().find(term) != std::string::npos ||
                    doc.content().find(term) != std::string::npos)
                    item.matchedTerms.push_back(term);
            }
            if (item.score > 0.0) results.push_back(std::move(item));
        }
        std::sort(results.begin(), results.end(),
                  [](const RankedDocument& a, const RankedDocument& b) {
                      if (a.score == b.score) return a.documentId < b.documentId;
                      return a.score > b.score;
                  });
        if (results.size() > limit) results.resize(limit);
        return results;
    }
};

} // namespace rar
