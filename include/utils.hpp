#ifndef INCLUDE_OPS_GENERIC_UTILS_HPP
#define INCLUDE_OPS_GENERIC_UTILS_HPP

#include <cassert>
#include <utility>

namespace iris {

class ListNode {
private:
  ListNode* m_next = nullptr;
  ListNode* m_prev = nullptr;

public:
  ListNode() = default;

  ListNode(const ListNode&) = delete;
  ListNode& operator=(const ListNode&) = delete;

  ListNode(ListNode&& other)
    : m_next(std::exchange(other.m_next, nullptr))
    , m_prev(std::exchange(other.m_prev, nullptr)) {}

  ListNode& operator=(ListNode&& other) {
    m_next = other.m_next;
    m_prev = other.m_prev;

    other.m_next = nullptr;
    other.m_prev = nullptr;

    return *this;
  }

  virtual ~ListNode() = default;

  ListNode(ListNode* next, ListNode* prev)
    : m_next(next)
    , m_prev(prev) {}

  void insert_after(ListNode* node) {
    assert(node != nullptr);

    auto next = m_next;
    m_next = node;

    node->m_prev = this;
    node->m_next = next;

    if (next != nullptr) {
      next->m_prev = node;
    }
  }

  void insert_before(ListNode* node) {
    assert(node != nullptr);

    auto prev = m_prev;
    m_prev = node;

    node->m_next = this;
    node->m_prev = prev;

    if (prev != nullptr) {
      prev->m_next = node;
    }
  }
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_UTILS_HPP
