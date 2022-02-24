// Created by Zhong on 2/18/22.
//

#include "Cache.h"

Response * Cache::get(const std::string & key) {
  std::unordered_map<std::string, Node *>::iterator it = hash_table.find(key);
  if (it == hash_table.end())
    return nullptr;
  Node * node = it->second;
  putBack(node, false);

  return node->val;
}

void Cache::put(const std::string & key, Response * value) {
  std::unordered_map<std::string, Node *>::iterator it = hash_table.find(key);
  if (it == hash_table.end()) {
    Node * new_node = new Node(key, value);
    hash_table[key] = new_node;
    putBack(new_node, true);
  }
  else {
    delete it->second->val;
    it->second->val = value;
    putBack(it->second, false);
  }
}

void Cache::putBack(Node * node, bool is_new) {
  if (is_new) {
    size++;
    if (size > cache_cap) {
      Node * to_remove = head->next;
      head->next = to_remove->next;
      to_remove->next->pre = head;
      hash_table.erase(to_remove->key);
      delete to_remove;
      size--;
    }
  }
  else {
    if (node == tail->pre)
      return;
    node->pre->next = node->next;
    node->next->pre = node->pre;
  }
  node->pre = tail->pre;
  node->next = tail;
  tail->pre = node;
  node->pre->next = node;
}
