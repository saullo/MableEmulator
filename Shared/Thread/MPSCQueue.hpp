#pragma once

#include <array>
#include <atomic>
#include <new>

namespace Thread
{
    namespace Details
    {
        template <typename T> class MPSCQueueNonIntrusive
        {
        public:
            MPSCQueueNonIntrusive(MPSCQueueNonIntrusive const &) = delete;
            MPSCQueueNonIntrusive &operator=(MPSCQueueNonIntrusive const &) = delete;
            MPSCQueueNonIntrusive() : m_head(new Node()), m_tail(m_head.load(std::memory_order_relaxed))
            {
                Node *front = m_head.load(std::memory_order_relaxed);
                front->Next.store(nullptr, std::memory_order_relaxed);
            }

            ~MPSCQueueNonIntrusive()
            {
                T *output;
                while (dequeue(output))
                    delete output;

                Node *front = m_head.load(std::memory_order_relaxed);
                delete front;
            }

            void enqueue(T *input)
            {
                Node *node = new Node(input);
                Node *previous_head = m_head.exchange(node, std::memory_order_acq_rel);
                previous_head->Next.store(node, std::memory_order_release);
            }

            bool dequeue(T *&result)
            {
                Node *tail = m_tail.load(std::memory_order_relaxed);
                Node *next = tail->Next.load(std::memory_order_acquire);
                if (!next)
                    return false;

                result = next->Data;
                m_tail.store(next, std::memory_order_release);
                delete tail;
                return true;
            }

        private:
            struct Node
            {
                Node() = default;
                explicit Node(T *data) : Data(data) { Next.store(nullptr, std::memory_order_relaxed); }

                T *Data;
                std::atomic<Node *> Next;
            };

            std::atomic<Node *> m_head;
            std::atomic<Node *> m_tail;
        };

        template <typename T, std::atomic<T *> T::*IntrusiveLink> class MPSCQueueIntrusive
        {
            using Atomic = std::atomic<T *>;

        public:
            MPSCQueueIntrusive(MPSCQueueIntrusive const &) = delete;
            MPSCQueueIntrusive &operator=(MPSCQueueIntrusive const &) = delete;
            MPSCQueueIntrusive()
                : m_dummy_ptr(reinterpret_cast<T *>(m_dummy.data())), m_head(m_dummy_ptr), m_tail(m_dummy_ptr)
            {
                Atomic *dummyNext = new (&(m_dummy_ptr->*IntrusiveLink)) Atomic();
                dummyNext->store(nullptr, std::memory_order_relaxed);
            }

            ~MPSCQueueIntrusive()
            {
                T *output;
                while (dequeue(output))
                    delete output;

                (m_dummy_ptr->*IntrusiveLink).~Atomic();
            }

            void enqueue(T *input)
            {
                (input->*IntrusiveLink).store(nullptr, std::memory_order_release);
                T *previous_head = m_head.exchange(input, std::memory_order_acq_rel);
                (previous_head->*IntrusiveLink).store(input, std::memory_order_release);
            }

            bool dequeue(T *&result)
            {
                T *tail = m_tail.load(std::memory_order_relaxed);
                T *next = (tail->*IntrusiveLink).load(std::memory_order_acquire);
                if (tail == m_dummy_ptr)
                {
                    if (!next)
                        return false;

                    m_tail.store(next, std::memory_order_release);
                    tail = next;
                    next = (next->*IntrusiveLink).load(std::memory_order_acquire);
                }

                if (next)
                {
                    m_tail.store(next, std::memory_order_release);
                    result = tail;
                    return true;
                }

                T *head = m_head.load(std::memory_order_acquire);
                if (tail != head)
                    return false;

                enqueue(m_dummy_ptr);
                next = (tail->*IntrusiveLink).load(std::memory_order_acquire);
                if (next)
                {
                    m_tail.store(next, std::memory_order_release);
                    result = tail;
                    return true;
                }
                return false;
            }

        private:
            alignas(T) std::array<std::byte, sizeof(T)> m_dummy;
            T *m_dummy_ptr;
            Atomic m_head;
            Atomic m_tail;
        };
    } // namespace Details

    template <typename T, std::atomic<T *> T::*IntrusiveLink = nullptr>
    using MPSCQueue = std::conditional_t<IntrusiveLink != nullptr, Details::MPSCQueueIntrusive<T, IntrusiveLink>,
                                         Details::MPSCQueueNonIntrusive<T>>;
} // namespace Thread
