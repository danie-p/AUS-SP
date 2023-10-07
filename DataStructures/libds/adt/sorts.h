#pragma once

#include <libds/amt/implicit_sequence.h>
#include <libds/adt/queue.h>
#include <libds/adt/array.h>
#include <libds/adt/list.h>
#include <functional>
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <string_view>

namespace ds::adt
{
    template <typename T>
    struct Sort
    {
    public:
        virtual void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) = 0;
        void sort(amt::ImplicitSequence<T>& is) { sort(is, [](const T& a, const T& b)->bool {return a < b; }); }
    };

    template <typename T>
    class SelectSort :
        public Sort<T>
    {
    public:
        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;
    };

    template <typename T>
    class InsertSort :
        public Sort<T>
    {
    public:
        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;
    };

    template <typename T>
    class BubbleSort :
        public Sort<T>
    {
    public:
        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;
    };

    template <typename T>
    class QuickSort :
        public Sort<T>
    {
    public:
        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;

    private:
        void quick(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare, size_t min, size_t max);
    };

    template <typename T>
    class HeapSort :
        public Sort<T>
    {
    public:
        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;
    };

    template <typename T>
    class ShellSort :
        public Sort<T>
    {
    public:
        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;

    private:
        void shell(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare, size_t k);
    };

    template <typename Key, typename T>
    class RadixSort :
        public Sort<T>
    {
        static_assert(std::is_integral_v<Key>, "Radix sort supports only integral types.");

    public:
        RadixSort();
        RadixSort(std::function<Key(const T&)> getKey);

        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;

    private:
        std::function<Key(const T&)> getKey_;
    };

    template <typename T>
    class MergeSort :
        public Sort<T>
    {
    public:
        void sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare) override;
        // void sort(adt::ImplicitList<T>& il, std::function<bool(const T&, const T&)> compare);

    private:
        void split(size_t n);
        void merge(std::function<bool(const T&, const T&)> compare, size_t n);

    private:
        ImplicitQueue<T>* queue1_ {nullptr};
        ImplicitQueue<T>* queue2_ {nullptr};
        ImplicitQueue<T>* mergeQueue_ {nullptr};
    };

    //----------

    template<typename T>
    void SelectSort<T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        // TODO 12
        // po implementacii vymazte vyhodenie vynimky!
        throw std::runtime_error("Not implemented yet");
    }

    template<typename T>
    void InsertSort<T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        // TODO 12
        // po implementacii vymazte vyhodenie vynimky!
        throw std::runtime_error("Not implemented yet");
    }

    template<typename T>
    void BubbleSort<T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        bool swapped;
        do
        {
            swapped = false;
            for (size_t i = 0; i < is.size() - 1; ++i)
            {
                if (compare(is.access(i + 1)->data_, is.access(i)->data_))
                {
                    using std::swap;
                    swap(is.access(i + 1)->data_, is.access(i)->data_);
                    swapped = true;
                }
            }
        }
        while (swapped);
    }

    template<typename T>
    void QuickSort<T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        if (!is.isEmpty())
        {
            quick(is, compare, 0, is.size() - 1);
        }
    }

    template<typename T>
    void QuickSort<T>::quick(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare, size_t min, size_t max)
    {
        // TODO 12
        // po implementacii vymazte vyhodenie vynimky!
        throw std::runtime_error("Not implemented yet");
    }

    template<typename T>
    void HeapSort<T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        // TODO 12
        // po implementacii vymazte vyhodenie vynimky!
        throw std::runtime_error("Not implemented yet");
    }

    template<typename T>
    void ShellSort<T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        shell(is, compare, std::log10(is.size()));
    }

    template<typename T>
    void ShellSort<T>::shell(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare, size_t k)
    {
        // TODO 12
        // po implementacii vymazte vyhodenie vynimky!
        throw std::runtime_error("Not implemented yet");
    }

    template<typename Key, typename T>
    RadixSort<Key, T>::RadixSort() :
        getKey_([](auto const& x) { return x; })
    {
    }

    template<typename Key, typename T>
    RadixSort<Key, T>::RadixSort(std::function<Key(const T&)> getKey) :
        getKey_(getKey)
    {
    }

    // impl
    template<typename Key, typename T>
    void RadixSort<Key, T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        Array<ExplicitQueue<T>*> buckets = 10;

        for (size_t i = 0; i < 10; i++)
        {
            buckets.set(new ExplicitQueue<T>(), i);
        }

        size_t component = 1;
        bool existsOtherComponent = true;

        while (existsOtherComponent)
        {
            existsOtherComponent = false;
            
            for (T element : is)
            {
                Key key = getKey_(element);
                buckets.access((key / component) % 10)->push(element);
                if (element / (component * 10) > 0)
                {
                    existsOtherComponent = true;
                }
            }

            component = component * 10;
            size_t index = 0;

            for (size_t i = 0; i < 10; i++)
            {
                ExplicitQueue<T>* bucket = buckets.access(i);
                while (!bucket->isEmpty())
                {
                    is.access(index)->data_ = bucket->pop();
                    ++index;
                }
            }
        }

        for (size_t i = 0; i < 10; i++)
        {
            delete buckets.access(i);
        }
    }

    // impl
    // napr obce a pocet samohlasok
    template<typename T>
    void MergeSort<T>::sort(amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
    {
        this->queue1_ = new ImplicitQueue<T>(is.size());
        this->queue2_ = new ImplicitQueue<T>(is.size());
        this->mergeQueue_ = new ImplicitQueue<T>(is.size());

        for (T element : is)
        {
            this->mergeQueue_->push(element);
        }

        size_t i = 1;
        while (i < is.size())
        {
            this->split(i);
            this->merge(compare, i);
            i = i * 2;
        }

        this->split(i);
        this->merge(compare, i);

        for (size_t i = 0; i < is.size(); i++)
        {
            // prvok T popnuty z mergeQueue treba nastavit do !!datovej casti!! bloku pamate, ktory vrati access nad IS
            is.access(i)->data_ = this->mergeQueue_->pop();
        }

        delete this->queue1_;
        delete this->queue2_;
        delete this->mergeQueue_;
        this->queue1_ = nullptr;
        this->queue2_ = nullptr;
        this->mergeQueue_ = nullptr;
    }

    // impl
    template<typename T>
    void MergeSort<T>::split(size_t n)
    {
        size_t count = 0;
        bool isFirst = true;

        while (!this->mergeQueue_->isEmpty())
        {
            if (count % n == 0)
            {
                count = 0;
                isFirst = !isFirst;
            }

            isFirst ? this->queue1_->push(this->mergeQueue_->pop()) : this->queue2_->push(this->mergeQueue_->pop());
            ++count;
        }
    }

    // impl
    template<typename T>
    void MergeSort<T>::merge(std::function<bool(const T&, const T&)> compare, size_t n)
    {
        size_t firstCount = 0;
        size_t secondCount = 0;
        
        do
        {
            if (firstCount == 0 && secondCount == 0)
            {
                firstCount = std::min<int>(n, this->queue1_->size());
                secondCount = std::min<int>(n, this->queue2_->size());
            }

            T* key1 = firstCount > 0 ? &(this->queue1_->peek()) : nullptr;
            T* key2 = secondCount > 0 ? &(this->queue2_->peek()) : nullptr;

            if (key1 != nullptr && key2 != nullptr)
            {
                if (compare(*key1, *key2))
                {
                    --firstCount;
                    this->mergeQueue_->push(this->queue1_->pop());
                }
                else
                {
                    --secondCount;
                    this->mergeQueue_->push(this->queue2_->pop());
                }
            }
            else
            {
                if (key1 != nullptr)
                {
                    --firstCount;
                    this->mergeQueue_->push(this->queue1_->pop());
                }
                else if (key2 != nullptr)
                {
                    --secondCount;
                    this->mergeQueue_->push(this->queue2_->pop());
                }
            }
        } while (!this->queue1_->isEmpty() || !this->queue2_->isEmpty());
    }

    //// pretazeny sort na implicit list
    //template<typename T>
    //void MergeSort<T>::sort(adt::ImplicitList<T>& il, std::function<bool(const T&, const T&)> compare)
    //{
    //    this->queue1_ = new ImplicitQueue<T>(il.size());
    //    this->queue2_ = new ImplicitQueue<T>(il.size());
    //    this->mergeQueue_ = new ImplicitQueue<T>(il.size());

    //    for (T element : il)
    //    {
    //        this->mergeQueue_->push(element);
    //    }

    //    size_t i = 1;
    //    while (i < il.size())
    //    {
    //        this->split(i);
    //        this->merge(compare, i);
    //        i = i * 2;
    //    }

    //    this->split(i);
    //    this->merge(compare, i);

    //    for (size_t i = 0; i < il.size(); i++)
    //    {
    //        *il.access(i) = *this->mergeQueue_->pop();
    //    }

    //    delete this->queue1_;
    //    delete this->queue2_;
    //    delete this->mergeQueue_;
    //    this->queue1_ = nullptr;
    //    this->queue2_ = nullptr;
    //    this->mergeQueue_ = nullptr;
    //}
}