#pragma once

#include <libds/adt/abstract_data_type.h>
#include <libds/amt/implicit_hierarchy.h>
#include <libds/amt/explicit_sequence.h>

namespace ds::adt {

    template <typename T>
    class Stack :
        virtual public ADT
    {
    public:
        virtual void push(T element) = 0;
        virtual T& peek() = 0;
        virtual T pop() = 0;
    };

    //----------

    template<typename T>
    class ImplicitStack :
        public Stack<T>,
        public ADS<T>
    {
    public:
        ImplicitStack();
        ImplicitStack(const ImplicitStack& other);

        void push(T element) override;
        T& peek() override;
        T pop() override;

    private:
        amt::IS<T>* getSequence() const;
    };

    //----------

    template<typename T>
    class ExplicitStack :
        public Stack<T>,
        public ADS<T>
    {
    public:
        ExplicitStack();
        ExplicitStack(const ExplicitStack& other);

        void push(T element) override;
        T& peek() override;
        T pop() override;

    private:
        amt::SinglyLS<T>* getSequence() const;
    };

    //----------

    template<typename T>
    ImplicitStack<T>::ImplicitStack() :
        ADS<T>(new amt::IS<T>())        // ako APS pride IS
    {
    }

    template<typename T>
    ImplicitStack<T>::ImplicitStack(const ImplicitStack& other) :
        ADS<T>(new amt::IS<T>(), other)
    {
    }

    template<typename T>
    void ImplicitStack<T>::push(T element)
    {
        this->getSequence()->insertLast().data_ = element;  // IS dokaze efektivne vkladat na KONIEC (last)
    }

    template<typename T>
    T& ImplicitStack<T>::peek()     // VRCHOL zasobnika = povie, ktory prvok bude najblizsie vybrany (access)
    {
        if (this->isEmpty())
        {
            this->error("Stack is empty!");
        }

        // ak zasobnik NIE je prazdny
        return this->getSequence()->accessLast()->data_;    // vrati pointer na data v poslednom prvku IS (najblizsie budeme vyberat odtial)
    }

    template<typename T>
    T ImplicitStack<T>::pop()       // vyberie prvok a vrati ho ako KOPIU
    {
        if (this->isEmpty())
        {
            this->error("Stack is empty!");
        }

        T popped = this->getSequence()->accessLast()->data_;    // odlozime si data, ktore ideme zahodit
        this->getSequence()->removeLast();                          // I sekvencia vyhodi posledny prvok
        return popped;                                          // vratime KOPIU odstraneneho prvku
    }

    template<typename T>
    amt::IS<T>* ImplicitStack<T>::getSequence() const
    {
        return dynamic_cast<amt::IS<T>*>(this->memoryStructure_);
    }

    template<typename T>
    ExplicitStack<T>::ExplicitStack() :
        ADS<T>(new amt::SinglyLS<T>())      // ako APS pride jednostranne zretazena ES
    {
    }

    template<typename T>
    ExplicitStack<T>::ExplicitStack(const ExplicitStack& other) :
        ADS<T>(new amt::SinglyLS<T>(), other)
    {
    }

    template<typename T>
    void ExplicitStack<T>::push(T element)
    {
        this->getSequence()->insertFirst().data_ = element; // singly linked ES dokaze vzdy efektivne vkladat na ZACIATOK (first)
                // efektivne vklada aj na koniec, ak ma pointer na posledny prvok
        // zasobnik ale funguje na principe, ze vkladame a vyberame z TOHO ISTEHO konca
        // teda musime vybrat taky koniec, kde je efektivne vkladanie aj vyberanie
            // pri ES je efektivne vkladanie na zaciatok aj koniec
            // ale vyberanie LEN zo zaciatku!!
    }

    template<typename T>
    T& ExplicitStack<T>::peek()
    {
        if (this->isEmpty())
        {
            this->error("Stack is empty!");
        }

        return this->getSequence()->accessFirst()->data_;   // najblizsie sa bude vyberat zo zaciatku
    }

    template<typename T>
    T ExplicitStack<T>::pop()
    {
        if (this->isEmpty())
        {
            this->error("Stack is empty!");
        }

        T popped = this->getSequence()->accessFirst()->data_;
        this->getSequence()->removeFirst();
            // v singly linked ES vieme vyberat efektivne LEN zo zaciatku (first) :)
        return popped;
    }

    template<typename T>
    amt::SinglyLS<T>* ExplicitStack<T>::getSequence() const
    {
        return dynamic_cast<amt::SinglyLS<T>*>(this->memoryStructure_);
    }
}

