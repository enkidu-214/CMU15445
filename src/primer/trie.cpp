#include "primer/trie.h"

#include <stack>
#include <string_view>

#include "common/exception.h"

namespace bustub {

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  auto node = root_;
  for (const char &c : key) {
    if (node->children_.find(c) == node->children_.end()) {
      return nullptr;
    }
    node = node->children_.at(c);
  }
  auto value_node = dynamic_cast<const TrieNodeWithValue<T> *>(node.get());
  if (value_node == nullptr) {
    return nullptr;  // Type mismatch
  }
  return value_node->value_.get();
  // You should walk through the trie to find the node corresponding to the key.
  // If the node doesn't exist, return nullptr. After you find the node, you
  // should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is
  // mismatched, and you should return nullptr. Otherwise, return the value.
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when
  // creating `shared_ptr` on that value. 特殊处理“”情况
  std::stack<std::pair<char, std::unique_ptr<TrieNode>>> st;
  if (key.empty()) {
    auto value_node = std::make_shared<TrieNodeWithValue<T>>(
        root_->children_, std::make_shared<T>(std::move(value)));
    return Trie(std::move(value_node));
  }
  auto parent = root_->Clone();
  // 先开通一条路让child可以走
  std::unique_ptr<TrieNode> node;
  if (parent->children_.find(key[0]) == parent->children_.end()) {
    node = std::make_unique<TrieNode>();
  } else {
    node = parent->children_.at(key[0])->Clone();
  }
  for (uint32_t i = 1; i < key.size(); ++i) {
    char c = key[i];
    std::unique_ptr<TrieNode> child;
    // 在node的下一层，假如没有这个字符，则创建一个新的TrieNode，假如有这个字符，则直接克隆，实现激活
    if (node->children_.find(c) == node->children_.end()) {
      child = std::make_unique<TrieNode>();
    } else {
      child = node->children_.at(c)->Clone();
    }
    st.emplace(key[i - 1], std::move(parent));
    parent = std::move(node);
    node = std::move(child);
  }
  st.emplace(key.back(), std::move(parent));
  auto value_node = std::make_unique<TrieNodeWithValue<T>>(
      node->children_, std::make_shared<T>(std::move(value)));
  node = std::move(value_node);
  // 就是有问题，查不出
  //  if(node->is_value_node_){
  //    //如果node是值节点，之前克隆了，直接转型，node生命周期结束
  //    auto value_node = dynamic_cast<TrieNodeWithValue<T> *>(node.get());
  //    value_node->value_ = std::make_shared<T>(std::move(value));
  //    node = std::unique_ptr<TrieNodeWithValue<T>>(std::move(value_node));
  //  } else{
  //    auto value_node =
  //    std::make_unique<TrieNodeWithValue<T>>(node->children_,std::make_shared<T>(std::move(value)));
  //    node = std::move(value_node);
  //  }
  // 父节点回收固化子节点
  while (!st.empty()) {
    auto [ch, par] = std::move(st.top());
    st.pop();
    par->children_[ch] = std::shared_ptr<TrieNode>(std::move(node));
    node = std::move(par);
  }
  return Trie(std::shared_ptr<TrieNode>(std::move(node)));
  // You should walk through the trie and create new nodes if necessary. If the
  // node corresponding to the key already exists, you should create a new
  // `TrieNodeWithValue`.
}

auto Trie::Remove(std::string_view key) const -> Trie {
  auto node = root_->Clone();
  std::stack<std::pair<char, std::unique_ptr<TrieNode>>> st;
  // 如果不满足条件则直接返回，如果满足条件则接着向前走
  for (const char &c : key) {
    if (node->children_.find(c) == node->children_.end()) {
      return Trie(root_);
    }
    // 字符，字符对应的父节点
    auto temp = node->children_.at(c)->Clone();
    st.emplace(c, std::move(node));
    node = std::move(temp);
  }
  if (!node->is_value_node_) {
    return Trie(root_);
  }
  node = std::make_unique<TrieNode>(node->children_);
  bool delete_node = true;
  if (!node->children_.empty()) {
    // 如果node没有子节点，则可以删除
    delete_node = false;
  }
  while (!st.empty()) {
    auto [ch, parent] = std::move(st.top());
    st.pop();
    if (delete_node) {
      parent->children_.erase(ch);
    } else {
      parent->children_[ch] = std::shared_ptr<TrieNode>(std::move(node));
    }
    if (!parent->children_.empty() || parent->is_value_node_) {
      delete_node = false;
    }
    node = std::move(parent);
  }
  return Trie(std::move(node));
  // You should walk through the trie and remove nodes if necessary. If the node
  // doesn't contain a value any more, you should convert it to `TrieNode`. If a
  // node doesn't have children any more, you should remove it.
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and
// functions in the header file. However, we separate the implementation into a
// .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate
// them here, so that they can be picked up by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below
// lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
