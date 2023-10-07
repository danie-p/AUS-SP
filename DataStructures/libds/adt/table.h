#pragma once

#include <libds/adt/abstract_data_type.h>
#include <libds/amt/implicit_sequence.h>
#include <libds/amt/implicit_hierarchy.h>
#include <libds/amt/explicit_hierarchy.h>
#include <functional>
#include <random>

// SP3
// 3 tabulky: zadat, z ktorej chceme vyhladavat podla kluca (nazvu)
// pri duplicitach napr dat na vyber, z ktoreho okresu myslime danu obec (napr visnove)

namespace ds::adt {

    template <typename K, typename T>
    struct TableItem
    {
        K key_;
        T data_;

        bool operator==(const TableItem<K, T>& other) const { return key_ == other.key_ && data_ == other.data_; }
        bool operator!=(const TableItem<K, T>& other) const { return !(*this == other); }
    };

    template <typename K, typename T>
    using TabItem = TableItem<K, T>;

    //----------

    template <typename K, typename T>
    class Table :
        virtual public ADT
    {
    public:
        // mnozine => keby sme nechali len K (lebo v mnozine data su kluce) a vynechali T
        // tryFind, contains, find => v podstate ten isty algoritmus; vieme find, contains naimpl pomocou tryFind
        virtual void insert(K key, T data) = 0;
        virtual T& find(K key);                         // vraciame ako referenciu (ak by sme ju chceli zmenit) => potrebujeme platne pamat. miesto nutne!! => NEmozeme dovolit, aby zlyhalo vyhladavanie (ak nechceme vyhadzovat vynimku) => potrebujeme do find vstupovat s tym, ze urcite dany prvok v tabulke je (pomocou bool tryFind)
        virtual bool tryFind(K key, T*& data) = 0;      // tu osetrime, ci sa podarilo vyhladavanie; posleme data ako pointer => ak tryFind vrati true, v data si isto najdeme ptr na platny prvok
        virtual bool contains(K key);
        virtual T remove(K key) = 0;
    };

    //----------

    template <typename K, typename T, typename SequenceType>
    class SequenceTable :
        public Table<K, T>,
        public ADS<TabItem<K, T>>
    {
    public:
        SequenceTable();
        SequenceTable(const SequenceTable& other);

        bool tryFind(K key, T*& data) override;         // zavolame findBlockWithKey a nastavime jeho data_.data_

    public:
        using BlockType = typename SequenceType::BlockType;
        using IteratorType = typename SequenceType::IteratorType;

        virtual BlockType* findBlockWithKey(K key) = 0; // vracia BLOK; aby sme vedeli vymienat bloky a naimpl bisekciu

        IteratorType begin();
        IteratorType end();

    protected:
        SequenceType* getSequence();
    };

    //----------

    template <typename K, typename T, typename SequenceType>
    class UnsortedSequenceTable :
        public SequenceTable<K, T, SequenceType>
    {
    protected:
        typename SequenceType::BlockType* findBlockWithKey(K key) override; // v IS aj ES: prechadzam a hladam, kym nenajdem
            // pouzit efektivnu prehliadku z rozhrania sekvencie (findBlockWithProperty => kontrolujem, ci param kluc sa zhoduje s klucom bloku)
    };

    template <typename K, typename T, typename SequenceType>
    using UnsortedSTab = UnsortedSequenceTable<K, T, SequenceType>;

    //----------

    template <typename K, typename T>
    class UnsortedImplicitSequenceTable :
        public UnsortedSequenceTable<K, T, amt::IS<TabItem<K, T>>>
    {
    public:
        void insert(K key, T data) override;    // MUSIME skontrolovat, ci v tabulke este nie je prvok s danym klucom (zabezpecit unikatnost kluca) !!
        T remove(K key) override;

    private:
        using BlockType = typename amt::IS<TabItem<K, T>>::BlockType;
    };

    template <typename K, typename T>
    using UnsortedISTab = UnsortedImplicitSequenceTable<K, T>;

    //----------

    template <typename K, typename T>
    class UnsortedExplicitSequenceTable :
        public UnsortedSequenceTable<K, T, amt::SinglyLS<TabItem<K, T>>>
    {
    public:
        void insert(K key, T data) override;
        T remove(K key) override;

    private:
        using BlockType = typename amt::SinglyLS<TabItem<K, T>>::BlockType;
    };

    template <typename K, typename T>
    using UnsortedESTab = UnsortedExplicitSequenceTable<K, T>;

    //----------

    template <typename K, typename T>
    class SortedSequenceTable :
        public SequenceTable<K, T, amt::IS<TabItem<K, T>>>
    {
    public:
        void insert(K key, T data) override;    // vyuzijem bisekciu (pri vkladani musi vyhladanie prvku zlyhat)
        T remove(K key) override;               // pri vyberani musi vyhladavanie byt uspesne

    protected:
        using BlockType = typename amt::IS<TabItem<K, T>>::BlockType;

        BlockType* findBlockWithKey(K key) override;

    private:
        // navratovu hodnotu si vynesiem do param lastBlock (kde algoritmus skoncil => kde bol najdeny existujuci prvok / kde by mal byt vlozeny este neexistujuci prvok)
        // musi vzdy platit OSTRO MENSI first ako last
        // firstIndex = prvy platny prvok; pri NEnajdeni v strede ho nastavime o jeden blok napravo (ak kontrolujeme pravu stranu od stredu - uz sme skontrolovali tento blok ako stred)
        // lastIndex = prvy NElatny prvok; pri NEnajdeni v strede ho nastavime prave na stred (ak kontrolujeme lavu stranu od stredu)
        bool tryFindBlockWithKey(K key, size_t firstIndex, size_t lastIndex, BlockType*& lastBlock);

    };

    template <typename K, typename T>
    using SortedSTab = SortedSequenceTable<K, T>;

    //---------- HASH TABLE ----------------------------------

    template <typename K, typename T>
    class HashTable :
        public Table<K, T>,
        public AUMS<TabItem<K, T>>
    {
    public:
        using HashFunctionType = std::function<size_t(K)>;

    public:
        HashTable();
        HashTable(const HashTable& other);
        HashTable(HashFunctionType hashFunction, size_t capacity);
        ~HashTable();

        ADT& assign(const ADT& other) override;
        bool equals(const ADT& other) override;
        void clear() override;
        size_t size() const override;
        bool isEmpty() const override;

        void insert(K key, T data) override;
        bool tryFind(K key, T*& data) override;
        T remove(K key) override;

    private:
        using SynonymTable = UnsortedESTab<K, T>;
        using SynonymTableIterator = typename SynonymTable::IteratorType;
        using PrimaryRegionIterator = typename amt::IS<SynonymTable*>::IteratorType;

    private:
        static const size_t CAPACITY = 100;

    private:
        amt::IS<SynonymTable*>* primaryRegion_;
        HashFunctionType hashFunction_;
        size_t size_;

    public:
        class HashTableIterator
        {
        public:
            HashTableIterator(PrimaryRegionIterator* tablesFirst, PrimaryRegionIterator* tablesLast);
            HashTableIterator(const HashTableIterator& other);
            ~HashTableIterator();
            HashTableIterator& operator++();
            HashTableIterator operator++(int);
            bool operator==(const HashTableIterator& other) const;
            bool operator!=(const HashTableIterator& other) const;
            TabItem<K, T>& operator*();

        private:
            PrimaryRegionIterator* tablesCurrent_;
            PrimaryRegionIterator* tablesLast_;
            SynonymTableIterator* synonymIterator_;
        };

        using IteratorType = HashTableIterator;

        IteratorType begin() const;

        IteratorType end() const;
    };

    //---------- BINARY SEARCH TREE -------------------------------------------

    template <typename K, typename T, typename ItemType>
    class GeneralBinarySearchTree :
        public Table<K, T>,
        public ADS<TabItem<K, T>>
    {
    public:
        using IteratorType = typename amt::BinaryEH<ItemType>::IteratorType;

    public:
        GeneralBinarySearchTree();
        GeneralBinarySearchTree(const GeneralBinarySearchTree& other);
        ~GeneralBinarySearchTree();

        size_t size() const override;
        bool isEmpty() const override;

        void insert(K key, T data) override;
        bool tryFind(K key, T*& data) override;
        T remove(K key) override;
        void clear() override;

        IteratorType begin() const;
        IteratorType end() const;

    protected:
        using BVSNodeType = typename amt::BinaryEH<ItemType>::BlockType;

        amt::BinaryEH<ItemType>* getHierarchy() const;

        // tieto 3 vyuzivaju tryFind_
        virtual BVSNodeType* findNodeWithRelation(K key);
        virtual BVSNodeType& insertNode(K key, BVSNodeType* relative);
        virtual void removeNode(BVSNodeType* node);

        virtual void balanceTree(BVSNodeType* node) { }

        // zacne v koreni, ak nasiel vrchol, zastane, ak nie, vyberie si vhodnu podhierarchiu, ktoru bude skumat
        // ako param node vrati vrchol, na ktorom algoritmus skoncil
        bool tryFindNodeWithKey(K key, BVSNodeType*& node) const;

        void rotateLeft(BVSNodeType* node);
        void rotateRight(BVSNodeType* node);

    // private:
        size_t size_;
    };

    //----------

    template <typename K, typename T>
    class BinarySearchTree :
        public GeneralBinarySearchTree<K, T, TabItem<K, T>>
    {
    };

    //---------- TREAP ----------------------------------------------------

    template <typename K, typename T>
    struct TreapItem:
        public TabItem<K, T>
    {
        // treap potrebuje pridat prioritu
        int priority_;
    };

    template <typename K, typename T>
    class Treap :
        public GeneralBinarySearchTree<K, T, TreapItem<K, T>>
    {
    public:
        Treap();

    protected:
        using BVSNodeType = typename GeneralBinarySearchTree<K, T, TreapItem<K, T>>::BVSNodeType;

        void removeNode(BVSNodeType* node) override;
        void balanceTree(BVSNodeType* node) override;

    private:
        std::default_random_engine rng_;
    };

    //----------

    template <typename K, typename T, typename ListDataType>
    class ModifiedTreap :
        public Treap<K, T*>
    {
    public:
        void insert(K key, ListDataType listData);

    protected:
        using BVSNodeType = typename Treap<K, T*>::BVSNodeType;
    };

    //----------

    template<typename K, typename T>
    T& Table<K, T>::find(K key)
    {
        T* data = nullptr;
        if (!this->tryFind(key, data))
        {
            throw structure_error("No such key!");
        }
        return *data;
    }

    template<typename K, typename T>
    bool Table<K, T>::contains(K key)
    {
        T* data = nullptr;
        return this->tryFind(key, data);
    }

    //----------

    template<typename K, typename T, typename SequenceType>
    SequenceTable<K, T, SequenceType>::SequenceTable() :
        ADS<TabItem<K, T>>(new SequenceType())
    {
    }

    template<typename K, typename T, typename SequenceType>
    SequenceTable<K, T, SequenceType>::SequenceTable(const SequenceTable& other) :
        ADS<TabItem<K, T>>(new SequenceType(), other)
    {
    }

    template<typename K, typename T, typename SequenceType>
    bool SequenceTable<K, T, SequenceType>::tryFind(K key, T*& data)
    {
        BlockType* blockWithKey = this->findBlockWithKey(key);  // zavolame najdenie BLOKU

        if (!blockWithKey)      // ak najdenie bloku vrati nullptr, prvok s klucom sa nenasiel
        {
            return false;
        }

        // ak sa blok s klucom nasiel
        data = &(blockWithKey->data_.data_);    // nastavime param data (referencia!) na datovu cast TableItemu v najdenom bloku
        return true;
    }

    template <typename K, typename T, typename SequenceType>
    auto SequenceTable<K, T, SequenceType>::begin() -> IteratorType
    {
        return this->getSequence()->begin();
    }

    template <typename K, typename T, typename SequenceType>
    auto SequenceTable<K, T, SequenceType>::end() -> IteratorType
    {
        return this->getSequence()->end();
    }

    template<typename K, typename T, typename SequenceType>
    SequenceType* SequenceTable<K, T, SequenceType>::getSequence()
    {
        return dynamic_cast<SequenceType*>(this->memoryStructure_);
    }

    //----------

    template<typename K, typename T, typename SequenceType>
    typename SequenceType::BlockType* UnsortedSequenceTable<K, T, SequenceType>::findBlockWithKey(K key)
    {
        // na najdenie bloku pouzijeme prislusnu metodu sekvencie
        return this->getSequence()->findBlockWithProperty([&](auto block)
            {
                // ako lambda predikat kontrolujeme, ci kluc aktualne kontrolovaneho bloku sa zhoduje s param klucom
                return block->data_.key_ == key;
            });
    }

    //---------- UNsorted IS

    template<typename K, typename T>
    void UnsortedImplicitSequenceTable<K, T>::insert(K key, T data)
    {
        // musime zabezpecit unikatnost kluca => nesmieme vlozit prvok s klucom, kt v tabulke uz je!
        if (this->contains(key))
        {
            this->error("Table already contains an element with given key!");
        }

        // vlozime na KONIEC (last) IS
        TabItem<K, T>* tableItem = &(this->getSequence()->insertLast().data_);
        tableItem->key_ = key;
        tableItem->data_ = data;
    }

    template<typename K, typename T>
    T UnsortedImplicitSequenceTable<K, T>::remove(K key)
    {
        // najdeme si BLOK s klucom
        BlockType* blockWithKey = this->findBlockWithKey(key);

        if (!blockWithKey)
        {
            // ak sa blok nenasiel (vratil sa nullptr)
            this->error("Table doesn't contain an element with given key!");
        }

        T removed = blockWithKey->data_.data_;
        BlockType* lastBlock = this->getSequence()->accessLast();   // z IS zoberieme POSLEDNY blok

        if (blockWithKey != lastBlock)
        {
            std::swap(blockWithKey->data_, lastBlock->data_);
        }

        this->getSequence()->removeLast();  // z IS vieme efektivne odstranovat z KONCA (last)
        return removed;
    }

    //---------- UNsorted ES

    template<typename K, typename T>
    void UnsortedExplicitSequenceTable<K, T>::insert(K key, T data)
    {
        if (this->contains(key))
        {
            this->error("Table already contains an element with given key!");
        }

        // vlozime na ZACIATOK (first) ES
        TabItem<K, T>* tableItem = &(this->getSequence()->insertFirst().data_);
        tableItem->key_ = key;
        tableItem->data_ = data;
    }

    template<typename K, typename T>
    T UnsortedExplicitSequenceTable<K, T>::remove(K key)
    {
        BlockType* blockWithKey = this->findBlockWithKey(key);

        if (!blockWithKey)
        {
            this->error("Table doesn't contain an element with given key!");
        }

        T removed = blockWithKey->data_.data_;
        BlockType* firstBlock = this->getSequence()->accessFirst(); // z ES si vyberieme PRVY prvok

        if (blockWithKey != firstBlock)
        {
            std::swap(blockWithKey->data_, firstBlock->data_);
        }

        this->getSequence()->removeFirst();     // z ES vieme efektivne odstranovat PRVY (first) prvok
        return removed;
    }

    //----------

    template<typename K, typename T>
    void SortedSequenceTable<K, T>::insert(K key, T data)
    {
        TabItem<K, T>* tabData = nullptr;

        if (this->isEmpty())
        {
            tabData = &(this->getSequence()->insertFirst().data_);
        }
        else
        {
            BlockType* blockWithKey = nullptr;
            // ak sa podari (vrati true) tryFind
            if (this->tryFindBlockWithKey(key, 0, this->size(), blockWithKey))
            {
                this->error("Table already contains an element with given key!");
            }

            tabData = key > blockWithKey->data_.key_ ?
                &(this->getSequence()->insertAfter(*blockWithKey).data_) :
                &(this->getSequence()->insertBefore(*blockWithKey).data_);
        }

        tabData->key_ = key;
        tabData->data_ = data;
    }

    template<typename K, typename T>
    T SortedSequenceTable<K, T>::remove(K key)
    {
        BlockType* blockWithKey = nullptr;

        // ak sa NEpodari (vrati false) tryFind
        if (!this->tryFindBlockWithKey(key, 0, this->size(), blockWithKey))
        {
            this->error("Table doesn't contain an element with given key!");
        }

        T removed = blockWithKey->data_.data_;

        if (this->getSequence()->accessFirst() == blockWithKey)
        {
            this->getSequence()->removeFirst();
        }
        else
        {
            this->getSequence()->removeNext(*this->getSequence()->accessPrevious(*blockWithKey));
        }

        return removed;
    }

    template<typename K, typename T>
    auto SortedSequenceTable<K, T>::findBlockWithKey(K key) -> BlockType*
    {
        BlockType* blockWithKey = nullptr;
        // ak tryFind odpovie true => vratime dany platny blok (ktory tryFind zapise do param blockWithKey)
        // ak false => vratime nullptr
        return this->tryFindBlockWithKey(key, 0, this->size(), blockWithKey) ? blockWithKey : nullptr;
    }

    template<typename K, typename T>
    bool SortedSequenceTable<K, T>::tryFindBlockWithKey(K key, size_t firstIndex, size_t lastIndex, BlockType*& lastBlock)
    {
        // pri vkladani musi tato metoda vratit false
            // v lastBlocku si najdeme blok, kde alg skoncil a pred/za ktory by sme mali vlozit novy prvok
                // pred/za skontrolujeme porovnanim klucov

        // pri mazani musi vratit true
            // v lastBlocku mame blok s klucom => odlozime si mazany blok
                // ideme mazat: NEmozeme vymenit s poslednym (pokazili by sme utriedenie)

        if (this->isEmpty())
        {
            lastBlock = nullptr;
            return false;
        }

        size_t centreIndex = firstIndex;    // zaciname od prveho param indexu

        while (firstIndex < lastIndex)
        {
            // bezpecny vypocet stredu bez overflowu
            centreIndex = firstIndex + (lastIndex - firstIndex) / 2;

            // lastBlock pride ako param referencia, do ktoreho mozem zapisovat
            lastBlock = this->getSequence()->access(centreIndex);

            if (lastBlock->data_.key_ < key)
            {
                firstIndex = centreIndex + 1;
            }
            else if (lastBlock->data_.key_ > key)
            {
                lastIndex = centreIndex;
            }
            else
            {
                break;
            }
        }

        // NEmusi byt (asi):
        // do lastBlock pre istotu nastavime prvok v strede, lebo pri jednoprvkovej tabulke by sa cyklus preskocil, ale musime ho aj tak nastavit
        // lastBlock = this->getSequence()->access(centreIndex);

        return lastBlock->data_.key_ == key;
    }

    //---------- HASH TABLE ---------------------------------------------------

    template<typename K, typename T>
    HashTable<K, T>::HashTable() :
        HashTable([](K key) { return std::hash<K>()(key); }, CAPACITY)
    {
    }

    template <typename K, typename T>
    HashTable<K, T>::HashTable(const HashTable& other) :
        primaryRegion_(new amt::IS<SynonymTable*>(other.primaryRegion_->size(), true)),
        hashFunction_(other.hashFunction_),
        size_(0)
    {
        assign(other);
    }

    template<typename K, typename T>
    HashTable<K, T>::HashTable(HashFunctionType hashFunction, size_t capacity) :
        primaryRegion_(new amt::IS<SynonymTable*>(capacity, true)),
        hashFunction_(hashFunction),
        size_(0)
    {
    }

    template <typename K, typename T>
    HashTable<K, T>::~HashTable()
    {
        this->clear();
        delete primaryRegion_;
    }

    template <typename K, typename T>
    ADT& HashTable<K, T>::assign(const ADT& other)
    {
        if (this != &other)
        {
            const HashTable& otherTable = dynamic_cast<const HashTable&>(other);
            this->clear();
            for (TabItem<K, T>& otherItem : otherTable)
            {
                this->insert(otherItem.key_, otherItem.data_);
            }
        }

        return *this;
    }

    template <typename K, typename T>
    bool HashTable<K, T>::equals(const ADT& other)
    {
        if (this == &other) { return true; }
        if (this->size() != other.size()) { return false; }

        const HashTable& otherTable = dynamic_cast<const HashTable&>(other);
        for (TabItem<K, T>& otherItem : otherTable)
        {
            T* otherData = nullptr;
            if (!this->tryFind(otherItem.key_, otherData) || *otherData != otherItem.data_)
            {
                return false;
            }
        }
        return true;
    }

    template <typename K, typename T>
    void HashTable<K, T>::clear()
    {
        size_ = 0;
        primaryRegion_->processAllBlocksForward([](typename amt::IS<SynonymTable*>::BlockType* blokSynoným)
            {
                delete blokSynoným->data_;
                blokSynoným->data_ = nullptr;
            });
    }

    template <typename K, typename T>
    size_t HashTable<K, T>::size() const
    {
        return size_;
    }

    template <typename K, typename T>
    bool HashTable<K, T>::isEmpty() const
    {
        return size() == 0;
    }

    // impl
    template <typename K, typename T>
    void HashTable<K, T>::insert(K key, T data)
    {
        size_t index = hashFunction_(key) % primaryRegion_->size();
        SynonymTable* synonyms = primaryRegion_->access(index)->data_;

        if (synonyms == nullptr)
        {
            synonyms = new SynonymTable();
            primaryRegion_->access(index)->data_ = synonyms;
        }

        synonyms->insert(key, data);
        ++size_;
    }

    // impl
    template <typename K, typename T>
    bool HashTable<K, T>::tryFind(K key, T*& data)
    {
        size_t index = hashFunction_(key) % primaryRegion_->size();
        SynonymTable* synonyms = primaryRegion_->access(index)->data_;

        return synonyms == nullptr ? false : synonyms->tryFind(key, data);
    }

    // impl
    template <typename K, typename T>
    T HashTable<K, T>::remove(K key)
    {
        size_t index = hashFunction_(key) % primaryRegion_->size();
        SynonymTable* synonyms = primaryRegion_->access(index)->data_;

        if (synonyms == nullptr)
        {
            this->error("Table doesn't contain an element with given key!");
        }

        T element = synonyms->remove(key);

        if (synonyms->isEmpty())
        {
            delete synonyms;
            primaryRegion_->access(index)->data_ = nullptr;
        }

        --size_;
        return element;
    }

    template <typename K, typename T>
    HashTable<K, T>::HashTableIterator::HashTableIterator
        (PrimaryRegionIterator* tablesFirst, PrimaryRegionIterator* tablesLast) :
        tablesCurrent_(tablesFirst),
        tablesLast_(tablesLast)
    {
        while (*tablesCurrent_ != *tablesLast_ && **tablesCurrent_ == nullptr)
        {
            ++(*tablesCurrent_);
        }
        synonymIterator_ = *tablesCurrent_ != *tablesLast_
            ? new SynonymTableIterator((**tablesCurrent_)->begin())
            : nullptr;
    }

    template <typename K, typename T>
    HashTable<K, T>::HashTableIterator::HashTableIterator
        (const HashTableIterator& other) :
        tablesCurrent_(other.tablesCurrent_),
        tablesLast_(other.tablesLast_),
        synonymIterator_(other.synonymIterator_)
    {
    }

    template <typename K, typename T>
    HashTable<K, T>::HashTableIterator::~HashTableIterator()
    {
        delete tablesCurrent_;
        delete tablesLast_;
        delete synonymIterator_;
    }

    // impl
    template <typename K, typename T>
    auto HashTable<K, T>::HashTableIterator::operator++() -> HashTableIterator&
    {
        ++(*this->synonymIterator_);

        SynonymTableIterator origSynonymIterator = *this->synonymIterator_;
        // ak iterator synonym uz nema dalsi (je rovny koncu a nema uz kde ist)
        if (!(origSynonymIterator != (**tablesCurrent_)->end()))
        {
            do
            {
                // iterator primarnej oblasti sa posunie vpred
                ++(*this->tablesCurrent_);
            // pokial iterator primarnej oblasti ma dalsi (aktualny sa nerovna koncovemu) A zaroven prvok, na ktory ukazuje iterator primarnej oblasti, je nullptr
            } while (*this->tablesCurrent_ != *this->tablesLast_ && **this->tablesCurrent_ == nullptr);

            delete this->synonymIterator_;
            this->synonymIterator_ = nullptr;

            // ak iterator primarnej oblasti ma dalsi (aktualny sa nerovna koncovemu)
            if (*this->tablesCurrent_ != *this->tablesLast_)
            {
                // vytvor novy iterator synonym ukazujuci na prvy iterator
                this->synonymIterator_ = new SynonymTableIterator ((**tablesCurrent_)->begin());
            }
        }

        return *this;
    }

    template <typename K, typename T>
    auto HashTable<K, T>::HashTableIterator::operator++(int) -> HashTableIterator
    {
        HashTableIterator tmp(*this);
        operator++();
        return tmp;
    }

    template <typename K, typename T>
    bool HashTable<K, T>::HashTableIterator::operator==(const HashTableIterator& other) const
    {
        return synonymIterator_ == other.synonymIterator_ ||
                 (synonymIterator_ != nullptr &&
                   other.synonymIterator_ != nullptr &&
                   *synonymIterator_ == *(other.synonymIterator_));
    }

    template <typename K, typename T>
    bool HashTable<K, T>::HashTableIterator::operator!=(const HashTableIterator& other) const
    {
        return !(*this == other);
    }

    template <typename K, typename T>
    TabItem<K, T>& HashTable<K, T>::HashTableIterator::operator*()
    {
        return **synonymIterator_;
    }

    //----------

    template <typename K, typename T>
    auto HashTable<K, T>::begin() const -> IteratorType
    {
        return HashTableIterator(
            new PrimaryRegionIterator(primaryRegion_->begin()),
            new PrimaryRegionIterator(primaryRegion_->end())
        );
    }

    template <typename K, typename T>
    auto HashTable<K, T>::end() const -> IteratorType
    {
        return HashTableIterator(
            new PrimaryRegionIterator(primaryRegion_->end()),
            new PrimaryRegionIterator(primaryRegion_->end())
        );
    }

    //---------- BINARY SEARCH TREE --------------------------------------------

    template<typename K, typename T, typename ItemType>
    GeneralBinarySearchTree<K, T, ItemType>::GeneralBinarySearchTree():
        ADS<TabItem<K, T>>(new amt::BinaryEH<ItemType>()),
        size_(0)
    {
    }

    template<typename K, typename T, typename ItemType>
    GeneralBinarySearchTree<K, T, ItemType>::GeneralBinarySearchTree(const GeneralBinarySearchTree& other):
        ADS<TabItem<K, T>>(new amt::BinaryEH<ItemType>(), other),
        size_(other.size_)
    {
    }

    template<typename K, typename T, typename ItemType>
    GeneralBinarySearchTree<K, T, ItemType>::~GeneralBinarySearchTree()
    {
        size_ = 0;
    }

    template<typename K, typename T, typename ItemType>
    size_t GeneralBinarySearchTree<K, T, ItemType>::size() const
    {
        return size_;
    }

    template<typename K, typename T, typename ItemType>
    bool GeneralBinarySearchTree<K, T, ItemType>::isEmpty() const
    {
        return size_ == 0;
    }

    template<typename K, typename T, typename ItemType>
    void GeneralBinarySearchTree<K, T, ItemType>::insert(K key, T data)
    {
        // BVS si bude sam napocitavat svoju velkost => lebo v hierarchii je metoda velkost O(n) (musi prejst vsetkymi vrcholmi a spocitat)
        // v pseudokode je otec = vrchol vo vztahu
        BVSNodeType* newNode = nullptr;
        if (this->isEmpty())
        {
            newNode = &this->getHierarchy()->emplaceRoot();
        }
        else
        {
            BVSNodeType* parentNode = nullptr;
            if (this->tryFindNodeWithKey(key, parentNode))
            {
                this->error("Table already contains an element with given key!");
            }

            // ak dany vrchol este neexistuje, mozeme ho vlozit ako spravneho syna svojho otca (vrchola, kde skoncil alg tryFind)
            newNode = key > parentNode->data_.key_ ?
                &this->getHierarchy()->insertRightSon(*parentNode) :
                &this->getHierarchy()->insertLeftSon(*parentNode);
        }

        newNode->data_.key_ = key;
        newNode->data_.data_ = data;
        ++size_;
        this->balanceTree(newNode);
    }

    template<typename K, typename T, typename ItemType>
    bool GeneralBinarySearchTree<K, T, ItemType>::tryFind(K key, T*& data)
    {
        BVSNodeType* searched = nullptr;

        if (!this->tryFindNodeWithKey(key, searched))
        {
            return false;
        }

        data = &searched->data_.data_;
        return true;
    }

    template<typename K, typename T, typename ItemType>
    T GeneralBinarySearchTree<K, T, ItemType>::remove(K key)
    {
        // vseobecne plati pri odstranovani akehokolvek vrchola z BVS
        BVSNodeType* removed = nullptr;

        // ak sa tryFind nepodari najst prvok s danym klucom, tak v BVS nie je 
        if (!this->tryFindNodeWithKey(key, removed))
        {
            this->error("Table doesn't contain an element with given key!");
        }

        // pred odstranenim si zachranime data
        T removedData = removed->data_.data_;
        this->removeNode(removed);
        --size_;

        return removedData;
    }

    template<typename K, typename T, typename ItemType>
    inline void GeneralBinarySearchTree<K, T, ItemType>::clear()
    {
        ADS<TabItem<K, T>>::clear();
        size_ = 0;
    }

    template <typename K, typename T, typename ItemType>
    auto GeneralBinarySearchTree<K, T, ItemType>::begin() const -> IteratorType
    {
        return this->getHierarchy()->begin();
    }

    template <typename K, typename T, typename ItemType>
    auto GeneralBinarySearchTree<K, T, ItemType>::end() const -> IteratorType
    {
        return this->getHierarchy()->end();
    }

    template<typename K, typename T, typename ItemType>
    amt::BinaryEH<ItemType>* GeneralBinarySearchTree<K, T, ItemType>::getHierarchy() const
    {
        return dynamic_cast<amt::BinaryEH<ItemType>*>(this->memoryStructure_);
    }

    template<typename K, typename T, typename ItemType>
    auto GeneralBinarySearchTree<K, T, ItemType>::findNodeWithRelation(K key) -> BVSNodeType*
    {
        BVSNodeType* node = nullptr;
        this->tryFindNodeWithKey(key, node);
        return node;
    }

    template<typename K, typename T, typename ItemType>
    auto GeneralBinarySearchTree<K, T, ItemType>::insertNode(K key, BVSNodeType* relative) -> BVSNodeType&
    {
        return key > relative->data_.key_
            ? this->getHierarchy()->insertRightSon(*relative)
            : this->getHierarchy()->insertLeftSon(*relative);
    }

    // treap prekryva len metodu balanceTree a removeNode

    template<typename K, typename T, typename ItemType>
    void GeneralBinarySearchTree<K, T, ItemType>::removeNode(BVSNodeType* node)
    {
        BVSNodeType* parent = this->getHierarchy()->accessParent(*node);

        switch (this->getHierarchy()->degree(*node))
        {
        // ak ideme mazat list
        case 0:
        {
            // ked je vrchol zaroven koren aj list => jediny vrchol => vymazat
            if (this->getHierarchy()->isRoot(*node))
            {
                this->getHierarchy()->clear();
            }
            // ak je odstranovany vrchol lavym synom
            else if (this->getHierarchy()->isLeftSon(*node))
            {
                // u jeho otca odstranime laveho syna
                this->getHierarchy()->removeLeftSon(*parent);
            }
            // ak je odstranovany vrchol pravym synom
            else
            {
                // u jeho otca odstranime praveho syna
                this->getHierarchy()->removeRightSon(*parent);
            }
            break;
        }
        // ak ideme mazat vrchol s 1 synom
        case 1:
        {
            BVSNodeType* son = this->getHierarchy()->hasLeftSon(*node) ?
                this->getHierarchy()->accessLeftSon(*node) :
                this->getHierarchy()->accessRightSon(*node);

            if (this->getHierarchy()->isLeftSon(*son))
            {
                this->getHierarchy()->changeLeftSon(*node, nullptr);
            }
            else
            {
                this->getHierarchy()->changeRightSon(*node, nullptr);
            }

            if (this->getHierarchy()->isRoot(*node))
            {
                this->getHierarchy()->clear();
                this->getHierarchy()->changeRoot(son);
            }
            else if (this->getHierarchy()->isLeftSon(*node))
            {
                this->getHierarchy()->removeLeftSon(*parent);
                this->getHierarchy()->changeLeftSon(*parent, son);
            }
            else
            {
                this->getHierarchy()->removeRightSon(*parent);
                this->getHierarchy()->changeRightSon(*parent, son);
            }
            break;
        }
        // ak ma odstranovany vrchol 2 synov
        case 2:
        {
            BVSNodeType* inOrderPrev = this->getHierarchy()->accessLeftSon(*node);

            while (this->getHierarchy()->hasRightSon(*inOrderPrev))
            {
                inOrderPrev = this->getHierarchy()->accessRightSon(*inOrderPrev);
            }
            std::swap(node->data_, inOrderPrev->data_);
            this->removeNode(inOrderPrev);

            break;
        }
        }
    }

    template<typename K, typename T, typename ItemType>
    bool GeneralBinarySearchTree<K, T, ItemType>::tryFindNodeWithKey(K key, BVSNodeType*& node) const
    {
        // optimisticky O(log(n))
        // horny asympt odhad je ale O(n)

        if (this->isEmpty())
        {
            node = nullptr;
            return false;
        }

        node = this->getHierarchy()->accessRoot();
        while (node->data_.key_ != key && !this->getHierarchy()->isLeaf(*node))
        {
            if (key < node->data_.key_)
            {
                if (this->getHierarchy()->hasLeftSon(*node))
                {
                    node = this->getHierarchy()->accessLeftSon(*node);
                }
                else
                {
                    return false;
                }
            }
            else if (this->getHierarchy()->hasRightSon(*node))
            {
                node = this->getHierarchy()->accessRightSon(*node);
            }
            else
            {
                return false;
            }
        }

        return node->data_.key_ == key;
    }

    // rotacie budu skuskova otazka <3
    template<typename K, typename T, typename ItemType>
    void GeneralBinarySearchTree<K, T, ItemType>::rotateLeft(BVSNodeType* node)
    {
        BVSNodeType* leftSon = this->getHierarchy()->accessLeftSon(*node);
        BVSNodeType* parent = this->getHierarchy()->accessParent(*node);
        BVSNodeType* grandParent = this->getHierarchy()->accessParent(*parent);

        this->getHierarchy()->changeRightSon(*parent, nullptr);
        this->getHierarchy()->changeLeftSon(*node, nullptr);

        if (grandParent != nullptr)
        {
            if (this->getHierarchy()->accessLeftSon(*grandParent) == parent)
            {
                this->getHierarchy()->changeLeftSon(*grandParent, node);
            }
            else
            {
                this->getHierarchy()->changeRightSon(*grandParent, node);
            }
        }
        else
        {
            this->getHierarchy()->changeRoot(node);
        }

        this->getHierarchy()->changeRightSon(*parent, leftSon);
        this->getHierarchy()->changeLeftSon(*node, parent);
    }

    template<typename K, typename T, typename ItemType>
    void GeneralBinarySearchTree<K, T, ItemType>::rotateRight(BVSNodeType* node)
    {
        BVSNodeType* rightSon = this->getHierarchy()->accessRightSon(*node);
        BVSNodeType* parent = this->getHierarchy()->accessParent(*node);
        BVSNodeType* grandParent = this->getHierarchy()->accessParent(*parent);

        this->getHierarchy()->changeLeftSon(*parent, nullptr);
        this->getHierarchy()->changeRightSon(*node, nullptr);

        if (grandParent != nullptr)
        {
            if (this->getHierarchy()->accessLeftSon(*grandParent) == parent)
            {
                this->getHierarchy()->changeLeftSon(*grandParent, node);
            }
            else
            {
                this->getHierarchy()->changeRightSon(*grandParent, node);
            }
        }
        else
        {
            this->getHierarchy()->changeRoot(node);
        }

        this->getHierarchy()->changeLeftSon(*parent, rightSon);
        this->getHierarchy()->changeRightSon(*node, parent);
    }

    //---------- TREAP ----------------------------------------------------------------

    template<typename K, typename T>
    Treap<K, T>::Treap():
        rng_(std::rand())
    {
    }

    // impl
    template<typename K, typename T>
    void Treap<K, T>::removeNode(BVSNodeType* node)
    {
        // umelo navysime prioritu a tlacit vrchol dole pomocou rotacii, kym nema 0/1 syna
            // rotujeme podla toho, ktory syn ma vyssiu prioritu 
        
        node->data_.priority_ = (std::numeric_limits<int>::max)();

        while (this->getHierarchy()->degree(*node) == 2)
        {
            BVSNodeType* leftSon = this->getHierarchy()->accessLeftSon(*node);
            BVSNodeType* rightSon = this->getHierarchy()->accessRightSon(*node);

            if (leftSon->data_.priority_ < rightSon->data_.priority_)
            {
                this->rotateRight(leftSon);
            }
            else
            {
                this->rotateLeft(rightSon);
            }
        }

        GeneralBinarySearchTree<K, T, TreapItem<K, T>>::removeNode(node);
    }

    // impl
    template<typename K, typename T>
    void Treap<K, T>::balanceTree(BVSNodeType* node)
    {
        // to iste ako v lavostrannej halde, len namiesto swapovania vrcholov ich rotujeme
        
        std::uniform_int_distribution<int> prioDistribution(
            (std::numeric_limits<int>::min)(),
            (std::numeric_limits<int>::max)());
        node->data_.priority_ = prioDistribution(rng_);

        BVSNodeType* parent = this->getHierarchy()->accessParent(*node);

        while (parent != nullptr && parent->data_.priority_ > node->data_.priority_)
        {
            if (this->getHierarchy()->accessLeftSon(*parent) == node)
            {
                this->rotateRight(node);
            }
            else
            {
                this->rotateLeft(node);
            }

            parent = this->getHierarchy()->accessParent(*node);
        }
    }

    //---------- MODIFIED TREAP

    template<typename K, typename T, typename ListDataType>
    inline void ModifiedTreap<K, T, ListDataType>::insert(K key, ListDataType listData)     // T = typ struktury, do ktorej sa budu ukladat prvky; ListDataType = typ prvkov ukladanych do struktur
    {
        BVSNodeType* newNode = nullptr;
            if (this->isEmpty())
            {
                newNode = &this->getHierarchy()->emplaceRoot();

                newNode->data_.key_ = key;
                newNode->data_.data_ = new T;
                newNode->data_.data_->insertLast(listData);

                ++this->size_;
                this->balanceTree(newNode);
            }
            else
            {
                BVSNodeType* parentNode = nullptr;
                if (this->tryFindNodeWithKey(key, parentNode))
                {
                    // v parentNode je referencia na existujuci vrchol, v ktorom algoritmus skoncil vyhladavanie
                    // k nemu treba pridat duplicitny zaznam
                    
                    parentNode->data_.data_->insertLast(listData);
                }
                else
                {
                    newNode = key > parentNode->data_.key_ ?
                        &this->getHierarchy()->insertRightSon(*parentNode) :
                        &this->getHierarchy()->insertLeftSon(*parentNode);

                    newNode->data_.key_ = key;
                    newNode->data_.data_ = new T;
                    newNode->data_.data_->insertLast(listData);

                    ++this->size_;
                    this->balanceTree(newNode);
                }
            }
    }
}