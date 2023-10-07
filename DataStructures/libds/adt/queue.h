#pragma once

#include <libds/adt/abstract_data_type.h>
#include <libds/amt/implicit_sequence.h>
#include <libds/amt/explicit_sequence.h>

// dolezite je uvedomit si, na ktorych koncoch vyberame / vkladame

namespace ds::adt {

    template <typename T>
    class Queue :
        virtual public ADT
    {
    public:
        virtual void push(T element) = 0;
        virtual T& peek() = 0;
        virtual T pop() = 0;
    };

    //----------

    template<typename T>
    class ImplicitQueue :
        public Queue<T>,
        public ADS<T>
    {
    public:
        ImplicitQueue();
        ImplicitQueue(const ImplicitQueue& other);
        ImplicitQueue(size_t capacity);

        ADT& assign(const ADT& other) override;
        void clear() override;
        size_t size() const override;
        bool isEmpty() const override;
        bool equals(const ADT& other) override;

        void push(T element) override;
        T& peek() override;
        T pop() override;

        static const int INIT_CAPACITY = 100;

    private:
        amt::CIS<T>* getSequence() const;

    private:
        size_t insertionIndex_;
        size_t removalIndex_;
        size_t size_;
    };

    //----------

    template<typename T>
    class ExplicitQueue :
        public Queue<T>,
        public ADS<T>
    {
    public:
        ExplicitQueue();
        ExplicitQueue(const ExplicitQueue& other);
        void push(T element) override;
        T& peek() override;
        T pop() override;

    private:
        amt::SinglyLS<T>* getSequence() const;
    };

    //----------

    template<typename T>
    ImplicitQueue<T>::ImplicitQueue():
        ImplicitQueue(INIT_CAPACITY)
    {
    }

    template<typename T>
    ImplicitQueue<T>::ImplicitQueue(size_t capacity) :
        ADS<T>(new amt::CIS<T>(capacity, true)),
        insertionIndex_(0),
        removalIndex_(0),
        size_(0)
    {
    }

    template<typename T>
    ImplicitQueue<T>::ImplicitQueue(const ImplicitQueue& other) :
        ADS<T>(new amt::CIS<T>(), other),
        insertionIndex_(other.insertionIndex_),
        removalIndex_(other.removalIndex_),
        size_(other.size_)
    {
    }

    template<typename T>
    ADT& ImplicitQueue<T>::assign(const ADT& other)
    {
        const ImplicitQueue<T>* otherImplicitQueue = dynamic_cast<const ImplicitQueue<T>*>(&other);
        if (!(otherImplicitQueue))  // ak iny NIE je typu IQ (teda pretypovanie sa NEpodarilo a vratil sa nullptr)
        {
            this->error("Other ADT is not an Implicit Queue!");
        }

        if (this != otherImplicitQueue)     // NEmozeme priradit sameho seba do seba
        {
            if (this->getSequence()->size() < (*otherImplicitQueue).size())     // ak velkost tohto frontu je mensia ako velkost priradovaneho frontu
            {                                                       // priradenie sa NEmoze uskutocnit
                this->error("Insufficient capacity of Implicit Queue!");
            }

            // ak sa priradenie moze uskutocnit
            this->clear();      // vycistime tento front => pripravime na priradenie

            this->insertionIndex_ = this->getSequence()->indexOfNext((*otherImplicitQueue).size() - 1);
            this->removalIndex_ = 0;
            this->size_ = (*otherImplicitQueue).size();

            size_t removalOther = (*otherImplicitQueue).removalIndex_;
            for (size_t i = 0; i < this->size_ - 1; i++)
            {
                this->getSequence()->access(i)->data_ = (*otherImplicitQueue).getSequence()->access(removalOther)->data_;
                removalOther = (*otherImplicitQueue).getSequence()->indexOfNext(removalOther);
            }
        }

        return *this;
    }

    template<typename T>
    void ImplicitQueue<T>::clear()
    {
        insertionIndex_ = removalIndex_;
        size_ = 0;
    }

    template<typename T>
    size_t ImplicitQueue<T>::size() const
    {
        return size_;
    }

    template<typename T>
    bool ImplicitQueue<T>::isEmpty() const
    {
        return this->size() == 0;
    }

    template<typename T>
    bool ImplicitQueue<T>::equals(const ADT& other)
    {
        const ImplicitQueue<T>* otherImplicitQueue = dynamic_cast<const ImplicitQueue<T>*>(&other);

        if (this == otherImplicitQueue)                  // porovnam adresy
        {
            return true;
        }

        if (this->size() != (*otherImplicitQueue).size())   // porovnam velkosti struktur
        {
            return false;
        }
        
        if (!otherImplicitQueue)            // zistim, ci je typ porovnavaneho IQ
        {
            return false;
        }

        size_t thisIndex = this->removalIndex_;     // index aktualne porovnavaneho prvku inicializujem na koncovy index (odkial sa vyberaju prvky)
        size_t otherIndex = (*otherImplicitQueue).removalIndex_;
        while (thisIndex != this->insertionIndex_)  // postupne prechadza prvok po prvku a porovnava data na rovnakom indexe v oboch strukturach
        {
            if (this->getSequence()->access(thisIndex)->data_ != (*otherImplicitQueue).getSequence()->access(otherIndex)->data_)
            {
                // v momente, ked narazi na rozlicne data na rovnakom indexe v oboch strukturach, porovnanie skonci nepravdou
                return false;
            }
        }
    }

    template<typename T>
    void ImplicitQueue<T>::push(T element)
    {
        if (this->size() == this->getSequence()->size())
        {
            this->error("Capacity of the Queue is full!");
        }
        
        // ak je vo fronte este volne miesto, vlozi sa prvok
        this->getSequence()->access(this->insertionIndex_)->data_ = element;                // spristupni prvok na indexe vkladania a do datovej casti vlozi param prvok
        this->insertionIndex_ = this->getSequence()->indexOfNext(this->insertionIndex_);    // aktualizuje index vkladania na jeho nasledovnika
        ++this->size_;      // navysime velkost
    }

    template<typename T>
    T& ImplicitQueue<T>::peek()
    {
        if (this->isEmpty())
        {
            this->error("Queue is empty!");
        }

        // ak front nie je prazdny
        return this->getSequence()->access(this->removalIndex_)->data_;    // vrati udaje na indexe vyberu
    }

    template<typename T>
    T ImplicitQueue<T>::pop()
    {
        if (this->isEmpty())
        {
            this->error("Queue is empty!");
        }

        T popped = this->getSequence()->access(this->removalIndex_)->data_;             // uchova vyberany prvok (prvok na indexe vyberu)
        this->removalIndex_ = this->getSequence()->indexOfNext(this->removalIndex_);    // aktualizuje index vyberu na jeho nasledovnika

        --this->size_;  // znizi velkost
        return popped;  // vrati vyberany prvok ako kopiu

    }

    template<typename T>
    amt::CIS<T>* ImplicitQueue<T>::getSequence() const
    {
        return dynamic_cast<amt::CIS<T>*>(this->memoryStructure_);
    }

    template<typename T>
    ExplicitQueue<T>::ExplicitQueue() :
        ADS<T>(new amt::SinglyLS<T>())
    {
    }

    template<typename T>
    ExplicitQueue<T>::ExplicitQueue(const ExplicitQueue& other) :
        ADS<T>(new amt::SinglyLS<T>(), other)
    {
    }

    template<typename T>
    void ExplicitQueue<T>::push(T element)
    {
        this->getSequence()->insertLast().data_ = element;
        // vkladame efektivne na KONIEC (last) ES
        // pretoze queue (front) funguje tak, ze vyberame Z INEHO KONCA ako vkladame
        // v ES je vyber efektivny len zo zaciatku, ale vkladanie aj na konci, aj na zaciatku
        // takze vkladat musime na koniec, aby sme mohli vyberat zo zaciatku
    }

    template<typename T>
    T& ExplicitQueue<T>::peek()
    {
        if (this->isEmpty())
        {
            this->error("Queue is empty!");
        }

        return this->getSequence()->accessFirst()->data_;
        // v ES je efektivne vyberame zo zaciatku (first)
    }

    template<typename T>
    T ExplicitQueue<T>::pop()
    {
        if (this->isEmpty())
        {
            this->error("Queue is empty!");
        }

        T popped = this->getSequence()->accessFirst()->data_;
        this->getSequence()->removeFirst();     // z ES odstranime prvok na zaciatku (first)
        return popped;
    }

    template<typename T>
    amt::SinglyLS<T>* ExplicitQueue<T>::getSequence() const
    {
        return dynamic_cast<amt::SinglyLS<T>*>(this->memoryStructure_);
    }
}