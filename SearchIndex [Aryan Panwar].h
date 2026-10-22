#pragma once
#include "Document [Aditya Negi].h"
#include "Tokenizer [Aryan Panwar].h"
#include <cmath>
#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rar {

// Inverted index: term -> document ID -> number of occurrences.
class SearchIndex {
public:
    using PostingList = std::unordered_map<int, int>;

private:
    std::unordered_map<std::string, PostingList> index_;
    std::unordered_map<int, std::unordered_map<std::string, int>> documentTerms_;
    std::size_t documentCount_{0};
    const Tokenizer& tokenizer_;

public:
    explicit SearchIndex(const Tokenizer& tokenizer) : tokenizer_(tokenizer) {}

    void clear() {
        index_.clear();
        documentTerms_.clear();
        documentCount_ = 0;
    }

    void add(const Document& document) {
        remove(document.id());
        auto tokens = tokenizer_.tokenize(document.title() + " " + document.content(), true);
        auto& frequencies = documentTerms_[document.id()];
        for (const auto& term : tokens) {
            ++frequencies[term];
            ++index_[term][document.id()];
        }
        ++documentCount_;
    }

    void remove(int documentId) {
        auto old = documentTerms_.find(documentId);
        if (old == documentTerms_.end()) return;
        for (const auto& termCount : old->second) {
            auto termIt = index_.find(termCount.first);
            if (termIt != index_.end()) {
                termIt->second.erase(documentId);
                if (termIt->second.empty()) index_.erase(termIt);
            }
        }
        documentTerms_.erase(old);
        if (documentCount_ > 0) --documentCount_;
    }

    void rebuild(const std::vector<Document>& documents) {
        clear();
        for (const auto& document : documents) add(document);
    }

    const PostingList* postings(const std::string& term) const {
        auto it = index_.find(term);
        return it == index_.end() ? nullptr : &it->second;
    }

    std::size_t documentFrequency(const std::string& term) const {
        auto it = index_.find(term);
        return it == index_.end() ? 0 : it->second.size();
    }

    int termFrequency(int documentId, const std::string& term) const {
        auto d = documentTerms_.find(documentId);
        if (d == documentTerms_.end()) return 0;
        auto t = d->second.find(term);
        return t == d->second.end() ? 0 : t->second;
    }

    double inverseDocumentFrequency(const std::string& term) const {
        const double n = static_cast<double>(documentCount_);
        const double df = static_cast<double>(documentFrequency(term));
        return std::log(1.0 + (n - df + 0.5) / (df + 0.5));
    }

    std::size_t size() const { return documentCount_; }
    std::size_t vocabularySize() const { return index_.size(); }

    std::unordered_set<int> candidates(const std::vector<std::string>& terms,
                                       bool requireAll = false) const {
        std::unordered_set<int> result;
        bool first = true;
        for (const auto& term : terms) {
            const auto* list = postings(term);
            if (!list) {
                if (requireAll) return {};
                continue;
            }
            std::unordered_set<int> ids;
            for (const auto& pair : *list) ids.insert(pair.first);
            if (first) {
                result = std::move(ids);
                first = false;
            } else if (requireAll) {
                for (auto it = result.begin(); it != result.end();) {
                    if (!ids.count(*it)) it = result.erase(it);
                    else ++it;
                }
            } else {
                result.insert(ids.begin(), ids.end());
            }
        }
        return result;
    }
};

} // namespace rar
