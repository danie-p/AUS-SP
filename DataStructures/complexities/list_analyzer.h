#pragma once

#include <complexities/complexity_analyzer.h>
#include <complexities/access_analyzer.h>
#include <iterator>
#include <list>
#include <random>
#include <vector>
#include <libds/adt/list.h>

// TabAnalyzer
// metoda na vkladanie N prvkov (insert podla kluca)
    // do predka sa to posielalo v konstruktore
// urobime 2 potomkov TabAnalyzera: InsertTabAnalyzer, FindTabAnalyzer => prekryjeme metodu insert, find (find z tabulky hocijakej)
// TabAnalyzer si zapise vsetky PLATNE kluce do nejakeho vectora; prehodim so spravnym koncom a vratim
    // alebo do priority queue vkladat kluce s nahodnou prioritou (potomok spravi len pop)
// kluc si nahodne vygenerujem v before Op

namespace ds::utils
{
    /**
     * @brief Common base for list analyzers.
     */
    template<class List>                                    // sablonu vola List (aby sme vedeli, ze je to uz konkretne pre ListAnalyzer)
    class ListAnalyzer : public ComplexityAnalyzer<List>
    {
    protected:
        explicit ListAnalyzer(const std::string& name);

    protected:
        void beforeOperation(List& structure) override;
        size_t getRandomIndex() const;
        int getRandomData() const;

    private:
        void insertNElements(List& list, size_t n);         // n prvkov vlozime do listu normalne pomocou foru

    private:
        std::default_random_engine rngData_;
        std::default_random_engine rngIndex_;
        size_t index_;
        int data_;
    };

    /**
     * @brief Analyzes complexity of the insert operation.
     */
    template<class List>
    class ListInsertAnalyzer : public ListAnalyzer<List>
    {
    public:
        explicit ListInsertAnalyzer(const std::string& name);

    protected:
        void executeOperation(List& structure) override;
    };

    /**
     * @brief Analyzes complexity of the erase operation.
     */
    template<class List>
    class ListRemoveAnalyzer : public ListAnalyzer<List>
    {
    public:
        explicit ListRemoveAnalyzer(const std::string& name);

    protected:
        void executeOperation(List& structure) override;
    };

    /**
     * @brief Container for all list analyzers.
     */
    class ListsAnalyzer : public CompositeAnalyzer
    {
    public:
        ListsAnalyzer() :
            CompositeAnalyzer("Lists")
        {
            this->addAnalyzer(std::make_unique<ListInsertAnalyzer<std::vector<int>>>("vector-insert"));     // ako typ testovanej struktury prebera std::vector
            this->addAnalyzer(std::make_unique<ListInsertAnalyzer<std::list<int>>>("list-insert"));         // ako typ testovanej struktury prebera std::list
            this->addAnalyzer(std::make_unique<ListRemoveAnalyzer<std::vector<int>>>("vector-remove"));
            this->addAnalyzer(std::make_unique<ListRemoveAnalyzer<std::list<int>>>("list-remove"));

            this->addAnalyzer(std::make_unique<ListAccessAnalyzer<ds::adt::ImplicitList<int>>>("implicit-list-access"));            // ako typ testovanej struktury prebera implicit list
            this->addAnalyzer(std::make_unique<ListAccessAnalyzer<ds::adt::SinglyLinkedList<int>>>("singly-linked-list-access"));   // ako typ testovanej struktury prebera singly linked list
        }
    };

    template<class List>
    ListAnalyzer<List>::ListAnalyzer(const std::string& name) :
        ComplexityAnalyzer<List>(name,
            // lambdu (kus kodu) vieme vlozit do typu std::function
            // hranate zatvorky obsahuju to, co chceme poslat do lambdy z jej okoliteho kontextu (napr. v okolitom konstruktore pozname this a vieme (aj potrebujeme) ho dostat do lambdy)
            [this](List& l, size_t s) {
                // lambda pozna iba to, co do nej posleme
                this->insertNElements(l, s);    // tu len zavolame funkciu s for cyklom, ale rovno by sme mohli napisat ten cyklus sem
            }),
        rngData_(std::random_device()()),
        rngIndex_(std::random_device()()),
        index_(0),
        data_(0)
    {
    }

    template<class List>
    void ListAnalyzer<List>::beforeOperation(List& structure)
    {
        std::uniform_int_distribution<size_t> indexDist(0, structure.size() - 1);
        index_ = indexDist(this->rngIndex_);
        data_ = rngData_();
    }

    template<class List>
    size_t ListAnalyzer<List>::getRandomIndex() const
    {
        return index_;
    }

    template<class List>
    int ListAnalyzer<List>::getRandomData() const
    {
        return data_;
    }

    template<class List>
    void ListAnalyzer<List>::insertNElements(List& list, size_t n)
    {
        for (size_t i = 0; i < n; ++i)
        {
            list.push_back(rngData_()); // ocakavame, ze list bude mat metodu push_back()
        }
    }

    template <class List>
    ListInsertAnalyzer<List>::ListInsertAnalyzer(const std::string& name) :
        ListAnalyzer<List>(name)
    {
    }

    template <class List>
    void ListInsertAnalyzer<List>::executeOperation(List& structure)
    {
        // prekryjeme execute operation pre InsertAnalyzer tak, aby vlozil data do listu
        structure.push_back(this->getRandomData());
    }

    template <class List>
    ListRemoveAnalyzer<List>::ListRemoveAnalyzer(const std::string& name) :
        ListAnalyzer<List>(name)
    {
    }

    template <class List>
    void ListRemoveAnalyzer<List>::executeOperation(List& structure)
    {
        // prekryjeme execute operation pre RemoveAnalyzer
        // structure.remove
        structure.pop_back();
    }
}
