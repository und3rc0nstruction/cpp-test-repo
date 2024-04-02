#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {

public:

    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
    
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double TF = 1.0/words.size();

        for (const string& s : words){
            documents[s][document_id] += TF;
        }
        ++document_count;
    }
    vector<Document> FindTopDocuments(const string& raw_query) const {

        const TQuery query_words = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    // --- значения
    map<string, map<int, double>> documents;
        int document_count = 0;

    struct TQuery
    {
        set<string> plus_words;
        set<string> minus_words;
    };

    set<string> stop_words;

    // --- функции

    bool IsStopWord(const string& word) const {
        return stop_words.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {

        vector<string> words;

        for (const string& s : SplitIntoWords(text)) {
            if (!IsStopWord(s)) {
                words.push_back(s);
            }
        }
        return words;
    }

    TQuery ParseQuery(const string& text) const{
        
        TQuery query;

        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-'){
                query.minus_words.insert(word.substr(1));
            }
            else{
                query.plus_words.insert(word);
            }
        }
        return query;
    }

    double IDFCalc(const string& word) const {
        return log(document_count * 1.0 / documents.at(word).size());
    }

       vector<Document> FindAllDocuments(const TQuery& query_words) const{

        map<int, double> doc_rev; // ключ - ID, значение - частота
        vector<Document> result;

        for (const string& s : query_words.plus_words) {
            if (documents.count(s) == 0){
                continue;
            }
            const double IDF = IDFCalc(s);
            for (const auto& [document_id, freq] : documents.at(s) ) {
                doc_rev[document_id] += freq * IDF;
            }
        }  // обходим плюс слова

           for (const string& s : query_words.minus_words) {
               if (documents.count(s) == 0) {
                   continue;
               }
               for (const auto& data : documents.at(s)) {
                   doc_rev.erase(data.first);
               }
           } // исключаем при наличии минус слов

           for (const auto& x : doc_rev) {
               result.push_back({x.first, x.second});
           }
           return result;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}


int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}