#ifndef ATOMIC_FORWARD_LIST_H_
#define ATOMIC_FORWARD_LIST_H_

#include <assert.h>
#include <atomic>

#include <intr_shared_ptr.h>

template <typename T> class atomic_forward_list
{
    struct link;
    struct node;
    typedef intr_shared_ptr<node> link_iptr_t;
    typedef typename link_iptr_t::shared_ptr link_sptr_t;
    struct link {
        std::atomic<unsigned long> ref_cnt;
        link_iptr_t next;
        link() : ref_cnt(0), next(nullptr) {}
        explicit link(const link_sptr_t& p) : ref_cnt(0), next(p) {}
        ~link() {}
        link(const link& x) = delete;
        link& operator=(const link& x) = delete;
        void AddRef() { ref_cnt.fetch_add(1); }
        bool DelRef() { return ref_cnt.fetch_sub(1) == 1; }
    };
    struct node : public link {
        T data;
        explicit node(const T& x, const link_sptr_t& p) : link(p), data(x) {}
        ~node() {}
    };

    public:
    atomic_forward_list() : head_(), head_p_(static_cast<node*>(&head_)) {
        head_.AddRef(); // Make sure head is not deleted by any smart pointer
    }
    ~atomic_forward_list() {
        this->clear();
    }
    void clear() {
        while (bool(head_.next)) {
          head_.next = head_.next.get()->next;
        }
    }
    bool empty() const {
        return !bool(head_.next);
    }
    bool push_front(const T& x) {
        // Capture current head.
        link_sptr_t h(head_.next.get());
        // Create new node, link it after the current head.
        node* n = new node(x, h);
        // Change the head to point to the new node.
        // If the head changed by another thread, CAS sets h to the new head,
        // so replace "next" pointer in the new node to point to that.
        while (!head_.next.compare_exchange_strong(h, n)) {
            n->next.reset(h);
        }
        return true;
    }

    bool pop_front(T& x) {
        // Capture current head.
        link_sptr_t h(head_.next.get());
        if (!bool(h)) return false;
        // Change the head to point to the new head (next node).
        // If the head changed by another thread, CAS sets h to the new head,
        // so replace "next" pointer in the new node to point to that.
        while (!head_.next.compare_exchange_strong(h, h->next.get())) {
            if (!bool(h)) return false;
        }
        x = h->data;
        return true;
    }

    class iterator {
        public:
        ~iterator() {}
        explicit operator bool() const { return bool(p_); }
        T* operator->() const { return &(p_->data); }
        T& operator*() const { return p_->data; }
        iterator operator++() {
            p_ = p_->next.get();
            return *this;
        }
        iterator operator++(int) {
            iterator tmp(*this);
            p_ = p_->next.get();
            return tmp;
        }
        bool operator==(const iterator& rhs) const {
            return p_ == rhs.p_;
        }
        bool operator!=(const iterator& rhs) const {
            return p_ != rhs.p_;
        }
        private:
        link_sptr_t p_;
        friend class atomic_forward_list;
        explicit iterator(const link_sptr_t& p) : p_(p) {}
        iterator() : p_() {}
    };

    iterator before_begin() {
        return iterator(head_p_.get());
    }

    iterator begin() {
        return iterator(head_.next.get());
    }

    iterator end() {
        return iterator();
    }

    iterator insert_after(const iterator& pos, const T& x) {
        // Capture current position.
        link_sptr_t p(pos.p_->next.get());
        // Create new node, link it after the current position.
        node* n = new node(x, p);
        // Change the current node to point to the new node.
        // If the node changed by another thread, CAS sets p to the new node,
        // so replace "next" pointer in the new node to point to that.
        while (!pos.p_->next.compare_exchange_strong(p, n)) {
            n->next.reset(p);
        }
        return iterator(pos.p_->next.get());
    }

    iterator erase_after(const iterator& pos) {
        // Capture current position.
        link_sptr_t p(pos.p_->next.get());
        if (!bool(p)) return iterator();
        // Change the current node to point to the next node.
        // If current node changed by another thread, CAS sets p to the new head,
        // so replace "next" pointer in the new node to point to that.
        while (!pos.p_->next.compare_exchange_strong(p, p->next.get())) {
            if (!bool(p)) return iterator();
        }
        return iterator(pos.p_->next.get());
    }

    iterator find(const T& x) const {
        link_sptr_t p(head_.next.get());
        while (p && !(p->data == x)) {
            p = p->next.get();
        }
        return iterator(p);
    }

    private:
    link head_;
    link_iptr_t head_p_;
};

#endif // ATOMIC_FORWARD_LIST_H_
