#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// analyza = ziskat a vytvorit .csv, vytvorit graf

namespace ds::utils
{
    /**
     *  @brief Analyzer with a name.
     */
    class Analyzer
    {
    public:
        explicit Analyzer(std::string name);
        virtual ~Analyzer() = default;
        virtual void analyze() = 0;                                // pure virtual metoda, je virtual a NEma telo
        virtual void setOutputDirectory(std::string path) = 0;
        virtual void setReplicationCount(size_t count) = 0;
        virtual void setStepSize(size_t size) = 0;
        virtual void setStepCount(size_t count) = 0;
        const std::string& getName() const;

    private:
        std::string name_;
    };

    /**
     *  @brief Container of analyzers.
     */
    class CompositeAnalyzer : public Analyzer
    {
    public:
        explicit CompositeAnalyzer(const std::string& name);
        void analyze() override;
        void setOutputDirectory(std::string path) override;
        void setReplicationCount(size_t count) override;
        void setStepSize(size_t size) override;
        void setStepCount(size_t count) override;
        void addAnalyzer(std::unique_ptr<Analyzer> analyzer);
        const std::vector<std::unique_ptr<Analyzer>>& getAnalyzers();

    private:
        std::vector<std::unique_ptr<Analyzer>> analyzers_;
    };

    /**
     *  @brief Specific analyzer.
     */
    class LeafAnalyzer : public Analyzer
    {
    public:
        explicit LeafAnalyzer(const std::string& name);
        void setOutputDirectory(std::string path) override;
        void setReplicationCount(size_t count) override;
        void setStepSize(size_t size) override;
        void setStepCount(size_t count) override;
        std::filesystem::path getOutputPath() const;
        bool wasSuccessful() const;

    protected:
        void resetSuccess();
        void setSuccess();
        size_t getReplicationCount() const;
        size_t getStepSize() const;
        size_t getStepCount() const;

    private:
        static const size_t DEFAULT_REPLICATION_COUNT = 100;
        static const size_t DEFAULT_STEP_SIZE = 10000;
        static const size_t DEFAULT_STEP_COUNT = 10;

    private:
        std::string outputDir_;
        size_t replicationCount_;
        size_t stepSize_;
        size_t stepCount_;
        bool wasSuccessful_;
    };

    /**
     *  @brief Universal analyzer of an operation of any structure.
     */
    template<class Structure>                               // definicia sablony
    class ComplexityAnalyzer : public LeafAnalyzer
    {
    public:
        // kazda struktura ma implicitny konstruktor
        void analyze() override;                            // zavola implicitny konstruktor (vytvori instanciu struktury) a zavola parametricuku analyze(Structure) metodu
        void analyze(Structure structurePrototype);         // aby sme nemuseli posielat stale vela parametrov

    protected:
        ComplexityAnalyzer(
            const std::string& name,
            std::function<void(Structure&, size_t)> insertN
        );

        // "hook"
        virtual void beforeOperation(Structure& structure) {};      // virtualna metoda s prazdnym telom => ak ju nepotrebujem zavolat, nic nespravi
                                                                    // mozeme ju ale overridnut a vykonat v nej to, co potrebujeme
        // abstraktna trieda (lebo ma tuto cisto abstraktnu metodu)
        virtual void executeOperation(Structure& structure) = 0;    // posielame strukturu ako REFERENCIU (NEvytvori sa kopia) = ukazovatel na existujuci objekt (je vytvoreny v pamati len raz)
                                                                    // (*s). = s-> => ak by sme to stale pouzivali len pointer
                                                                    // alebo posleme param referenciu a mozeme pristupovat priamo cez bodku s.
                                                                    // na zabranenie modifikacie objektu passovaneho ako referenciu vieme pouzit const
        virtual void afterOperation(Structure& structure) {};

    private:
        using duration_t = std::chrono::nanoseconds;                // premenujeme std::chrono::nanoseconds na duration_t a vsade inde v kode mozeme pouzivat duration_t
                                                                    // ak budeme chciet zmenit jednotky, staci ich zmenit tu

    private:
        void saveToCsvFile(const std::map<size_t, std::vector<duration_t>>& data) const;

    private:
        std::function<void(Structure&, size_t)> insertN_;
    };

    template <class Structure>
    ComplexityAnalyzer<Structure>::ComplexityAnalyzer(
        const std::string& name,
        std::function<void(Structure&, size_t)> insertN
    ) :
        LeafAnalyzer(name),
        insertN_(insertN)
    {
    }

    template <class Structure>
    void ComplexityAnalyzer<Structure>::analyze()
    {
        if constexpr (std::is_default_constructible_v<Structure>)
        {
            this->analyze(Structure());                 // zavolanie implicitneho konstruktora (vytvorenie instancie)
        }
        else
        {
            throw std::runtime_error("Structure is not default constructible. Use the other overload.");
        }
    }

    template<class Structure>
    void ComplexityAnalyzer<Structure>::analyze(Structure structurePrototype)   // ako parameter dostavame prototyp struktury => budeme vytvarat jeho kopiu
    {
        std::map<size_t, std::vector<duration_t>> samples;

        for (int replication = 0; replication < this->getReplicationCount(); replication++)
        {
            // vytvor prazdnu strukturu
            Structure structure(structurePrototype);                    // vytvori sa instancia, ktora ma typ Structure (sablona)

            for (int step = 0; step < this->getStepCount(); step++)     // spusti replikaciu
            {
                size_t expectedSize = this->getStepSize() * (step + 1); // aktualny step * stepSize = aktualny pocet prvkov
                size_t neededCount = expectedSize - structure.size();   // nad sablonou structure by sme mali vediet zavolat size() (nad konkretnym objektom typu sablony)
                                                                        // size() vrati pocet prvkov struktury => aj my budeme pocet prvkov nasich vlastnych struktur nazvyat size()

                // vloz stepSize prvkov
                // insertN je atribut s funkciou
                this->insertN_(structure, neededCount);

                this->beforeOperation(structure);   // ak potrebujeme nieco vykonat este pred operaciou a nechceme, aby to ovplyvnilo merany cas operacie
                
                // spusti meranie casu
                // std::chrono je namespace
                auto timeStart = std::chrono::high_resolution_clock::now(); // trieda high_resolution_clock ma metodu now() = je to metoda TRIEDY (nie instancie)
                                                                            // auto => kompilator dedukuje navratovy typ metody now()
                // vykonaj operaciu
                this->executeOperation(structure);

                // zastav meranie casu
                auto timeEnd = std::chrono::high_resolution_clock::now();
                
                this->afterOperation(structure);

                duration_t duration = std::chrono::duration_cast<duration_t>(timeEnd - timeStart);  // pretypujeme vysledny rozdiel casov

                // uloz par (pocetPrvkov, cas)
                samples[expectedSize].push_back(duration);  // ku "klucu" danemu velkostou aktualneho stepu vlozime trvanie operacie
                                                            // do mapy vlozime dvojicu (pocet prvkov, cas)
            }
        }
        // uloz namerane casy
        this->saveToCsvFile(samples);
    }

    template <class Structure>                     // const aby sme si nezmenili mapu, ktoru posielame ako referenciu
    void ComplexityAnalyzer<Structure>::saveToCsvFile(const std::map<size_t, std::vector<duration_t>>& data) const
    {
        constexpr char Separator = ';';
        auto path = this->getOutputPath();
        std::ofstream ost(path);

        if (!ost.is_open())
        {
            throw std::runtime_error("Failed to open output file.");
        }

        const size_t rowCount = data.begin()->second.size();
        const size_t lastSize = (--data.end())->first;

        for (const auto& [size, durations] : data)
        {
            ost << size << (size != lastSize ? Separator : '\n');
        }

        for (int i = 0; i < rowCount; ++i)
        {
            for (const auto& [size, durations] : data)
            {
                ost << data.at(size)[i].count() << (size != lastSize ? Separator : '\n');
            }
        }
    }
}
