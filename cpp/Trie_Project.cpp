// cpp/Trie_Project.cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <pqxx/pqxx>
#include <cctype>

using namespace std;

// ============================================================================
// Trie Data Structure
// ============================================================================
struct TrieNode {
    TrieNode* children[26];
    bool isEndOfWord;
    int frequency;

    TrieNode() : isEndOfWord(false), frequency(0) {
        for (auto &c : children) c = nullptr;
    }

    ~TrieNode() {
        for (auto c : children) delete c;
    }
};

class Trie {
private:
    TrieNode* root;

public:
    Trie() { root = new TrieNode(); }
    ~Trie() { delete root; }

    void insert(const string& word, int freq = 1) {
        TrieNode* node = root;
        for (char ch : word) {
            if (!isalpha(static_cast<unsigned char>(ch))) continue;
            char c = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
            int idx = c - 'a';
            if (idx < 0 || idx >= 26) continue;
            if (!node->children[idx])
                node->children[idx] = new TrieNode();
            node = node->children[idx];
        }
        node->isEndOfWord = true;
        node->frequency += freq;
    }

    bool search(const string& word) const {
        TrieNode* node = root;
        for (char ch : word) {
            if (!isalpha(static_cast<unsigned char>(ch))) continue;
            char c = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
            int idx = c - 'a';
            if (idx < 0 || idx >= 26) return false;
            if (!node->children[idx]) return false;
            node = node->children[idx];
        }
        return node->isEndOfWord;
    }

    TrieNode* reachPrefixNode(const string& prefix) const {
        TrieNode* node = root;
        for (char ch : prefix) {
            if (!isalpha(static_cast<unsigned char>(ch))) continue;
            char c = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
            int idx = c - 'a';
            if (idx < 0 || idx >= 26) return nullptr;
            if (!node->children[idx]) return nullptr;
            node = node->children[idx];
        }
        return node;
    }

    void collect(TrieNode* node, string current, vector<pair<string,int>>& out) const {
        if (!node) return;
        if (node->isEndOfWord)
            out.push_back({current, node->frequency});
        for (int i = 0; i < 26; ++i) {
            if (node->children[i]) {
                collect(node->children[i], current + static_cast<char>('a' + i), out);
            }
        }
    }

    // Return (word, frequency) sorted by frequency desc
    vector<pair<string,int>> suggest(const string& prefix) const {
        vector<pair<string,int>> result;
        TrieNode* node = reachPrefixNode(prefix);
        if (!node) return result;
        collect(node, prefix, result);
        sort(result.begin(), result.end(),
             [](const pair<string,int>& a, const pair<string,int>& b){
                 if (a.second != b.second) return a.second > b.second;
                 return a.first < b.first;
             });
        return result;
    }
};

// ============================================================================
// Database Wrapper (clean interface)
// ============================================================================

class Database {
private:
    string connStr;

public:
    explicit Database(const string& connectionString) : connStr(connectionString) {}

    // Load all words and their frequencies; callback(word, frequency)
    void loadAll(function<void(const string&, int)> callback) {
        try {
            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            pqxx::result r = txn.exec("SELECT word, frequency FROM trie_words");
            for (auto row : r) {
                string w = row["word"].as<string>();
                int f = row["frequency"].as<int>();
                callback(w, f);
            }
            // no txn.commit() needed for read-only in this style; destructor will clean up
        } catch (const exception& e) {
            cerr << "DB load error: " << e.what() << '\n';
        }
    }

    // Save a single word: insert or increment frequency and update updated_at
    void saveWord(const string& word) {
        try {
            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            txn.exec_params(
                "INSERT INTO trie_words (word, frequency) VALUES ($1, 1) "
                "ON CONFLICT (word) DO UPDATE "
                "SET frequency = trie_words.frequency + 1, updated_at = NOW()",
                word
            );
            txn.commit();
        } catch (const exception& e) {
            cerr << "DB save error: " << e.what() << '\n';
        }
    }

    // Return all words from DB as (word, frequency)
    vector<pair<string,int>> fetchAllFromDb() {
        vector<pair<string,int>> out;
        try {
            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            pqxx::result r = txn.exec("SELECT word, frequency FROM trie_words ORDER BY word");
            for (auto row : r) {
                out.emplace_back(row["word"].as<string>(), row["frequency"].as<int>());
            }
        } catch (const exception& e) {
            cerr << "DB fetchAll error: " << e.what() << '\n';
        }
        return out;
    }
};

// ============================================================================
// Program helpers (IO + UI)
// ============================================================================

void loadWordsIntoTrie(Trie& trie, Database& db) {
    db.loadAll([&](const string& w, int f) {
        trie.insert(w, f);
    });
}

int showMenu() {
    cout << "\nTrie Autocomplete System\n";
    cout << "------------------------\n";
    cout << "1. Insert word\n";
    cout << "2. Search word\n";
    cout << "3. Autocomplete\n";
    cout << "4. View all words (from database)\n";
    cout << "0. Exit\n";
    cout << "Choice: ";

    int choice = -1;
    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }
    return choice;
}

void handleInsert(Trie& trie, Database& db) {
    string w;
    cout << "Enter word: ";
    cin >> w;
    if (w.empty()) {
        cout << "Empty word. Cancelled.\n";
        return;
    }
    trie.insert(w);
    db.saveWord(w);
    cout << "Word saved.\n";
}

void handleSearch(const Trie& trie) {
    string w;
    cout << "Word to search: ";
    cin >> w;
    cout << (trie.search(w) ? "Found\n" : "Not found\n");
}

void handleAutocomplete(const Trie& trie) {
    string p;
    cout << "Prefix: ";
    cin >> p;
    auto results = trie.suggest(p);
    if (results.empty()) {
        cout << "No suggestions.\n";
        return;
    }
    cout << "Suggestions:\n";
    for (auto &pr : results) {
        cout << " - " << pr.first << " (" << pr.second << " uses)\n";
    }
}

void handleViewAll(Database& db) {
    auto rows = db.fetchAllFromDb();
    if (rows.empty()) {
        cout << "No words in database.\n";
        return;
    }
    cout << "\nAll words (from DB):\n";
    cout << "---------------------\n";
    for (auto &r : rows) {
        cout << r.first << " (" << r.second << " uses)\n";
    }
}

// ============================================================================
// Clean main
// ============================================================================

int main() {
    // Connection string: update dbname/user/password/host as needed
    const string conn = "dbname=trie_db user=trie_user password=2024161143 host=localhost";
    Trie trie;
    Database db(conn);

    // Load existing DB entries into the in-memory trie
    loadWordsIntoTrie(trie, db);

    while (true) {
        int c = showMenu();
        if (c == 1) handleInsert(trie, db);
        else if (c == 2) handleSearch(trie);
        else if (c == 3) handleAutocomplete(trie);
        else if (c == 4) handleViewAll(db);
        else if (c == 0) break;
        else cout << "Invalid choice. Try again.\n";
    }

    cout << "Exiting. Goodbye.\n";
    return 0;
}