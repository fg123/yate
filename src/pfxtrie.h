#ifndef PFXTRIE_H
#define PFXTRIE_H

#include <climits>
#include <string>
#include <vector>

/* Simple Prefix Trie data structure for autocomplete */

/* Since children[0] will never be reached (technically neither will
 *   the first 32 control characters, we can use children[0] to store
 *   the count of strings that end there. */
struct TrieNode {
  char val;
  int level;
  TrieNode* children[UCHAR_MAX];
  TrieNode(char val, TrieNode* parent);
  ~TrieNode();
  std::vector<std::string> getSuffixes();
};

class PrefixTrie {
  TrieNode* root;
public:
  PrefixTrie();
  ~PrefixTrie();

  TrieNode* getNode(const std::string& word);
  void insert(const std::string& word);
  void remove(const std::string& word);
  size_t count(const std::string& word);
  void reset();
  void insertWordsFromLine(const std::string& line);

  std::vector<std::string> getMatchingPrefixes(const std::string& word);
};


#endif
