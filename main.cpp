#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <cctype>
using namespace std;

class Document {
public:
    int id;
    string title, text;

    Document(int i, string t, string x) : id(i), title(t), text(x) {}

    vector<string> words() const {
        vector<string> result;
        string w;

        for (char c : text) {
            if (isalnum((unsigned char)c))
                w += (char)tolower((unsigned char)c);
            else if (!w.empty()) {
                result.push_back(w);
                w.clear();
            }
        }

        if (!w.empty())
            result.push_back(w);

        return result;
    }
};

class SearchEngine {
private:
    vector<Document> docs;
    unordered_map<string, unordered_set<int>> index;
    unordered_map<int, double> learning;

    vector<string> tokenize(string q) {
        Document temp(0, "", q);
        return temp.words();
    }

    void buildIndex() {
        for (const auto& d : docs) {
            for (const auto& w : d.words()) {
                index[w].insert(d.id);
            }
        }
    }

public:
    SearchEngine() {
        docs.emplace_back(1, "Artificial Intelligence",
            "Artificial intelligence AI agents search information and make decisions.");

        docs.emplace_back(2, "Machine Learning",
            "Machine learning learns patterns from data using algorithms classification regression and training.");

        docs.emplace_back(3, "Data Structures",
            "Data structures include arrays linked lists stacks queues trees graphs hash tables and priority queues.");

        docs.emplace_back(4, "C++ OOP",
            "C++ object oriented programming uses classes objects encapsulation inheritance polymorphism abstraction constructors.");

        buildIndex();
    }

    void showDocuments() {
        cout << "\n--- PRIVATE DOCUMENTS ---\n";

        for (const auto& d : docs)
            cout << d.id << ". " << d.title << "\n";
    }

    void search(string query) {
        vector<string> q = tokenize(query);

        if (q.empty()) {
            cout << "Enter a valid search.\n";
            return;
        }

        unordered_set<int> candidates;

        // Find documents containing at least one query word
        for (const string& word : q) {
            if (index.count(word)) {
                for (int id : index[word])
                    candidates.insert(id);
            }
        }

        if (candidates.empty()) {
            cout << "\nNo results found.\n";
            return;
        }

        // Rank documents
        priority_queue<pair<double, int>> ranking;

        for (int id : candidates) {
            double score = learning[id];

            for (const string& word : q) {
                for (const string& docWord : docs[id - 1].words()) {
                    if (word == docWord)
                        score += 1;
                }
            }

            ranking.push({score, id});
        }

        cout << "\n--- SEARCH RESULTS ---\n";

        vector<pair<double, int>> results;

        while (!ranking.empty()) {
            results.push_back(ranking.top());
            ranking.pop();
        }

        for (int i = 0; i < (int)results.size(); i++) {
            int id = results[i].second;

            cout << i + 1 << ". "
                 << docs[id - 1].title
                 << " | Score: "
                 << results[i].first << "\n";
        }

        // Feedback
        cout << "\nSelect result for feedback (0 = skip): ";
        int choice;
        cin >> choice;

        if (choice >= 1 && choice <= (int)results.size()) {
            int id = results[choice - 1].second;

            cout << "Relevant? (1 = Yes, 0 = No): ";
            int feedback;
            cin >> feedback;

            if (feedback == 1)
                learning[id] += 1;
            else
                learning[id] -= 0.5;

            cout << "Learning updated.\n";
        }
    }

    void showLearning() {
        cout << "\n--- LEARNING WEIGHTS ---\n";

        if (learning.empty()) {
            cout << "No feedback yet.\n";
            return;
        }

        for (const auto& x : learning)
            cout << "Document " << x.first
                 << " = " << x.second << "\n";
    }
};

int main() {
    SearchEngine ai;
    int choice;

    while (true) {
        cout << "\n====================================\n";
        cout << " SELF-LEARNING PRIVATE AI SEARCH\n";
        cout << "====================================\n";
        cout << "1. View Documents\n";
        cout << "2. Search\n";
        cout << "3. View Learning\n";
        cout << "4. Exit\n";
        cout << "Choose: ";

        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            ai.showDocuments();
        }
        else if (choice == 2) {
            string query;
            cout << "Search: ";
            getline(cin, query);
            ai.search(query);
        }
        else if (choice == 3) {
            ai.showLearning();
        }
        else if (choice == 4) {
            cout << "Program closed.\n";
            break;
        }
        else {
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}
