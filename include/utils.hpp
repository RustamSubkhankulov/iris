#ifndef INCLUDE_UTILS_HPP
#define INCLUDE_UTILS_HPP

#include <cassert>
#include <concepts>
#include <utility>

namespace iris {
namespace detail {

class ListNode {
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

private:
  ListNode* m_next = nullptr;
  ListNode* m_prev = nullptr;

  friend class List;
};

class List final {
public:
  List() = default;

  List(const List&) = delete;
  List& operator=(const List&) = delete;

  List(List&& other) noexcept
    : m_head(std::exchange(other.m_head, nullptr))
    , m_tail(std::exchange(other.m_tail, nullptr))
    , m_size(std::exchange(other.m_size, 0LLU)) {}

  List& operator=(List&& other) {
    freeNodes();
    m_head = std::exchange(other.m_head, nullptr);
    m_tail = std::exchange(other.m_tail, nullptr);
    m_size = std::exchange(other.m_size, 0LLU);
    return *this;
  }

  ~List() {
    freeNodes();
  }

private:
  template <typename ListNodeT>
  class IteratorImpl final {
  public:
    IteratorImpl(ListNodeT* ptr)
      : m_ptr(ptr) {}

    IteratorImpl(const IteratorImpl&) = default;
    IteratorImpl& operator=(const IteratorImpl&) = default;

    ~IteratorImpl() = default;

    ListNodeT& operator*() const {
      return *m_ptr;
    }
    ListNodeT* operator->() const {
      return m_ptr;
    }

    ListNodeT* get() const {
      return m_ptr;
    }

    IteratorImpl& operator++() {
      if (m_ptr != nullptr) {
        m_ptr = m_ptr->m_next;
      }
      return *this;
    }

    IteratorImpl& operator++(int) {
      auto& tmp = *this;
      if (m_ptr != nullptr) {
        m_ptr = m_ptr->m_next;
      }
      return tmp;
    }

    bool operator==(const IteratorImpl& other) const {
      return m_ptr == other.m_ptr;
    }

  private:
    ListNodeT* m_ptr;
  };

public:
  using iterator = IteratorImpl<ListNode>;
  using const_iterator = IteratorImpl<const ListNode>;

  ListNode& front() {
    return *m_head;
  }

  const ListNode& front() const {
    return *m_head;
  }

  const ListNode& cfront() const {
    return *m_head;
  }

  ListNode& back() {
    return *m_tail;
  }

  const ListNode& back() const {
    return *m_tail;
  }

  const ListNode& cback() const {
    return *m_tail;
  }

  iterator begin() {
    return iterator{m_head};
  }
  iterator end() {
    return iterator{nullptr};
  }

  const_iterator begin() const {
    return const_iterator{m_head};
  }
  const_iterator end() const {
    return const_iterator{nullptr};
  }

  const_iterator cbegin() const {
    return const_iterator{m_head};
  }
  const_iterator cend() const {
    return const_iterator{nullptr};
  }

  std::size_t size() const {
    return m_size;
  }

  void append_front(ListNode* node) {
    if (m_size == 0LLU) {
      m_head = m_tail = node;
    } else {
      m_head->insert_before(node);
      m_head = node;
    }
    m_size += 1;
  }

  void append_back(ListNode* node) {
    if (m_size == 0LLU) {
      m_head = m_tail = node;
    } else {
      m_tail->insert_after(node);
      m_tail = node;
    }
    m_size += 1;
  }

  void clear() {
    freeNodes();
    m_head = m_tail = nullptr;
    m_size = 0LLU;
  }

private:
  ListNode* m_head = nullptr;
  ListNode* m_tail = nullptr;
  std::size_t m_size = 0LLU;

  void freeNodes() {
    if (m_size == 0) {
      return;
    }
    ListNode* node = m_head;
    while (node != nullptr) {
      ListNode* next = node->m_next;
      delete node;
      node = next;
    }
  }
};

template <typename Derived>
class Singleton {
protected:
  Singleton() = default;
  ~Singleton() = default;

public:
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;

  static Derived& get() {
    static Derived m_instance;
    return m_instance;
  }
};

template <std::unsigned_integral IdType>
class IDProvider final {
public:
  IdType obtainID() {
    return m_curID++;
  }

  IdType getLastID() const {
    return m_curID;
  }

  void reset() {
    m_curID = 0;
  }

private:
  IdType m_curID = 0;
};

} // namespace detail
} // namespace iris

#endif // INCLUDE_UTILS_HPP
