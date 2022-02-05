#pragma once

#include <array>
#include <stdexcept>

template<class T>
class Stack
{
    struct Node
    {
        T     data;
        Node *last;
        Node *next;
    };

public:
    Stack() : head(nullptr), tail(nullptr), length(0) {}

    ~Stack()
    {
        if(!length)
            return;

        Node *node = head;

        while(node)
        {
            Node *next = node->next;
            delete node;
            node = next;
        }
    }

    void push(T item)
    {
        Node *node = new Node{item, tail, nullptr};

        if(!head)
            head = node;
        if(!tail)
            tail = node;

        tail->next = node;
        tail       = node;

        length++;
    }

    void pop()
    {
        Node *temp = tail;

        tail = tail->last;

        delete temp;

        if(tail)
            tail->next = nullptr;

        length--;
    }

    void pop_n(size_t amount)
    {
        if(amount > length)
            return;

        for(size_t i = 0; i < amount; i++)
            pop();
    }

    T& back()
    {
        if(tail == nullptr)
            throw std::out_of_range("back of stack is empty");
        return tail->data;
    }

    T& front()
    {
        if(head == nullptr)
            throw std::out_of_range("front of stack is empty");
        return head->data;
    }

    inline bool empty() const
    {
        return length == 0;
    }

    size_t len() const
    {
        return length;
    }

    template<size_t n>
    std::array<T, n> get_array_from_back(bool auto_pop = false)
    {
        if(!tail || n > length)
            return {};

        std::array<T, n> output{};

        Node *node = tail;

        for(size_t i = n; node && i != 0; i--)
        {
            output[i-1] = node->data;
            node = node->last;

            if(auto_pop)
                pop();
        }

        return output;
    }

    std::vector<T> get_vec_from_back(size_t n, bool auto_pop = false)
    {
        if(!tail || n > length)
            return {};

        std::vector<T> output;

        output.reserve(n);

        Node *node = tail;

        for(size_t i = n-1; node && i >= 0; i--)
        {
            output.push_back(node->data);
            node = node->last;

            if(auto_pop)
                pop();
        }

        return output;
    }

    std::pair<T&, T&> top_two()
    {
        return {tail->last->data, tail->data};
    }

private:
    Node *head;
    Node *tail;
    size_t length;
};