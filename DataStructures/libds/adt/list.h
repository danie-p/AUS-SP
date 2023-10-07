#pragma once

#include <libds/adt/abstract_data_type.h>
#include <libds/amt/implicit_sequence.h>
#include <libds/amt/explicit_sequence.h>

namespace ds::adt {

    // <T> uz odkazuje len na UDAJE (nie bloky pamate)
    template <typename T>
    class List :
        virtual public ADT
    {
    public:
        virtual size_t calculateIndex(T element) = 0;

        virtual bool contains(T element) = 0;

        virtual T accessFirst() = 0;
        virtual T accessLast() = 0;
        virtual T access(size_t index) = 0;

        virtual void insertFirst(T element) = 0;
        virtual void insertLast(T element) = 0;             // add, push_back..
        virtual void insert(T element, size_t index) = 0;

        virtual void set(size_t index, T element) = 0;

        virtual void removeFirst() = 0;
        virtual void removeLast() = 0;
        virtual void remove(size_t index) = 0;
    };

    //----------

    template <typename T, typename SequenceType>
    class GeneralList :
        virtual public List<T>,
        public ADS<T>
    {
    public:
        using IteratorType = typename SequenceType::IteratorType;

    public:
        GeneralList();
        GeneralList(const GeneralList& other);

        size_t calculateIndex(T element) override;

        bool contains(T element) override;

        T accessFirst() override;
        T accessLast() override;
        T access(size_t index) override;

        void insertFirst(T element) override;
        void insertLast(T element) override;
        void insert(T element, size_t index) override;

        void set(size_t index, T element) override;

        void removeFirst() override;
        void removeLast() override;
        void remove(size_t index) override;

        IteratorType begin();
        IteratorType end();

    protected:
        SequenceType* getSequence() const;
    };

    //----------

    // potomkovia dosadzaju svoje vlastne pamatove struktury (tu rozne sekvencie)
    template <typename T>
    class ImplicitList :
        public GeneralList<T, amt::IS<T>>
    {
    };

    //----------

    template <typename T>
    class ImplicitCyclicList :
        public GeneralList<T, amt::CIS<T>>
    {
    };

    //----------

    template <typename T>
    class SinglyLinkedList :
        public GeneralList<T, amt::SinglyLS<T>>     // general list, kde sekvencia je jednostranne zretazena
    {
    };

    //----------

    template <typename T>
    class SinglyCyclicLinkedList :
        public GeneralList<T, amt::SinglyCLS<T>>
    {
    };

    //----------

    template <typename T>
    class DoublyLinkedList :
        public GeneralList<T, amt::DoublyLS<T>>
    {
    };

    //----------

    template <typename T>
    class DoublyCyclicLinkedList :
        public GeneralList<T, amt::DoublyCLS<T>>
    {
    };

    //----------

    template<typename T, typename SequenceType>
    GeneralList<T, SequenceType>::GeneralList() :
        ADS<T>(new SequenceType())          // zavola sa konstruktor tej sekvencie, ktoru si definoval konkretny potomok 
    {
    }

    template<typename T, typename SequenceType>
    GeneralList<T, SequenceType>::GeneralList(const GeneralList& other) :
        ADS<T>(new SequenceType(), other)
    {
    }

    template<typename T, typename SequenceType>
    size_t GeneralList<T, SequenceType>::calculateIndex(T element)
    {
        // potrebujeme zavolat existujucu prehliadku: findBlockWithProperty: ci ma tie dane data + navysovat index
        // & lebo potrebujem poznat element
        // vysledok prehliadky si ulozime do premennej block
        size_t index = 0;
        auto* block = this->getSequence()->findBlockWithProperty([&](auto* b) {
            if (b->data_ == element)
            {
                return true;
            }
            else {
                ++index;
                return false;
            }       
            });
        
        // ak to bude hned prvy blok, vratil by 1 (lebo uz navysil index) => potrebujeme tu znizit index o 1 ?? is that true?
        // ak sa blok s danym indexom NEnajde, v blocku bude nullptr
        return block != nullptr ? index : INVALID_INDEX;
    }

    template<typename T, typename SequenceType>
    bool GeneralList<T, SequenceType>::contains(T element)
    {
        return this->calculateIndex(element) != INVALID_INDEX;
    }

    template<typename T, typename SequenceType>
    T GeneralList<T, SequenceType>::accessFirst()
    {
        // je jedno, aky konkretny typ sekvencie tam mame
        // auto* lebo vrati pointer na blok pamate
        auto* block = this->getSequence()->accessFirst();
        // vrati nullptr, ak nenajde prvy blok
        if (block != nullptr)           // KONTROLA VSTUPOV
        {
            return block->data_;       // vratime DATA (datovu cast) na bloku, ktory sme ziskali
        }
        else
        {
            throw structure_error("List is empty!");
        }
    }

    template<typename T, typename SequenceType>
    T GeneralList<T, SequenceType>::accessLast()
    {
        auto* block = this->getSequence()->accessLast();

        if (block != nullptr)
        {
            return block->data_;
        }
        else
        {
            throw structure_error("List is empty!");
        }
    }

    template<typename T, typename SequenceType>
    T GeneralList<T, SequenceType>::access(size_t index)
    {
        auto* block = this->getSequence()->access(index);

        if (block != nullptr)
        {
            return block->data_;
        }
        else
        {
            throw structure_error("Cannot access data at invalid index!");
        }
    }

    template<typename T, typename SequenceType>
    void GeneralList<T, SequenceType>::insertFirst(T element)
    {
        this->getSequence()->insertFirst().data_ = element;
    }

    template<typename T, typename SequenceType>
    void GeneralList<T, SequenceType>::insertLast(T element)
    {
        /*
        auto block = getSequence()->insertLast();
        block->data_ = element;
        */
        this->getSequence()->insertLast().data_ = element;
    }

    template<typename T, typename SequenceType>
    void GeneralList<T, SequenceType>::insert(T element, size_t index)
    {
        if (index < 0 || index > this->size())
        {
            throw structure_error("Cannot insert data at invalid index!");
        }
        this->getSequence()->insert(index).data_ = element;
    }

    template<typename T, typename SequenceType>
    void GeneralList<T, SequenceType>::set(size_t index, T element)
    {
        if (index < 0 || index >= this->size())
        {
            throw structure_error("Cannot set data at invalid index!");
        }
        this->getSequence()->access(index)->data_ = element;  // access vracia pointer na blok, takze ->data_
    }

    template<typename T, typename SequenceType>
    void GeneralList<T, SequenceType>::removeFirst()
    {
        if (this->isEmpty())
        {
            throw structure_error("List is empty!");
        }
        this->getSequence()->removeFirst();
    }

    template<typename T, typename SequenceType>
    void GeneralList<T, SequenceType>::removeLast()
    {
        if (this->isEmpty())
        {
            throw structure_error("List is empty!");
        }
        this->getSequence()->removeLast();
    }

    template<typename T, typename SequenceType>
    void GeneralList<T, SequenceType>::remove(size_t index)
    {
        if (index < 0 || index >= this->size())
        {
            throw structure_error("Invalid index!");
        }
        this->getSequence()->remove(index);
    }

    // vieme, ze dvojicku iteratorov ma KAZDA sekvencia => len si ich vratime

    // SequenceType::IteratorType => definovany v konkretnej sekvencii
    template <typename T, typename SequenceType>
    auto GeneralList<T, SequenceType>::begin() -> IteratorType
    {
        return this->getSequence()->begin();
    }

    template <typename T, typename SequenceType>
    auto GeneralList<T, SequenceType>::end() -> IteratorType
    {
        return this->getSequence()->end();
    }

    template<typename T, typename SequenceType>
    SequenceType* GeneralList<T, SequenceType>::getSequence() const
    {
        return dynamic_cast<SequenceType*>(this->memoryStructure_);
    }
}