#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>

namespace rar {

class Tokenizer {
private:
    std::unordered_set<std::string> stopWords_{
        "a","an","and","are","as","at","be","been","but","by","for","from",
        "has","have","he","in","is","it","its","of","on","or","that","the",
        "their","this","to","was","were","will","with","you","your","i","we"
    };

public:
    Tokenizer() = default;

    std::string normalize(const std::string& input) const {
        std::string output;
        output.reserve(input.size());
        for (unsigned char ch : input) {
            if (std::isalnum(ch)) output.push_back(static_cast<char>(std::tolower(ch)));
            else output.push_back(' ');
        }
        return output;
    }

    std::vector<std::string> tokenize(const std::string& input,
                                      bool removeStopWords = false) const {
        std::vector<std::string> tokens;
        std::string normalized = normalize(input);
        std::string current;
        for (char ch : normalized) {
            if (ch == ' ') {
                if (!current.empty()) {
                    if (!removeStopWords || !isStopWord(current)) tokens.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(ch);
            }
        }
        if (!current.empty() && (!removeStopWords || !isStopWord(current)))
            tokens.push_back(current);
        return tokens;
    }

    bool isStopWord(const std::string& token) const {
        return stopWords_.find(token) != stopWords_.end();
    }

    void addStopWord(const std::string& word) {
        stopWords_.insert(normalize(word));
    }

    std::unordered_set<std::string> uniqueTerms(const std::string& input) const {
        const auto tokens = tokenize(input, true);
        return {tokens.begin(), tokens.end()};
    }
};

} // namespace rar
