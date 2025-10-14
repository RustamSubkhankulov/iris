#ifndef INCLUDE_UTILS_HPP
#define INCLUDE_UTILS_HPP

#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <exception.hpp>

namespace iris {
namespace detail {

template <typename Derived>
class ListNode;

template <typename T>
concept ListNodeBased = std::is_base_of_v<ListNode<T>, T>;

template <typename Derived>
class ListNode {
public:
  ListNode() = default;

  ListNode(const ListNode&) = delete;
  ListNode& operator=(const ListNode&) = delete;

  ListNode(ListNode&& other) noexcept
    : m_next(std::exchange(other.m_next, nullptr))
    , m_prev(std::exchange(other.m_prev, nullptr)) {}

  virtual ~ListNode() = default;

  ListNode(Derived* next, Derived* prev) noexcept
    : m_next(next)
    , m_prev(prev) {}

  void insertAfter(Derived* node) noexcept;
  void insertBefore(Derived* node) noexcept;

  void unlink() noexcept;

  void replaceWith(ListNode& that) noexcept;

private:
  Derived* m_next = nullptr;
  Derived* m_prev = nullptr;

  template <ListNodeBased T>
  friend class List;
};

template <typename Derived>
void ListNode<Derived>::insertAfter(Derived* node) noexcept {
  assert(node != nullptr);

  auto next = m_next;
  m_next = node;

  node->m_prev = static_cast<Derived*>(this);
  node->m_next = next;

  if (next != nullptr) {
    next->m_prev = node;
  }
}

template <typename Derived>
void ListNode<Derived>::insertBefore(Derived* node) noexcept {
  assert(node != nullptr);

  auto prev = m_prev;
  m_prev = node;

  node->m_next = static_cast<Derived*>(this);
  node->m_prev = prev;

  if (prev != nullptr) {
    prev->m_next = node;
  }
}

template <typename Derived>
void ListNode<Derived>::unlink() noexcept {
  if (m_prev != nullptr) {
    m_prev->m_next = m_next;
    m_prev = nullptr;
  }
  if (m_next != nullptr) {
    m_next->m_prev = m_prev;
    m_next = nullptr;
  }
}

template <typename Derived>
void ListNode<Derived>::replaceWith(ListNode& that) noexcept {
  that.unlink();
  if (m_prev != nullptr) {
    m_prev->m_next = static_cast<Derived*>(&that);
    m_prev = nullptr;
  }
  if (m_next != nullptr) {
    m_next->m_prev = static_cast<Derived*>(&that);
    m_next = nullptr;
  }
}

template <ListNodeBased T>
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
  template <typename ValueTy>
  class IteratorImpl final {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = ValueTy;
    using difference_type = std::ptrdiff_t;

    using pointer = ValueTy*;
    using reference = ValueTy&;

    IteratorImpl(ValueTy* ptr) noexcept
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
    ValueTy* m_ptr;
  };

public:
  using iterator = IteratorImpl<T>;
  using const_iterator = IteratorImpl<const T>;

  using reverse_iterator = std::reverse_iterator<IteratorImpl<T>>;
  using const_reverse_iterator = std::reverse_iterator<IteratorImpl<const T>>;

  T& front() noexcept {
    return *m_head;
  }

  T& try_front() {
    if (m_head == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_head;
  }

  const T& front() const noexcept {
    return *m_head;
  }

  const T& try_front() const {
    if (m_head == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_head;
  }

  const T& cfront() const noexcept {
    return *m_head;
  }

  const T& try_cfront() const {
    if (m_head == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_head;
  }

  T& back() noexcept {
    return *m_tail;
  }

  T& try_back() {
    if (m_tail == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_tail;
  }

  const T& back() const noexcept {
    return *m_tail;
  }

  const T& try_back() const {
    if (m_tail == nullptr) {
      throw IrisException("List is empty!");
    }
    return *m_tail;
  }

  const T& cback() const noexcept {
    return *m_tail;
  }

  const T& try_cback() const {
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

  void insertFront(std::unique_ptr<T> node) noexcept;
  void insertBack(std::unique_ptr<T> node) noexcept;

  void insertBefore(iterator pos, std::unique_ptr<T> node);
  void insertBefore(const_iterator pos, std::unique_ptr<T> node);

  void insertAfter(iterator pos, std::unique_ptr<T> node);
  void insertAfter(const_iterator pos, std::unique_ptr<T> node);

  void erase(iterator pos);
  void erase(const_iterator pos);

  void clear() noexcept;

private:
  T* m_head = nullptr;
  T* m_tail = nullptr;
  std::size_t m_size = 0LLU;

  void freeNodes() noexcept;

  void doInsertBefore(T* posNodePtr, T* nodePtr);
  void doInsertAfter(T* posNodePtr, T* nodePtr);
  void doErase(T* nodePtr);
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

template <ListNodeBased T>
void List<T>::insertFront(std::unique_ptr<T> node) noexcept {
  auto nodePtr = node.release();

  if (m_size == 0LLU) {
    m_head = m_tail = nodePtr;
  } else {
    m_head->insertBefore(nodePtr);
    m_head = nodePtr;
  }
  m_size += 1LLU;
}

template <ListNodeBased T>
void List<T>::insertBack(std::unique_ptr<T> node) noexcept {
  auto nodePtr = node.release();

  if (m_size == 0LLU) {
    m_head = m_tail = nodePtr;
  } else {
    m_tail->insertAfter(nodePtr);
    m_tail = nodePtr;
  }
  m_size += 1LLU;
}

template <ListNodeBased T>
void List<T>::doInsertBefore(T* posNodePtr, T* nodePtr) {
  posNodePtr->insertBefore(nodePtr);
  if (posNodePtr == m_head) {
    m_head = nodePtr;
  }
  m_size += 1LLU;
}

template <ListNodeBased T>
void List<T>::insertBefore(iterator pos, std::unique_ptr<T> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = pos.get();
  auto nodePtr = node.release();
  doInsertBefore(posNodePtr, nodePtr);
}

template <ListNodeBased T>
void List<T>::insertBefore(const_iterator pos, std::unique_ptr<T> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = const_cast<T*>(pos.get());
  auto nodePtr = node.release();
  doInsertBefore(posNodePtr, nodePtr);
}

template <ListNodeBased T>
void List<T>::doInsertAfter(T* posNodePtr, T* nodePtr) {
  posNodePtr->insertAfter(nodePtr);
  if (posNodePtr == m_tail) {
    m_tail = nodePtr;
  }
  m_size += 1LLU;
}

template <ListNodeBased T>
void List<T>::insertAfter(iterator pos, std::unique_ptr<T> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = pos.get();
  auto nodePtr = node.release();
  doInsertAfter(posNodePtr, nodePtr);
}

template <ListNodeBased T>
void List<T>::insertAfter(const_iterator pos, std::unique_ptr<T> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = const_cast<T*>(pos.get());
  auto nodePtr = node.release();
  doInsertAfter(posNodePtr, nodePtr);
}

template <ListNodeBased T>
void List<T>::doErase(T* nodePtr) {
  if (nodePtr == m_head) {
    m_head = m_head->m_next;
  }
  if (nodePtr == m_tail) {
    m_tail = m_tail->m_prev;
  }

  nodePtr->unlink();
  delete nodePtr;
  m_size -= 1LLU;
}

template <ListNodeBased T>
void List<T>::erase(iterator pos) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto nodePtr = pos.get();
  doErase(nodePtr);
}

template <ListNodeBased T>
void List<T>::erase(const_iterator pos) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto nodePtr = const_cast<T*>(pos.get());
  doErase(nodePtr);
}

template <ListNodeBased T>
void List<T>::clear() noexcept {
  if (m_size == 0LLU) {
    return;
  }

  freeNodes();
  m_head = m_tail = nullptr;
  m_size = 0LLU;
}

template <ListNodeBased T>
void List<T>::freeNodes() noexcept {
  if (m_size == 0) {
    return;
  }
  auto* node = m_head;
  while (node != nullptr) {
    auto* next = node->m_next;
    delete node;
    node = next;
  }
}

} // namespace detail
} // namespace iris

#endif // INCLUDE_UTILS_HPP
