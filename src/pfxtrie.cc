#include "pfxtrie.h"
#include "logging.h"
#include "util.h"

#include <queue>

TrieNode::TrieNode(char val, TrieNode* parent) : val(val), parent(parent) {
  for (size_t i = 0; i < UCHAR_MAX; i++) {
    children[i] = nullptr;
  }
  if (parent) {
    level = parent->level + 1;
  } else {
    level = 0;
  }
}

TrieNode::~TrieNode() {
  for (size_t i = 1; i < UCHAR_MAX; i++) {
    delete children[i];
  }
}

std::vector<std::string> TrieNode::getSuffixes() {
  std::vector<std::string> results;
  if (children[0]) {
    results.emplace_back(1, val);
  }
  for (size_t i = 1; i < UCHAR_MAX; i++) {
    if (children[i]) {
      size_t old = results.size();
      std::vector<std::string> childSuffixes = children[i]->getSuffixes();
      results.insert(results.end(), childSuffixes.begin(), childSuffixes.end());
      for (size_t j = old; j < results.size(); j++) {
        results[j].insert(results[j].begin(), val);
      }
    }
  }
  return results;
}

PrefixTrie::PrefixTrie() : root(new TrieNode('*', nullptr)) {}

PrefixTrie::~PrefixTrie() { delete root; }

void PrefixTrie::insert(const std::string& word) { insertWordsFromLine(word); }

void PrefixTrie::removeNode(TrieNode* node) {
  if (node->parent) {
    node->parent->children[(size_t) node->val] = nullptr;
    delete node;
  }
}

void PrefixTrie::removeWordsFromLine(const std::string& line) {
  if (!enabled) return;
  TrieNode* curr = root;
  for (size_t i = 0; i <= line.size(); i++) {
    if (!isIdentifierChar(line[i]) || i == line.size()) {
      // End of word
      if (curr != root) {
        // Logging::breadcrumb("PFX Remove: " + line);
        curr->children[0] = (TrieNode*)((size_t)curr->children[0] - 1);
        if (curr->children[0] == 0) {
          removeNode(curr);
        }
      }
      curr = root;
      continue;
    }
    if (!curr->children[(size_t)line[i]]) {
      // Invalid State
      // curr->children[(size_t)line[i]] = new TrieNode(line[i], curr);
      return;
    }
    curr = curr->children[(size_t)line[i]];
  }
}

void PrefixTrie::insertWordsFromLine(const std::string& line) {
  if (!enabled) return;
  TrieNode* curr = root;
  for (size_t i = 0; i <= line.size(); i++) {
    if (!isIdentifierChar(line[i]) || i == line.size()) {
      // End of word
      // Use pointer storage to store the count
      if (curr != root) {
        // Don't increment for now since we never delete
        //   anything from the prefix trie
        // Logging::breadcrumb("PFX Insert: " + line);
        curr->children[0] = (TrieNode*)((size_t)curr->children[0] + 1);
      }
      curr = root;
      continue;
    }
    if (!curr->children[(size_t)line[i]]) {
      curr->children[(size_t)line[i]] = new TrieNode(line[i], curr);
    }
    curr = curr->children[(size_t)line[i]];
  }
}

void PrefixTrie::remove(const std::string& word) {
  removeWordsFromLine(word);
}

void PrefixTrie::reset() {
  delete root;
  root = new TrieNode('*', nullptr);
}

size_t PrefixTrie::count(const std::string& word) {
  TrieNode* node = getNode(word);
  if (node) return (size_t) node->children[0];
  return 0;
}

TrieNode* PrefixTrie::getNode(const std::string& word) {
  TrieNode* curr = root;
  size_t i = 0;
  while (i < word.size()) {
    if (!curr->children[(size_t)word[i]]) {
      return nullptr;
    }
    curr = curr->children[(size_t)word[i]];
    i++;
  }
  return curr == root ? nullptr : curr;
}

std::vector<std::string> PrefixTrie::getAllEntries() {
  if (root) {
    return root->getSuffixes();
  }
  return std::vector<std::string>();
}

std::vector<std::string> PrefixTrie::getMatchingPrefixes(
    const std::string& word) {
  TrieNode* node = getNode(word);
  if (node) {
    return node->getSuffixes();
  }
  return std::vector<std::string>();
}
