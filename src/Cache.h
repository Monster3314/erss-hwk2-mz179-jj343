//
// Created by Zhong on 2/18/22.
//

#ifndef ERSS_HWK2_MZ179_JJ343_CACHE_H
#define ERSS_HWK2_MZ179_JJ343_CACHE_H

#include <unordered_map>

#include "response.hpp"

struct Node {
  std::string key;
  Response * val;
  Node * next;
  Node * pre;
  Node(std::string x, Response * y) : key(x), val(y), next(nullptr), pre(nullptr) {}
  ~Node() { delete val; }
};
class Cache {
 public:
  int cache_cap;
  int size;
  std::unordered_map<std::string, Node *> hash_table;
  Node * head;
  Node * tail;

 public:
  Cache(int capacity) : cache_cap(capacity), size(0) {
    head = new Node("", nullptr);
    tail = new Node("", nullptr);
    head->next = tail;
    tail->pre = head;
  }

  ~Cache() {
    for (std::unordered_map<std::string, Node *>::iterator it = hash_table.begin();
         it != hash_table.end();
         it++) {
      delete it->second;
    }
    delete head;
    delete tail;
  }
  Response * get(const std::string & key);
  void put(const std::string & key, Response * value);
  void putBack(Node * node, bool is_new);
};
#endif  //ERSS_HWK2_MZ179_JJ343_CACHE_H
