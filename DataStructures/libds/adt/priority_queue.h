#pragma once

#include <libds/adt/abstract_data_type.h>
#include <libds/amt/implicit_sequence.h>
#include <libds/amt/explicit_sequence.h>
#include <libds/amt/implicit_hierarchy.h>
#include <cmath>
#include <functional>

// T TwoLists<P, T>::pop()

namespace ds::adt {

    template <typename P, typename T>
    struct PriorityQueueItem
    {
        P priority_;
        T data_;

        bool operator==(const PriorityQueueItem<P, T>& other) { return priority_ == other.priority_ && data_ == other.data_; }
    };

    template <typename P, typename T>
    using PQItem = PriorityQueueItem<P, T>;

    //----------

    template <typename P, typename T>
    class PriorityQueue :
        virtual public ADT
    {
    public:
        virtual void push(P priority, T data) = 0;      // do pushu pride priorita => umelost priority
        virtual T& peek() = 0;
        virtual T pop() = 0;
    };

    template <typename P, typename T>
    using PQ = PriorityQueue<P, T>;

    //----------

    template <typename P, typename T, typename SequenceType>
    class SequencePriorityQueue :
        public PriorityQueue<P, T>,
        public ADS<PQItem<P, T>>                        // APS s datovou castou typu PQItem
    {
    public:
        SequencePriorityQueue();
        SequencePriorityQueue(const SequencePriorityQueue& other);

    protected:
        SequenceType* getSequence() const;
    };

    template <typename P, typename T, typename SequenceType>
    using SPQ = SequencePriorityQueue<P, T, SequenceType>;

    //----------

    template <typename P, typename T, typename SequenceType>
    class UnsortedSequencePriorityQueue :
        public SequencePriorityQueue<P, T, SequenceType>
    {
    public:
        T& peek() override;

    protected:
        typename SequenceType::BlockType* findHighestPriorityBlock();
    };

    template <typename P, typename T, typename SequenceType>
    using UnsortedSPQ = UnsortedSequencePriorityQueue<P, T, SequenceType>;

    //----------

    template <typename P, typename T, typename SequenceType>
    class SortedSequencePriorityQueue :
        public SequencePriorityQueue<P, T, SequenceType>
    {
    public:
        T& peek() override;
        T pop() override;

    protected:
        virtual size_t indexOfHighestPriorityBlock() const = 0; // abstraktna metoda, pomocou nej implementujeme peek a pop
    };

    template <typename P, typename T, typename SequenceType>
    using SortedSPQ = SortedSequencePriorityQueue<P, T, SequenceType>;

    //----------

    template <typename P, typename T>
    class UnsortedImplicitSequencePriorityQueue :       // neutriedeny PQ v IS
        public UnsortedSequencePriorityQueue<P, T, amt::IS<PQItem<P,T>>>
    {
    public:
        void push(P priority, T data) override;
        T pop() override;

    private:
        using SequenceBlockType = typename amt::IS<PQItem<P, T>>::BlockType;
    };

    template <typename P, typename T>
    using UnsortedISPQ = UnsortedImplicitSequencePriorityQueue<P, T>;

    //----------

    template <typename P, typename T>
    class UnsortedExplicitSequencePriorityQueue :       // neutriedeny PQ v ES
        public UnsortedSequencePriorityQueue<P, T, amt::SinglyLS<PQItem<P, T>>>
    {
    public:
        void push(P priority, T data) override;
        T pop() override;

    private:
        using SequenceBlockType = typename amt::SinglyLS<PQItem<P, T>>::BlockType;
    };

    template <typename P, typename T>
    using UnsortedESPQ = UnsortedExplicitSequencePriorityQueue<P, T>;

    //----------

    template <typename P, typename T>
    class SortedImplicitSequencePriorityQueue :     // utriedeny PQ v IS
        public SortedSequencePriorityQueue<P, T, amt::IS<PQItem<P, T>>>
    {
    public:
        void push(P priority, T data) override;

    protected:
        size_t indexOfHighestPriorityBlock() const override;

    private:
        using SequenceBlockType = typename amt::IS<PQItem<P, T>>::BlockType;
    };

    template <typename P, typename T>
    using SortedISPQ = SortedImplicitSequencePriorityQueue<P, T>;

    //----------

    template <typename P, typename T>
    class SortedExplicitSequencePriorityQueue :     // utriedeny PQ v ES
        public SortedSequencePriorityQueue<P, T, amt::SinglyLS<PQItem<P, T>>>
    {
    public:
        void push(P priority, T data) override;

    protected:
        size_t indexOfHighestPriorityBlock() const override;

    private:
        using SequenceBlockType = typename amt::SinglyLS<PQItem<P, T>>::BlockType;
    };

    template <typename P, typename T>
    using SortedESPQ = SortedExplicitSequencePriorityQueue<P, T>;

    //----------

    template <typename P, typename T>
    class TwoLists :                // dvojzoznam <3
        public AUMS<PQItem<P, T>>,
        public PriorityQueue<P, T>
    {
    public:
        TwoLists(size_t expectedSize);
        TwoLists(const TwoLists<P, T>& other);
        ~TwoLists();

        ADT& assign(const ADT& other) override;
        void clear() override;
        size_t size() const override;
        bool isEmpty() const override;
        bool equals(const ADT& other) override;

        void push(P priority, T data) override;
        T& peek() override;
        T pop() override;

    private:
        using ShortSequenceType = amt::IS<PQItem<P, T>>;
        using LongSequenceType = amt::SinglyLS<PQItem<P, T>>;
        using ShortSequenceBlockType = typename ShortSequenceType::BlockType;
        using LongSequenceBlockType = typename LongSequenceType::BlockType;

        ShortSequenceType* shortSequence_;
        LongSequenceType* longSequence_;
    };

    //----------

    template <typename P, typename T>
    class BinaryHeap :              // binarna halda <3
        public PriorityQueue<P, T>,
        public ADS<PQItem<P, T>>
    {
    public:
        BinaryHeap();
        BinaryHeap(const BinaryHeap& other);
        void push(P priority, T data) override;
        T& peek() override;
        T pop() override;

    private:
        using HierarchyBlockType = typename amt::BinaryIH<PQItem<P, T>>::BlockType;
        amt::BinaryIH<PQItem<P, T>>* getHierarchy();
    };

    //----------

    template<typename P, typename T, typename SequenceType>
    SequencePriorityQueue<P, T, SequenceType>::SequencePriorityQueue() :
        ADS<PQItem<P, T>>(new SequenceType())
    {
    }

    template<typename P, typename T, typename SequenceType>
    SequencePriorityQueue<P, T, SequenceType>::SequencePriorityQueue(const SequencePriorityQueue& other) :
        ADS<PQItem<P, T>>(new SequenceType(), other)
    {
    }

    template<typename P, typename T, typename SequenceType>
    SequenceType* SequencePriorityQueue<P, T, SequenceType>::getSequence() const
    {
        return dynamic_cast<SequenceType*>(this->memoryStructure_);
    }

    template<typename P, typename T, typename SequenceType>
    T& UnsortedSequencePriorityQueue<P, T, SequenceType>::peek()
    {
        if (this->isEmpty()) { this->error("Priority Queue is empty!"); }

        return this->findHighestPriorityBlock()->data_.data_;       // pqitem (data) -> data
    }

    template<typename P, typename T, typename SequenceType>
    typename SequenceType::BlockType* UnsortedSequencePriorityQueue<P, T, SequenceType>::findHighestPriorityBlock()
    {
        // process all blocks forward => flexibilne, vyhodne vo vsetkych pripadoch
        // foreach garantuje najefektivnejsiu prehliadku
        // operacia = spristupnim prioritu a porovnam

        SequenceType::BlockType* bestBlock = this->getSequence()->accessFirst();
        this->getSequence()->processAllBlocksForward([&] (SequenceType::BlockType* b)
            {
                if (bestBlock->data_.priority_ > b->data_.priority_)
                {
                    bestBlock = b;
                }
            });

        return bestBlock;
    }

    template<typename P, typename T, typename SequenceType>
    T& SortedSequencePriorityQueue<P, T, SequenceType>::peek()
    {
        if (this->isEmpty()) { this->error("Priority Queue is empty!"); }

        return this->getSequence()->access(this->indexOfHighestPriorityBlock())->data_.data_;
    }

    template<typename P, typename T, typename SequenceType>
    T SortedSequencePriorityQueue<P, T, SequenceType>::pop()
    {
        if (this->isEmpty())
        {
            throw std::exception("Priority Queue is empty!");
        }

        size_t index = this->indexOfHighestPriorityBlock();
        T popped = this->getSequence()->access(index)->data_.data_;
        this->getSequence()->remove(index);
        return popped;
    }

    template<typename P, typename T>
    void UnsortedImplicitSequencePriorityQueue<P, T>::push(P priority, T data)
    {
        // vytvorim si novy PQItem (az ten musim vlozit do PQ)
        PQItem<P, T>& newItem = getSequence()->insertLast().data_;  // v IS je efektivne vkladanie na KONIEC (last)
        newItem.data_ = data;
        newItem.priority_ = priority;
    }

    template<typename P, typename T>
    T UnsortedImplicitSequencePriorityQueue<P, T>::pop()
    {
        if (this->isEmpty())
        {
            this->error("Priority Queue is empty!");
        }

        SequenceBlockType* bestBlock = this->findHighestPriorityBlock();
        T popped = bestBlock->data_.data_;      // data z bloku s najvyssou prioritou si odlozime este PRED swapovanim!! su to data prvku, ktory ma byt popnuty
        SequenceBlockType* lastBlock = this->getSequence()->accessLast();

        // chceme odstranovat data z KONCA IS => ak je to potrebne, spravime swap dat bloku s najvyssou prioritou (ktory ma byt odstraneny)
            // a dat bloku na konci IS (efektive odstranovanie z lastu)
        if (bestBlock != lastBlock)
        {
            // v swape vymiename IBA DATA!! nie samotne bloky, lebo by sme narusili strukturu
            std::swap(bestBlock->data_, lastBlock->data_);
        }
        
        this->getSequence()->removeLast();    // v IS je efektivne vyberanie z KONCA (last)
        return popped;
    }

    template<typename P, typename T>
    void UnsortedExplicitSequencePriorityQueue<P, T>::push(P priority, T data)
    {
        PQItem<P, T>& newItem = getSequence()->insertFirst().data_; // v ES je efektivne vkladanie na ZACIATOK (fisrt)
        newItem.data_ = data;
        newItem.priority_ = priority;
    }

    template<typename P, typename T>
    T UnsortedExplicitSequencePriorityQueue<P, T>::pop()
    {
        if (this->isEmpty())
        {
            this->error("Priority Queue is empty!");
        }

        SequenceBlockType* bestBlock = this->findHighestPriorityBlock();
        SequenceBlockType* firstBlock = this->getSequence()->accessFirst();       // budeme odstranovat blok na zaciatku ES
        T popped = bestBlock->data_.data_;

        if (bestBlock != firstBlock)
        {
            std::swap(bestBlock->data_, firstBlock->data_);
        }
        
        this->getSequence()->removeFirst();    // v ES je efektivne vyberanie zo ZACIATKU (first)
        return popped;
    }

    template<typename P, typename T>
    void SortedImplicitSequencePriorityQueue<P, T>::push(P priority, T data)
    {
        PQItem<P, T>* newItem = nullptr;

        if (this->isEmpty() || priority <= this->getSequence()->accessLast()->data_.priority_)
        {
            newItem = &(this->getSequence()->insertLast().data_);
        }
        else if (priority >= this->getSequence()->accessFirst()->data_.priority_)
        {
            newItem = &(this->getSequence()->insertFirst().data_);
        }
        else
        {
            auto blockAfter = this->getSequence()->findBlockWithProperty([&](SequenceBlockType* block)
                {
                    return block->data_.priority_ <= priority;
                });
            newItem = &(this->getSequence()->insertBefore(*blockAfter).data_);
        }

        newItem->priority_ = priority;
        newItem->data_ = data;
    }

    template<typename P, typename T>
    size_t SortedImplicitSequencePriorityQueue<P, T>::indexOfHighestPriorityBlock() const
    {
        return this->size() - 1;
    }

    template<typename P, typename T>
    void SortedExplicitSequencePriorityQueue<P, T>::push(P priority, T data)
    {
        PQItem<P, T>* queueData = nullptr;

        if (this->isEmpty() || priority <= this->getSequence()->accessFirst()->data_.priority_)
        {
            queueData = &(this->getSequence()->insertFirst().data_);
        }
        else if (priority >= this->getSequence()->accessLast()->data_.priority_)
        {
            queueData = &(this->getSequence()->insertLast().data_);
        }
        else
        {
            auto blockBefore = this->getSequence()->findPreviousToBlockWithProperty([&](auto block)
                { return block->data_.priority_ >= priority; });
            queueData = &(this->getSequence()->insertAfter(*blockBefore).data_);
        }

        queueData->priority_ = priority;
        queueData->data_ = data;
    }

    template<typename P, typename T>
    size_t SortedExplicitSequencePriorityQueue<P, T>::indexOfHighestPriorityBlock() const
    {
        return 0;
    }

    template<typename P, typename T>
    TwoLists<P, T>::TwoLists(size_t expectedSize):
        shortSequence_(new ShortSequenceType(std::ceil(std::sqrt(expectedSize)), false)),
        longSequence_(new LongSequenceType())
    {
    }

    template <typename P, typename T>
    TwoLists<P, T>::TwoLists(const TwoLists<P, T>& other) :
        shortSequence_(new ShortSequenceType()),
        longSequence_(new LongSequenceType())
    {
        assign(other);
    }

    template<typename P, typename T>
    TwoLists<P, T>::~TwoLists()
    {
        delete shortSequence_;
        shortSequence_ = nullptr;
        delete longSequence_;
        longSequence_ = nullptr;
    }

    template<typename P, typename T>
    ADT& TwoLists<P, T>::assign(const ADT& other)
    {
        if (this != &other) {
            const TwoLists<P, T>& otherTwoLists = dynamic_cast<const TwoLists<P, T>&>(other);

            shortSequence_->assign(*otherTwoLists.shortSequence_);
            longSequence_->assign(*otherTwoLists.longSequence_);
        }

        return *this;
    }

    template<typename P, typename T>
    void TwoLists<P, T>::clear()
    {
        shortSequence_->clear();
        longSequence_->clear();
    }

    template<typename P, typename T>
    size_t TwoLists<P, T>::size() const
    {
        return shortSequence_->size() + longSequence_->size();
    }

    template<typename P, typename T>
    bool TwoLists<P, T>::isEmpty() const
    {
        return shortSequence_->isEmpty();
    }

    template<typename P, typename T>
    bool TwoLists<P, T>::equals(const ADT& other)
    {
        if (this != &other) {
            if (this->size() != other.size()) { return false; }

            const TwoLists<P, T>& otherTwoLists = dynamic_cast<const TwoLists<P, T>&>(other);

            return shortSequence_->equals(*otherTwoLists.shortSequence_) && longSequence_->equals(*otherTwoLists.longSequence_);
        }
        else
        {
            return true;
        }
    }

    template<typename P, typename T>
    void TwoLists<P, T>::push(P priority, T data)
    {
        PQItem<P, T>* queueData = nullptr;

        if (this->longSequence_->size() == 0 ||
            !this->shortSequence_->isEmpty() && this->shortSequence_->accessFirst()->data_.priority_ > priority)
        {
            if (this->shortSequence_->size() == this->shortSequence_->getCapacity())
            {
                ShortSequenceBlockType* shortBlock = this->shortSequence_->accessFirst();
                LongSequenceBlockType& longBlock = this->longSequence_->insertLast();

                longBlock.data_.priority_ = shortBlock->data_.priority_;
                longBlock.data_.data_ = shortBlock->data_.data_;

                this->shortSequence_->removeFirst();
            }

            if (this->shortSequence_->isEmpty() ||
                priority < this->shortSequence_->accessLast()->data_.priority_)
            {
                queueData = &(this->shortSequence_->insertLast().data_);
            }
            else
            {
                if (priority > this->shortSequence_->accessFirst()->data_.priority_)
                {
                    queueData = &(this->shortSequence_->insertFirst().data_);
                }
                else
                {
                    auto blockAfter = this->shortSequence_->findBlockWithProperty([&](ShortSequenceBlockType* block)
                        {
                            return block->data_.priority_ <= priority;
                        });
                    queueData = &(this->shortSequence_->insertBefore(*blockAfter).data_);
                }
            }
        }

        else
        {
            queueData = &(this->longSequence_->insertLast().data_);
        }

        queueData->priority_ = priority;
        queueData->data_ = data;
    }

    template<typename P, typename T>
    T& TwoLists<P, T>::peek()
    {
        if (this->shortSequence_->isEmpty())
        {
            this->error("Priority Queue is empty!");
        }

        return this->shortSequence_->accessLast()->data_.data_;
    }

    // ????????????????????????????????????????????
    template<typename P, typename T>
    T TwoLists<P, T>::pop()
    {
        if (this->isEmpty())
        {
            this->error("Priority Queue is empty!");
        }

        T popped = this->shortSequence_->accessLast()->data_.data_;
        this->shortSequence_->removeLast();

        if (this->shortSequence_->size() == 0 && this->longSequence_->size() > 0)
        {
            auto oldLongSequence = this->longSequence_;
            this->longSequence_ = new ds::amt::SinglyLinkedSequence<PQItem<P, T>>();
            this->shortSequence_->changeCapacity(std::ceil(std::sqrt(oldLongSequence->size())));

            while (!oldLongSequence->isEmpty())
            {
                LongSequenceBlockType* block = oldLongSequence->accessFirst();
                this->push(block->data_.priority_, block->data_.data_);
                oldLongSequence->removeFirst();
            }
            delete oldLongSequence;
        }

        return popped;
    }

    template<typename P, typename T>
    BinaryHeap<P, T>::BinaryHeap() :
        ADS<PQItem<P, T>>(new amt::BinaryIH<PQItem<P, T>>())
    {
    }

    template<typename P, typename T>
    BinaryHeap<P, T>::BinaryHeap(const BinaryHeap& other) :
        ADS<PQItem<P, T>>(new amt::BinaryIH<PQItem<P, T>>(), other) // ako APS pride binarna implicitna hierarchia
    {
    }

    template<typename P, typename T>
    void BinaryHeap<P, T>::push(P priority, T data)
    {
        PQItem<P, T>& newItem = getHierarchy()->insertLastLeaf().data_;
        newItem.data_ = data;
        newItem.priority_ = priority;

        auto blockNew = getHierarchy()->accessLastLeaf();
        auto blockParent = getHierarchy()->accessParent(*blockNew);

        while (blockParent != nullptr && blockNew->data_.priority_ < blockParent->data_.priority_)
        {
            std::swap(blockNew->data_, blockParent->data_);
            blockNew = blockParent;                                 // syna posuniem na uroven parenta
            blockParent = getHierarchy()->accessParent(*blockNew);   // parenta posuniem o uroven vyssie (na parentovho parenta)
        }
    }

    template<typename P, typename T>
    T& BinaryHeap<P, T>::peek()
    {
        if (this->isEmpty())
        {
            this->error("Heap is empty!");
        }

        return this->getHierarchy()->accessRoot()->data_.data_;
    }

    template<typename P, typename T>
    T BinaryHeap<P, T>::pop()
    {
        if (this->isEmpty())
        {
            this->error("Priority Queue is empty!");
        }

        HierarchyBlockType* currBlock = this->getHierarchy()->accessRoot();
        T popped = currBlock->data_.data_;

        std::swap(currBlock->data_, this->getHierarchy()->accessLastLeaf()->data_);
        this->getHierarchy()->removeLastLeaf();

        if (!this->isEmpty())
        {
            std::function findMorePrioritizedSon = [&](HierarchyBlockType* parentBlock) -> HierarchyBlockType*
                {
                    HierarchyBlockType* leftSon = this->getHierarchy()->accessLeftSon(*parentBlock);
                    HierarchyBlockType* rightSon = this->getHierarchy()->accessRightSon(*parentBlock);
                    if (rightSon == nullptr || leftSon->data_.priority_ < rightSon->data_.priority_)
                    {
                        return leftSon;
                    }
                    else
                    {
                        return rightSon;
                    }
                };

            HierarchyBlockType* sonBlock = findMorePrioritizedSon(currBlock);

            while (sonBlock != nullptr && currBlock->data_.priority_ > sonBlock->data_.priority_)
            {
                std::swap(currBlock->data_, sonBlock->data_);
                currBlock = sonBlock;
                sonBlock = findMorePrioritizedSon(currBlock);
            }
        }

        return popped;
    }

    template<typename P, typename T>
    amt::BinaryIH<PQItem<P, T>>* BinaryHeap<P, T>::getHierarchy()
    {
        return dynamic_cast<amt::BinaryIH<PQItem<P, T>>*>(this->memoryStructure_);
    }

}