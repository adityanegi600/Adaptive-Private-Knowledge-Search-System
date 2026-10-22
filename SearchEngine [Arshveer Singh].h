#pragma once
#include "Config [Arshveer Singh].h"
#include "Document [Aditya Negi].h"
#include "FeedbackLearner [Anshuman Singh].h"
#include "Ranker [Anshuman Singh].h"
#include "SearchIndex [Aryan Panwar].h"
#include "StorageManager [Aditya Negi].h"
#include "Tokenizer [Aryan Panwar].h"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace rar {

class SearchEngine {
private:
    Tokenizer tokenizer_;
    std::vector<Document> documents_;
    SearchIndex index_;
    Ranker ranker_;
    FeedbackLearner learner_;
    StorageManager storage_;
    std::vector<std::string> history_;
    int nextId_{1};

    Document* findMutable(int id) {
        for (auto& doc : documents_) if (doc.id() == id) return &doc;
        return nullptr;
    }

public:
    SearchEngine()
        : index_(tokenizer_), ranker_(tokenizer_, index_),
          learner_(ranker_, tokenizer_) {
        storage_.ensureDataDirectory();
        documents_ = storage_.loadDocuments(DOCUMENT_DB);
        for (const auto& d : documents_) nextId_ = std::max(nextId_, d.id() + 1);
        index_.rebuild(documents_);
        history_ = storage_.loadHistory(HISTORY_DB);
        storage_.loadLearning(FEEDBACK_DB, ranker_, learner_);
    }

    const std::vector<Document>& documents() const { return documents_; }
    const std::vector<std::string>& history() const { return history_; }
    std::size_t vocabularySize() const { return index_.vocabularySize(); }

    bool addDocument(const std::string& title, const std::string& content,
                     const std::string& source = "manual",
                     const std::vector<std::string>& tags = {}) {
        if (title.empty() || content.empty()) return false;
        Document doc(nextId_++, title, content, source, tags);
        documents_.push_back(doc);
        index_.add(doc);
        return saveAll();
    }

    bool importTextFile(const std::string& path) {
        std::ifstream in(path);
        if (!in) return false;
        std::string content, line;
        while (std::getline(in, line)) {
            if (!content.empty()) content += '\n';
            content += line;
        }
        std::string title = path;
        const std::string::size_type slash = title.find_last_of("/\\");
        if (slash != std::string::npos) title = title.substr(slash + 1);
        return addDocument(title, content, path);
    }

    bool deleteDocument(int id) {
        auto it = std::find_if(documents_.begin(), documents_.end(),
                               [id](const Document& d) { return d.id() == id; });
        if (it == documents_.end()) return false;
        index_.remove(id);
        documents_.erase(it);
        return saveAll();
    }

    bool updateDocument(int id, const std::string& title, const std::string& content) {
        Document* doc = findMutable(id);
        if (!doc || title.empty() || content.empty()) return false;
        doc->setTitle(title);
        doc->setContent(content);
        index_.rebuild(documents_);
        return saveAll();
    }

    std::vector<RankedDocument> search(const std::string& query,
                                       bool requireAll = false,
                                       std::size_t limit = MAX_RESULTS) {
        const auto terms = tokenizer_.tokenize(query, true);
        if (terms.empty()) return {};
        history_.push_back(query);
        if (history_.size() > 100) history_.erase(history_.begin());
        storage_.saveHistory(HISTORY_DB, history_);

        // OR is default. AND mode uses the index's intersection candidate set.
        auto ranked = ranker_.rank(documents_, terms, false, limit);
        if (requireAll) {
            const auto required = index_.candidates(terms, true);
            ranked.erase(std::remove_if(ranked.begin(), ranked.end(),
                [&required](const RankedDocument& item) {
                    return !required.count(item.documentId);
                }), ranked.end());
        }
        return ranked;
    }

    const Document* getDocument(int id) const {
        for (const auto& d : documents_) if (d.id() == id) return &d;
        return nullptr;
    }

    bool feedback(int id, const std::string& query, bool useful) {
        if (!getDocument(id)) return false;
        learner_.record(id, query, useful, "local-session");
        return storage_.saveLearning(FEEDBACK_DB, ranker_, learner_);
    }

    std::vector<Document> findByTag(const std::string& tag) const {
        std::vector<Document> found;
        for (const auto& doc : documents_) if (doc.containsTag(tag)) found.push_back(doc);
        return found;
    }

    bool saveAll() {
        const bool docsOk = storage_.saveDocuments(DOCUMENT_DB, documents_);
        const bool learnOk = storage_.saveLearning(FEEDBACK_DB, ranker_, learner_);
        const bool histOk = storage_.saveHistory(HISTORY_DB, history_);
        return docsOk && learnOk && histOk;
    }

    void printStatistics() const {
        std::cout << "\n--- System statistics ---\n"
                  << "Documents: " << documents_.size() << '\n'
                  << "Vocabulary terms: " << index_.vocabularySize() << '\n'
                  << "Searches stored: " << history_.size() << '\n'
                  << "Data location: ./data/\n";
    }
};

} // namespace rar
