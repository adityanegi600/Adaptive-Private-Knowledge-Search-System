#pragma once                                    // Stops compiler from including this file twice
#include "Document.h"                           // Brings in the Document class from your project
#include "SearchIndex.h"                        // Brings in the SearchIndex class from your project
#include "Tokenizer.h"                          // Brings in the Tokenizer class (splits text into words)
#include <algorithm>                            // Gives us std::count, std::max, and std::min
#include <string>                               // Gives us std::string (text)
#include <unordered_map>                        // Gives us unordered_map (key -> value storage)
#include <unordered_set>                        // Gives us unordered_set (stores unique items only)
#include <vector>                               // Gives us std::vector (resizable array)

namespace rar {                                 // Groups all code under 'rar' to match other files

// Stores the final result for one document
struct RankedDocument {                         // A simple structure to hold 3 pieces of data
    int documentId{0};                          // ID number of the document (starts at 0)
    double score{0.0};                          // Calculated relevance score (starts at 0.0)
    std::vector<std::string> matchedTerms;      // List of search words found in this document
};                                              // End of RankedDocument struct

// =======================================================
// 1. LINKED LIST (Stores highest-scoring document first)
// =======================================================
class RankedList {                              // Custom Linked List class
    struct Node {                               // One box (node) in the linked list
        RankedDocument data;                    // Holds one RankedDocument
        Node* next;                             // Pointer to the next node in the chain
    };                                          // End of Node struct
    Node* head = nullptr;                       // Pointer to the first node (starts empty / NULL)

public:                                         // Functions below can be called from outside
    ~RankedList() {                             // Destructor: runs automatically to free memory
        while (head) {                          // Keep going while the list is not empty
            Node* t = head;                     // Save the current first node in temp pointer 't'
            head = head->next;                  // Move head forward to the next node
            delete t;                           // Delete the old first node from memory
        }                                       // End of while loop
    }                                           // End of destructor

    void insert(RankedDocument item) {          // Adds a document in descending order of score
        Node* n = new Node{item, nullptr};      // Create a new node on the heap with 'item'
        if (!head || item.score > head->data.score) { // If list is empty OR new score beats head
            n->next = head;                     // Point new node to the old first node
            head = n;                           // Make new node the new head
            return;                             // Done inserting, exit function
        }                                       // End of if
        Node* t = head;                         // Start a temp pointer 't' at the beginning
        while (t->next && t->next->data.score >= item.score) // Walk until we find a smaller score
            t = t->next;                        // Move 't' to the next node
        n->next = t->next;                      // Link new node to the rest of the list
        t->next = n;                            // Link previous node 't' to the new node
    }                                           // End of insert function

    std::vector<RankedDocument> toVector(std::size_t limit) const { // Copies top 'limit' nodes to vector
        std::vector<RankedDocument> res;        // Create an empty result vector
        for (Node* t = head; t && res.size() < limit; t = t->next) // Walk list up to 'limit' items
            res.push_back(t->data);             // Add each node's document to the vector
        return res;                             // Return the vector to the caller
    }                                           // End of toVector function
};                                              // End of RankedList class

// =======================================================
// 2. OOP BASE CLASS (Abstraction)
// =======================================================
class BaseRanker {                              // Parent (abstract) class
public:                                         // Public interface
    virtual ~BaseRanker() {}                    // Virtual destructor (required for safe inheritance)
    virtual double score(const Document& doc, const std::vector<std::string>& q, bool phrase = false) const = 0; // Pure virtual function (= 0)
};                                              // End of BaseRanker class

// =======================================================
// 3. OOP CHILD CLASS (Inheritance & Encapsulation)
// =======================================================
class Ranker : public BaseRanker {              // Child class inheriting from BaseRanker
private:                                        // Encapsulation: hides internal data from outside
    const Tokenizer& tokenizer_;                // Reference to the project's Tokenizer
    const SearchIndex& index_;                  // Reference to the project's SearchIndex
    std::unordered_map<std::string, double> termWeights_;   // Stores custom weight for each word
    std::unordered_map<int, double> documentBoosts_;        // Stores extra boost for each doc ID

public:                                         // Public functions used by other project files
    Ranker(const Tokenizer& t, const SearchIndex& i) : tokenizer_(t), index_(i) {} // Constructor: saves references

    // --- Word Weight Getters & Setters ---
    void setTermWeight(const std::string& t, double w) {    // Sets a word's weight
        termWeights_[t] = std::max(0.20, std::min(w, 3.00)); // Clamps weight between 0.20 and 3.00
    }                                                       // End of setTermWeight
    double termWeight(const std::string& t) const {         // Gets a word's weight
        return termWeights_.count(t) ? termWeights_.at(t) : 1.0; // Returns saved weight, or 1.0 if missing
    }                                                       // End of termWeight
    const std::unordered_map<std::string, double>& termWeights() const { // Returns the whole weights map
        return termWeights_;                                // Needed by other files
    }                                                       // End of termWeights

    // --- Document Boost Getters & Setters ---
    void setDocumentBoost(int id, double b) {               // Sets extra boost for a document ID
        documentBoosts_[id] = b;                            // Saves boost in the map
    }                                                       // End of setDocumentBoost
    double documentBoost(int id) const {                    // Gets boost for a document ID
        return documentBoosts_.count(id) ? documentBoosts_.at(id) : 0.0; // Returns boost, or 0.0 if missing
    }                                                       // End of documentBoost
    const std::unordered_map<int, double>& documentBoosts() const { // Returns the whole boosts map
        return documentBoosts_;                             // Needed by other files
    }                                                       // End of documentBoosts

    // --- Score ONE Document (OOP: Polymorphism using 'override') ---
    double score(const Document& doc, const std::vector<std::string>& q, bool phrase = false) const override {
        double total = documentBoost(doc.id());             // Start total score with document's boost
        auto title = tokenizer_.tokenize(doc.title(), true);// Split document title into words
        auto body = tokenizer_.tokenize(doc.content(), true);// Split document content into words

        // Put query words in unordered_set to remove duplicates, then loop through each unique word
        for (const auto& w : std::unordered_set<std::string>(q.begin(), q.end())) {
            double tf = 3.0 * std::count(title.begin(), title.end(), w) // Count word in title (worth 3x)
                      + std::count(body.begin(), body.end(), w);        // Plus count word in body (worth 1x)
            if (tf > 0)                                     // If word appeared at least once
                total += tf * index_.inverseDocumentFrequency(w) * termWeight(w); // Add TF * IDF * Weight
        }                                                   // End of word loop
        return (phrase && !q.empty()) ? total + 2.0 : total;// Add +2.0 bonus if exact phrase matched
    }                                                       // End of score function

    // --- Rank ALL Matching Documents using our Linked List ---
    std::vector<RankedDocument> rank(const std::vector<Document>& docs, const std::vector<std::string>& q,
                                     bool phrase = false, std::size_t limit = 20) const {
        RankedList list;                                    // Create our empty sorted linked list
        auto candidates = index_.candidates(q, false);      // Ask SearchIndex for matching doc IDs

        for (const auto& doc : docs) {                      // Loop through every document
            if (!candidates.count(doc.id())) continue;      // Skip if this doc is not a candidate

            RankedDocument item{doc.id(), score(doc, q, phrase), {}}; // Create result with ID & score
            for (const auto& w : q)                         // Check each query word
                if (doc.title().find(w) != std::string::npos || doc.content().find(w) != std::string::npos)
                    item.matchedTerms.push_back(w);         // Save word if found in title or content

            if (item.score > 0) list.insert(item);          // Insert into linked list if score > 0
        }                                                   // End of document loop
        return list.toVector(limit);                        // Return top 'limit' results as a vector
    }                                                       // End of rank function
};                                                          // End of Ranker class

} // namespace rar                                          // End of 'rar' namespace
