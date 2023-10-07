#pragma once
#include <libds/amt/implicit_sequence.h>
#include "Unit.h"
#include "DataIO.h"
#include "Algorithm.h"

class ImplicitSequences
{
public:
    ImplicitSequences();
    ~ImplicitSequences();
    void findAndProcessUnit();
    ds::amt::ImplicitSequence<Unit*>& getRegions() { return regions; };
    ds::amt::ImplicitSequence<Unit*>& getDistricts() { return districts; };
    ds::amt::ImplicitSequence<Unit*>& getMunicipalities() { return municipalities; };

private:
    ds::amt::ImplicitSequence<Unit*> regions{};
    ds::amt::ImplicitSequence<Unit*> districts{};
    ds::amt::ImplicitSequence<Unit*> municipalities{};
};

ImplicitSequences::ImplicitSequences()
{
    try
    {
        DataIO<Unit>().dataInput("kraje", [&](Unit* unit) { regions.insertLast().data_ = unit; });
        DataIO<Unit>().dataInput("okresy", [&](Unit* unit) { districts.insertLast().data_ = unit; });
        DataIO<Unit>().dataInput("obce", [&](Unit* unit) { municipalities.insertLast().data_ = unit; });
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what();
    }
}

ImplicitSequences::~ImplicitSequences()
{
    for (auto unit : regions)
    {
        delete unit;
    }

    for (auto unit : districts)
    {
        delete unit;
    }

    for (auto unit : municipalities)
    {
        delete unit;
    }
}

void ImplicitSequences::findAndProcessUnit()
{
    std::cout << "\n=== ÚROVEÒ 1 ===\n\n";
    size_t cont = 1;
    while (cont)
    {
        // vyber typu jednotky
        size_t unitType{};
        InputCheck().checkInput(unitType, "Typ územnej jednotky: kraje [1] | okresy [2] | obce [3]: ", "Nesprávny vstup. Zadajte znova: ", [&unitType]() -> bool { return unitType != 1 && unitType != 2 && unitType != 3; });

        ds::amt::ImplicitSequence<Unit*>& data{ unitType == 1 ? regions : unitType == 2 ? districts : municipalities };

        // vyber operacie
        char operation{};
        InputCheck().checkInput(operation, "Operácia: zaèína [z] | obsahuje [o] | má aspoò n materských škôl [m]: ", "Nesprávny vstup. Zadajte znova: ", [&operation]() -> bool { return operation != 'z' && operation != 'o' && operation != 'm'; });

        std::string searchedStr{};
        size_t kindergartenNum{};

        ds::amt::ImplicitSequence<Unit*> processedData{};
        std::function<void(Unit* insertedUnit)> insert = [&](Unit* insertedUnit) { processedData.insertLast().data_ = insertedUnit; };

        Algorithm<Unit*, ds::amt::ImplicitSequence<Unit*>::ImplicitSequenceIterator> a;

        if (operation == 'z' || operation == 'o')
        {
            // vyber substringu
            std::cout << "Zadajte h¾adaný substring: ";
            std::getline(std::cin, searchedStr);

            std::function<bool(Unit* testedUnit)> containsStr = [&](Unit* testedUnit) { return (*testedUnit).containsStr(searchedStr); };
            std::function<bool(Unit* testedUnit)> startsWithStr = [&](Unit* testedUnit) { return (*testedUnit).startsWithStr(searchedStr); };

            if (operation == 'z')
            {
                a.findAndProcess(data.begin(), data.end(),
                    startsWithStr,  // predicate
                    insert);        // process
            }
            else
            {
                a.findAndProcess(data.begin(), data.end(),
                    containsStr,    // predicate
                    insert);        // process
            }
        }
        else
        {
            std::string numInput;
            std::cout << "Zadajte minimálny poèet materských škôl: ";
            std::getline(std::cin, numInput);
            kindergartenNum = std::stoi(numInput);

            std::function<bool(Unit* testedUnit)> atLeastNKindergartens = [&](Unit* testedUnit) { return (*testedUnit).getKindergartenNum() >= kindergartenNum; };

            a.findAndProcess(data.begin(), data.end(),
                atLeastNKindergartens,
                insert);
        }

        std::cout << "Nájdené dáta:\n";
        for (auto unit : processedData)
        {
            std::cout << "\t" << *unit << "\n";
        }

        // vyber pokracovania
        InputCheck().checkInput(cont, "Pokraèova? [0/1]: ", "Nesprávny vstup. Zadajte znova: ", [&cont]() -> bool { return cont != 0 && cont != 1; });
        std::cout << '\n';
    }

    std::cout << "=== KONIEC ÚROVNE 1 ===\n\n";
}