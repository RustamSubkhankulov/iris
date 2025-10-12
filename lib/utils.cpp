#include <utils.hpp>

namespace iris {
namespace detail {

void ListNode::insertAfter(ListNode* node) noexcept {
  assert(node != nullptr);

  auto next = m_next;
  m_next = node;

  node->m_prev = this;
  node->m_next = next;

  if (next != nullptr) {
    next->m_prev = node;
  }
}

void ListNode::insertBefore(ListNode* node) noexcept {
  assert(node != nullptr);

  auto prev = m_prev;
  m_prev = node;

  node->m_next = this;
  node->m_prev = prev;

  if (prev != nullptr) {
    prev->m_next = node;
  }
}

void ListNode::unlink() noexcept {
  if (m_prev != nullptr) {
    m_prev->m_next = m_next;
    m_prev = nullptr;
  }
  if (m_next != nullptr) {
    m_next->m_prev = m_prev;
    m_next = nullptr;
  }
}

void List::insertFront(std::unique_ptr<ListNode> node) noexcept {
  auto nodePtr = node.release();

  if (m_size == 0LLU) {
    m_head = m_tail = nodePtr;
  } else {
    m_head->insertBefore(nodePtr);
    m_head = nodePtr;
  }
  m_size += 1LLU;
}

void List::insertBack(std::unique_ptr<ListNode> node) noexcept {
  auto nodePtr = node.release();

  if (m_size == 0LLU) {
    m_head = m_tail = nodePtr;
  } else {
    m_tail->insertAfter(nodePtr);
    m_tail = nodePtr;
  }
  m_size += 1LLU;
}

void List::doInsertBefore(ListNode* posNodePtr, ListNode* nodePtr) {
  posNodePtr->insertBefore(nodePtr);
  if (posNodePtr == m_head) {
    m_head = nodePtr;
  }
  m_size += 1LLU;
}

void List::insertBefore(iterator pos, std::unique_ptr<ListNode> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = pos.get();
  auto nodePtr = node.release();
  doInsertBefore(posNodePtr, nodePtr);
}

void List::insertBefore(const_iterator pos, std::unique_ptr<ListNode> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = const_cast<ListNode*>(pos.get());
  auto nodePtr = node.release();
  doInsertBefore(posNodePtr, nodePtr);
}

void List::doInsertAfter(ListNode* posNodePtr, ListNode* nodePtr) {
  posNodePtr->insertAfter(nodePtr);
  if (posNodePtr == m_tail) {
    m_tail = nodePtr;
  }
  m_size += 1LLU;
}

void List::insertAfter(iterator pos, std::unique_ptr<ListNode> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = pos.get();
  auto nodePtr = node.release();
  doInsertAfter(posNodePtr, nodePtr);
}

void List::insertAfter(const_iterator pos, std::unique_ptr<ListNode> node) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto posNodePtr = const_cast<ListNode*>(pos.get());
  auto nodePtr = node.release();
  doInsertAfter(posNodePtr, nodePtr);
}

void List::doErase(ListNode* nodePtr) {
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

void List::erase(iterator pos) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto nodePtr = pos.get();
  doErase(nodePtr);
}

void List::erase(const_iterator pos) {
  if (m_size == 0LLU) {
    throw IrisException("List is empty, 'pos' cannot be valid!");
  }

  auto nodePtr = const_cast<ListNode*>(pos.get());
  doErase(nodePtr);
}

void List::clear() noexcept {
  if (m_size == 0LLU) {
    return;
  }

  freeNodes();
  m_head = m_tail = nullptr;
  m_size = 0LLU;
}

void List::freeNodes() noexcept {
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

} // namespace detail
} // namespace iris