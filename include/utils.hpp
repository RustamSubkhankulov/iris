#ifndef INCLUDE_UTILS_HPP
#define INCLUDE_UTILS_HPP

#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include <exception.hpp>

namespace iris {
namespace detail {

class ListNode {
public:
  ListNode() = default;

  ListNode(const ListNode&) = delete;
  ListNode& operator=(const ListNode&) = delete;

  ListNode(ListNode&& other) noexcept
    : m_next(std::exchange(other.m_next, nullptr))
    , m_prev(std::exchange(other.m_prev, nullptr)) {}

  virtual ~ListNode() = default;

  ListNode(ListNode* next, ListNode* prev) noexcept
    : m_next(next)
    , m_prev(prev) {}

  void insertAfter(ListNode* node) noexcept;
  void insertBefore(ListNode* node) noexcept;

  void unlink() noexcept;

  void replaceWith(ListNode& that) noexcept;

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

  List& operator=(List&& other) noexcept {
    freeNodes();
    m_head = std::exchange(other.m_head, nullptr);
    m_tail = std::exchange(other.m_tail, nullptr);
    m_size = std::exchange(other.m_size, 0LLU);
    return *this;
  }

  void swap(List&& other) noexcept {
    std::swap(m_head, other.m_head);
    std::swap(m_tail, other.m_tail);
    std::swap(m_size, other.m_size);
  }

  ~List() noexcept {
    freeNodes();
  }

private:
  template <typename ListNodeT>
  class IteratorImpl final {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = ListNodeT;
    using difference_type = std::ptrdiff_t;

    using pointer = ListNodeT*;
    using reference = ListNodeT&;

    IteratorImpl(pointer ptr) noexcept
      : m_ptr(ptr) {}

    IteratorImpl(const IteratorImpl&) = default;
    IteratorImpl& operator=(const IteratorImpl&) = default;

    ~IteratorImpl() = default;

    reference operator*() const {
      return *m_ptr;
    }

    pointer operator->() const {
      return m_ptr;
    }

    pointer get() const {
      return m_ptr;
    }

    IteratorImpl& operator++() {
      if (m_ptr != nullptr) {
        m_ptr = m_ptr->m_next;
      }
      return *this;
    }

    IteratorImpl& operator--() {
      if (m_ptr != nullptr) {
        m_ptr = m_ptr->m_prev;
      }
      return *this;
    }

    IteratorImpl& operator++(int) {
      auto tmp = *this;
      if (m_ptr != nullptr) {
        m_ptr = m_ptr->m_next;
      }
      return tmp;
    }

    IteratorImpl& operator--(int) {
      auto tmp = *this;
      if (m_ptr != nullptr) {
        m_ptr = m_ptr->m_prev;
      }
      return tmp;
    }

    bool operator==(const IteratorImpl& other) const {
      return m_ptr == other.m_ptr;
    }

    bool operator!=(const IteratorImpl& other) const {
      return m_ptr != other.m_ptr;
    }

  private:
    ListNodeT* m_ptr;
  };

public:
  using iterator = IteratorImpl<ListNode>;
  using const_iterator = IteratorImpl<const ListNode>;

  using reverse_iterator = std::reverse_iterator<IteratorImpl<ListNode>>;
  using const_reverse_iterator =
    std::reverse_iterator<IteratorImpl<const ListNode>>;

  ListNode& front() noexcept {
    return *m_head;
  }

  ListNode& try_front() {
    if (m_head == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_head;
  }

  const ListNode& front() const noexcept {
    return *m_head;
  }

  const ListNode& try_front() const {
    if (m_head == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_head;
  }

  const ListNode& cfront() const noexcept {
    return *m_head;
  }

  const ListNode& try_cfront() const {
    if (m_head == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_head;
  }

  ListNode& back() noexcept {
    return *m_tail;
  }

  ListNode& try_back() {
    if (m_tail == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_tail;
  }

  const ListNode& back() const noexcept {
    return *m_tail;
  }

  const ListNode& try_back() const {
    if (m_tail == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_tail;
  }

  const ListNode& cback() const noexcept {
    return *m_tail;
  }

  const ListNode& try_cback() const {
    if (m_tail == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_tail;
  }

  iterator begin() noexcept {
    return iterator{m_head};
  }
  iterator end() noexcept {
    return iterator{nullptr};
  }

  const_iterator begin() const noexcept {
    return const_iterator{m_head};
  }
  const_iterator end() const noexcept {
    return const_iterator{nullptr};
  }

  const_iterator cbegin() const noexcept {
    return const_iterator{m_head};
  }
  const_iterator cend() const noexcept {
    return const_iterator{nullptr};
  }

  reverse_iterator rbegin() noexcept {
    return std::reverse_iterator(end());
  }
  const_reverse_iterator rbegin() const noexcept {
    return std::reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const noexcept {
    return std::reverse_iterator(cend());
  }

  reverse_iterator rend() noexcept {
    return std::reverse_iterator(begin());
  }
  const_reverse_iterator rend() const noexcept {
    return std::reverse_iterator(begin());
  }
  const_reverse_iterator crend() const noexcept {
    return std::reverse_iterator(cbegin());
  }

  std::size_t size() const noexcept {
    return m_size;
  }

  void insertFront(std::unique_ptr<ListNode> node) noexcept;
  void insertBack(std::unique_ptr<ListNode> node) noexcept;

  void insertBefore(iterator pos, std::unique_ptr<ListNode> node);
  void insertBefore(const_iterator pos, std::unique_ptr<ListNode> node);

  void insertAfter(iterator pos, std::unique_ptr<ListNode> node);
  void insertAfter(const_iterator pos, std::unique_ptr<ListNode> node);

  void erase(iterator pos);
  void erase(const_iterator pos);

  void clear() noexcept;

private:
  ListNode* m_head = nullptr;
  ListNode* m_tail = nullptr;
  std::size_t m_size = 0LLU;

  void freeNodes() noexcept;

  void doInsertBefore(ListNode* posNodePtr, ListNode* nodePtr);
  void doInsertAfter(ListNode* posNodePtr, ListNode* nodePtr);
  void doErase(ListNode* nodePtr);
};

template <std::unsigned_integral IdType>
class IDProvider final {
public:
  IdType obtainID() {
    if (m_curID == std::numeric_limits<IdType>::max()) {
      throw std::overflow_error("ID Provider has overflown!");
    }
    return m_curID++;
  }

  IdType getLastID() const noexcept {
    return m_curID;
  }

  void reset() noexcept {
    m_curID = 0;
  }

private:
  IdType m_curID = 0;
};

} // namespace detail
} // namespace iris

#endif // INCLUDE_UTILS_HPP
