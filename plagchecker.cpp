/*
 * PlagCheck - Local Plagiarism Detection Engine (GCC 6 Compatible)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cmath>

// ─────────────────────────────────────────
// UTILITY
// ─────────────────────────────────────────

std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string normalize(const std::string& text) {
    std::string result;
    bool prevSpace = false;
    for (char c : text) {
        if (isalnum(c)) {
            result += tolower(c);
            prevSpace = false;
        } else {
            if (!prevSpace && !result.empty()) {
                result += ' ';
                prevSpace = true;
            }
        }
    }
    if (!result.empty() && result.back() == ' ')
        result.pop_back();
    return result;
}

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) tokens.push_back(word);
    return tokens;
}

// ─────────────────────────────────────────
// DSA 1: RABIN-KARP
// ─────────────────────────────────────────

const long long MOD1 = 1e9 + 7;
const long long BASE1 = 31;

std::unordered_set<long long> computeShingles(const std::vector<std::string>& tokens, int k = 5) {
    std::unordered_set<long long> shingles;
    if (tokens.size() < k) return shingles;

    long long hash = 0, power = 1;

    for (int i = 0; i < k; i++) {
        long long wordHash = 0;
        for (char c : tokens[i])
            wordHash = (wordHash * BASE1 + (c - 'a' + 1)) % MOD1;
        hash = (hash + wordHash * power) % MOD1;
        if (i < k - 1) power = power * BASE1 % MOD1;
    }

    shingles.insert(hash);

    for (int i = k; i < tokens.size(); i++) {
        long long oldWordHash = 0;
        for (char c : tokens[i - k])
            oldWordHash = (oldWordHash * BASE1 + (c - 'a' + 1)) % MOD1;

        hash = (hash - oldWordHash + MOD1) % MOD1;
        hash = hash * BASE1 % MOD1;

        long long newWordHash = 0;
        for (char c : tokens[i])
            newWordHash = (newWordHash * BASE1 + (c - 'a' + 1)) % MOD1;

        hash = (hash + newWordHash * power) % MOD1;
        shingles.insert(hash);
    }

    return shingles;
}

// ─────────────────────────────────────────
// DSA 2: JACCARD
// ─────────────────────────────────────────

double jaccardSimilarity(const std::unordered_set<long long>& a,
                          const std::unordered_set<long long>& b) {
    if (a.empty() && b.empty()) return 1.0;
    if (a.empty() || b.empty()) return 0.0;

    int intersection = 0;
    for (auto& h : a)
        if (b.count(h)) intersection++;

    int unionSize = a.size() + b.size() - intersection;
    return (double)intersection / unionSize;
}

// ─────────────────────────────────────────
// DSA 3: LCS
// ─────────────────────────────────────────

struct LCSResult {
    int length;
    std::string snippet;
};

LCSResult longestCommonSubstring(const std::vector<std::string>& a,
                                 const std::vector<std::string>& b) {

    int m = std::min((int)a.size(), 200);
    int n = std::min((int)b.size(), 200);

    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    int maxLen = 0, endIdx = 0;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
                if (dp[i][j] > maxLen) {
                    maxLen = dp[i][j];
                    endIdx = i;
                }
            }
        }
    }

    std::string snippet;
    int start = std::max(0, endIdx - maxLen);
    for (int i = start; i < endIdx; i++) {
        if (i > start) snippet += " ";
        snippet += a[i];
    }

    return {maxLen, snippet};
}

// ─────────────────────────────────────────
// DSA 4: COSINE
// ─────────────────────────────────────────

std::unordered_map<std::string, double> computeTF(const std::vector<std::string>& tokens) {
    std::unordered_map<std::string, int> freq;
    for (auto& t : tokens) freq[t]++;

    std::unordered_map<std::string, double> tf;
    for (auto& p : freq)
        tf[p.first] = (double)p.second / tokens.size();

    return tf;
}

double cosineSimilarity(const std::unordered_map<std::string, double>& a,
                        const std::unordered_map<std::string, double>& b) {

    double dot = 0, normA = 0, normB = 0;

    for (auto& p : a) {
        normA += p.second * p.second;
        if (b.count(p.first))
            dot += p.second * b.at(p.first);
    }

    for (auto& p : b)
        normB += p.second * p.second;

    if (normA == 0 || normB == 0) return 0.0;

    return dot / (sqrt(normA) * sqrt(normB));
}

// ─────────────────────────────────────────
// DSA 5: PHRASE MATCH
// ─────────────────────────────────────────

int countExactPhraseMatches(const std::string& t1, const std::string& t2, int k = 6) {
    std::unordered_set<std::string> phrases;

    std::istringstream iss(t1);
    std::vector<std::string> words;
    std::string w;

    while (iss >> w) words.push_back(w);

    for (int i = 0; i + k <= words.size(); i++) {
        std::string phrase;
        for (int j = i; j < i + k; j++) {
            if (j > i) phrase += " ";
            phrase += words[j];
        }
        phrases.insert(phrase);
    }

    int count = 0;
    for (auto& p : phrases)
        if (t2.find(p) != std::string::npos)
            count++;

    return count;
}

// ─────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────

int main(int argc, char* argv[]) {

    if (argc < 3) {
        std::cout << "Usage: plagcheck file1 file2\n";
        return 1;
    }

    std::string raw1 = readFile(argv[1]);
    std::string raw2 = readFile(argv[2]);

    std::string norm1 = normalize(raw1);
    std::string norm2 = normalize(raw2);

    auto t1 = tokenize(norm1);
    auto t2 = tokenize(norm2);

    auto s1 = computeShingles(t1);
    auto s2 = computeShingles(t2);

    double jaccard = jaccardSimilarity(s1, s2);
    double cosine = cosineSimilarity(computeTF(t1), computeTF(t2));
    int phrases = countExactPhraseMatches(norm1, norm2);

    auto lcs = longestCommonSubstring(t1, t2);

    double overall = (jaccard * 0.45) + (cosine * 0.35) + std::min(phrases * 0.02, 0.2);

    std::string verdict;
    if (overall >= 0.75) verdict = "HIGH PLAGIARISM";
    else if (overall >= 0.45) verdict = "MODERATE";
    else if (overall >= 0.2) verdict = "LOW";
    else verdict = "ORIGINAL";

    std::cout << "\n===== RESULT =====\n";
    std::cout << "Overall: " << overall * 100 << "%\n";
    std::cout << "Jaccard: " << jaccard * 100 << "%\n";
    std::cout << "Cosine : " << cosine * 100 << "%\n";
    std::cout << "LCS    : " << lcs.length << " words\n";
    std::cout << "Phrase : " << phrases << "\n";
    std::cout << "Verdict: " << verdict << "\n";

    if (!lcs.snippet.empty())
        std::cout << "Snippet: " << lcs.snippet << "\n";

    return 0;
}