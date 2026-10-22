#pragma once
#include "Config [Arshveer Singh].h"
#include "SearchEngine [Arshveer Singh].h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace rar {

// Console agent coordinates input, retrieval actions, and reflection/feedback.
class Agent {
private:
    SearchEngine engine_;
    std::string lastQuery_;
    std::vector<RankedDocument> lastResults_;

    static void clearInput() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    static int readInt(const std::string& prompt) {
        std::cout << prompt;
        int value;
        while (!(std::cin >> value)) {
            clearInput();
            std::cout << "Enter a valid number: ";
        }
        clearInput();
        return value;
    }

    static std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        return value;
    }

    void printResults() const {
        if (lastResults_.empty()) {
            std::cout << "No matching documents.\n";
            return;
        }
        std::cout << "\n--- Ranked results ---\n";
        for (std::size_t i = 0; i < lastResults_.size(); ++i) {
            const auto& result = lastResults_[i];
            const auto* doc = engine_.getDocument(result.documentId);
            if (!doc) continue;
            std::cout << i + 1 << ". [" << doc->id() << "] " << doc->title()
                      << " | score=" << result.score << "\n"
                      << "   Source: " << doc->source() << "\n"
                      << "   Date: " << doc->dateString() << "\n"
                      << "   " << doc->preview(PREVIEW_LENGTH) << "\n";
        }
    }

    void addDocument() {
        const auto title = readLine("Document title: ");
        const auto content = readLine("Document content (one line in this menu): ");
        const auto source = readLine("Source label (e.g. notes, assignment): ");
        const auto tagsText = readLine("Comma-separated tags (or blank): ");
        std::vector<std::string> tags;
        std::string current;
        for (char ch : tagsText) {
            if (ch == ',') {
                if (!current.empty()) tags.push_back(current);
                current.clear();
            } else if (ch != ' ') current.push_back(ch);
        }
        if (!current.empty()) tags.push_back(current);
        if (engine_.addDocument(title, content, source.empty() ? "manual" : source, tags))
            std::cout << "Saved locally.\n";
        else std::cout << "Could not save document. Check title/content and disk access.\n";
    }

    void importFile() {
        const auto path = readLine("Path to local plain-text file: ");
        if (engine_.importTextFile(path)) std::cout << "Imported and indexed locally.\n";
        else std::cout << "Import failed. Check the path and file permissions.\n";
    }

    void search() {
        lastQuery_ = readLine("Search query: ");
        const auto mode = readLine("Match mode: [O]R (any term) / [A]ND (all terms): ");
        const bool all = !mode.empty() && (mode[0] == 'a' || mode[0] == 'A');
        std::cout << "[Reason] Tokenizing query and identifying candidate documents...\n";
        lastResults_ = engine_.search(lastQuery_, all);
        std::cout << "[Act] Retrieval and ranking finished.\n";
        printResults();
        std::cout << "[Reflect] Rate a result using the feedback menu to influence future ranking.\n";
    }

    void feedback() {
        if (lastResults_.empty()) {
            std::cout << "Search first.\n";
            return;
        }
        printResults();
        const int resultNo = readInt("Result number to rate (0 cancels): ");
        if (resultNo == 0) return;
        if (resultNo < 1 || resultNo > static_cast<int>(lastResults_.size())) {
            std::cout << "Invalid result number.\n";
            return;
        }
        const auto answer = readLine("Was this useful? [y/n]: ");
        if (answer.empty() || (answer[0] != 'y' && answer[0] != 'Y' &&
                               answer[0] != 'n' && answer[0] != 'N')) {
            std::cout << "Feedback cancelled.\n";
            return;
        }
        const bool useful = answer[0] == 'y' || answer[0] == 'Y';
        if (engine_.feedback(lastResults_[resultNo - 1].documentId, lastQuery_, useful))
            std::cout << "[Reflect] Feedback saved; ranking weights updated locally.\n";
        else std::cout << "Feedback could not be saved.\n";
    }

    void listDocuments() const {
        if (engine_.documents().empty()) {
            std::cout << "No documents yet.\n";
            return;
        }
        for (const auto& doc : engine_.documents())
            std::cout << "[" << doc.id() << "] " << doc.title()
                      << " | " << doc.source() << " | " << doc.dateString() << '\n';
    }

    void deleteDocument() {
        const int id = readInt("Document ID to delete: ");
        if (engine_.deleteDocument(id)) std::cout << "Document deleted.\n";
        else std::cout << "No document with that ID, or storage failed.\n";
    }

    void updateDocument() {
        const int id = readInt("Document ID to update: ");
        const auto title = readLine("New title: ");
        const auto content = readLine("New content: ");
        if (engine_.updateDocument(id, title, content)) std::cout << "Document updated and re-indexed.\n";
        else std::cout << "Update failed.\n";
    }

    void searchByTag() const {
        const auto tag = readLine("Exact tag: ");
        const auto found = engine_.findByTag(tag);
        if (found.empty()) std::cout << "No documents with that tag.\n";
        for (const auto& doc : found)
            std::cout << "[" << doc.id() << "] " << doc.title() << '\n';
    }

public:
    void run() {
        while (true) {
            std::cout << "\n============================================\n"
                      << " " << APP_NAME << " v" << APP_VERSION << "\n"
                      << " Private Knowledge Retrieval | C++ Prototype\n"
                      << "============================================\n"
                      << "1. Add document\n"
                      << "2. Import local .txt file\n"
                      << "3. Search documents\n"
                      << "4. Give feedback / reflect\n"
                      << "5. List documents\n"
                      << "6. Delete document\n"
                      << "7. Update document\n"
                      << "8. Search by tag\n"
                      << "9. Statistics\n"
                      << "10. Save and exit\n";
            const int choice = readInt("Choose: ");
            switch (choice) {
                case 1: addDocument(); break;
                case 2: importFile(); break;
                case 3: search(); break;
                case 4: feedback(); break;
                case 5: listDocuments(); break;
                case 6: deleteDocument(); break;
                case 7: updateDocument(); break;
                case 8: searchByTag(); break;
                case 9: engine_.printStatistics(); break;
                case 10:
                    if (engine_.saveAll()) std::cout << "All local data saved. Goodbye.\n";
                    else std::cout << "Warning: one or more data files could not be saved.\n";
                    return;
                default: std::cout << "Choose a number from 1 to 10.\n";
            }
        }
    }
};

} // namespace rar
