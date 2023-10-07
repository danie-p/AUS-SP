#pragma once
#include <libds/amt/explicit_hierarchy.h>
#include "Unit.h"
#include "Algorithm.h"
#include "Sort.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

struct IteratorWrapper
{
    ds::amt::Hierarchy<ds::amt::MWEHBlock<Unit*>>::PreOrderHierarchyIterator moveIter(ds::amt::MultiWayExplicitHierarchy<Unit*>& hierarchy, ds::amt::MWEHBlock<Unit*>*& currBlock, int sonIndex = -2)
    {
        if (sonIndex != -2)
        {
            if (sonIndex == -1)
            {
                currBlock = hierarchy.accessParent(*currBlock);
            }
            else
            {
                currBlock = hierarchy.accessSon(*currBlock, sonIndex);
            }
        }
        ds::amt::Hierarchy<ds::amt::MWEHBlock<Unit*>>::PreOrderHierarchyIterator currIter(&hierarchy, currBlock);
        return currIter;
    }
};

template <typename ISType>
class HierarchySVK
{
public:
	HierarchySVK(ISType& ISregions, ISType& ISdistricts, ISType& ISmunicipalities);
	~HierarchySVK();
	void navigateHierarchy();

private:
    void loadUnits(ISType& ISregions, ISType& ISdistricts, ISType& ISmunicipalities);
    void beforeLoad(size_t fileNum, int& indexRegion, int& indexDistrict, int& indexMunicipality, std::string& region, std::string& district, size_t& n);
    void currDesc(size_t& i);
    void toSortOrNotToSort(ds::amt::ImplicitSequence<Unit*>& processed);
    void whereDoIGo(ds::amt::MultiWayExplicitHierarchy<Unit*>& hierarchy, ds::amt::MWEHBlock<Unit*>*& currBlock, size_t& i);

private:
	ds::amt::MultiWayExplicitHierarchy<Unit*> hierarchy;
    ds::amt::MWEHBlock<Unit*>* currBlock;
	Algorithm<Unit*, ds::amt::Hierarchy<ds::amt::MWEHBlock<Unit*>>::PreOrderHierarchyIterator> algorithm;
    Sort<Unit*> sort{};
};

template <typename ISType>
HierarchySVK<ISType>::HierarchySVK(ISType& ISregions, ISType& ISdistricts, ISType& ISmunicipalities)
{ 
    hierarchy.emplaceRoot().data_ = new Unit(1, "SK", "Slovenská republika", "Slovensko", "Slovensko", "SVK", 3102, "Slovenská republika");
	this->loadUnits(ISregions, ISdistricts, ISmunicipalities);
    currBlock = hierarchy.accessRoot();
}

template <typename ISType>
void HierarchySVK<ISType>::loadUnits(ISType& ISregions, ISType& ISdistricts, ISType& ISmunicipalities)
{
	size_t n;				// n = poradie syna u svojho otca
	int indexRegion;
	int indexDistrict;
	int indexMunicipality;
	std::string region;
	std::string district;

	this->beforeLoad(0, indexRegion, indexDistrict, indexMunicipality, region, district, n);
	for (auto unit : ISregions)
	{
		hierarchy.emplaceSon(*hierarchy.accessRoot(), n).data_ = unit;
		++n;
	}

	this->beforeLoad(1, indexRegion, indexDistrict, indexMunicipality, region, district, n);
	for (auto unit : ISdistricts)
	{
		std::string newRegion = unit->getCode().substr(3, 2);
		indexRegion = region == newRegion ? indexRegion : indexRegion + 1;
		indexDistrict = region == newRegion ? indexDistrict + 1 : 0;
		region = newRegion;
		hierarchy.emplaceSon((*hierarchy.accessSon(*hierarchy.accessRoot(), indexRegion)), indexDistrict).data_ = unit;
	}

	this->beforeLoad(2, indexRegion, indexDistrict, indexMunicipality, region, district, n);
	for (auto unit : ISmunicipalities)
	{
		std::string newRegion = unit->getCode().substr(3, 2);
		std::string newDistrict = unit->getCode().substr(3, 3);
		if (region != newRegion)								// ak prejde na novy kraj
		{
			++indexRegion;
			indexDistrict = 0;
			indexMunicipality = 0;
		}
		else
		{														// ak ostava vramci toho isteho kraja
			if (district != newDistrict)							// ak prejde na novy okres
			{
				++indexDistrict;
				indexMunicipality = 0;
			}
			else
			{														// ak ostava vramci toho isteho okresu
				++indexMunicipality;
			}
		}
		if (newRegion == "ZZ" && unit->getNote() != "")
		{
			indexDistrict = 1;
			indexMunicipality = 0;
		}
		region = newRegion;
		district = newDistrict;
		hierarchy.emplaceSon((*hierarchy.accessSon(*hierarchy.accessSon(*hierarchy.accessRoot(), indexRegion), indexDistrict)), indexMunicipality).data_ = unit;
	}
}

template <typename ISType>
void HierarchySVK<ISType>::beforeLoad(size_t fileNum, int& indexRegion, int& indexDistrict, int& indexMunicipality, std::string& region, std::string& district, size_t& n)
{
	if (fileNum == 0)
	{							// regions
		n = 0;
	}
	else if (fileNum == 1)
	{							// districts
		region = "10";
		indexRegion = 0;
		indexDistrict = -1;
	}
	else
	{							// municipalities
		region = "10";
		district = "101";
		indexRegion = 0;
		indexDistrict = 0;
		indexMunicipality = -1;
	}
}

template<typename ISType>
void HierarchySVK<ISType>::currDesc(size_t& i)
{
    std::cout << "Nachádzate sa vo vrchole " << (*currBlock->data_).getOfficialTitle() << "\n";
    if (hierarchy.isRoot(*currBlock))
    {
        std::cout << "\tAktuálny vrchol je koreò. Nemôžete sa posunú na otca.\n";
    }
    else
    {
        std::cout << "\tOtec aktuálneho vrchola:\n\t\t[0]\t" << (*currBlock->parent_->data_).getOfficialTitle() << "\n";
    }

    if (hierarchy.isLeaf(*currBlock))
    {
        std::cout << "\tAktuálny vrchol je list. Nemôžete sa posunú na syna.\n";
    }
    else
    {
        std::cout << "\tMnožina synov aktuálneho vrchola:\n";
        for (auto son : *currBlock->sons_)
        {
            std::cout << "\t\t[" << i << "]\t" << (*son->data_).getOfficialTitle() << "\n";
            ++i;
        }
    }
}

template<typename ISType>
void HierarchySVK<ISType>::navigateHierarchy()
{
    std::cout << "\n=== ÚROVEÒ 2 ===\n\n";
    ds::amt::Hierarchy<ds::amt::MWEHBlock<Unit*>>::PreOrderHierarchyIterator lastIter(&hierarchy, nullptr);
    size_t cont{ 1 };
    while (cont)
    {
        auto currIter = IteratorWrapper().moveIter(hierarchy, currBlock);
        ds::amt::ImplicitSequence<Unit*> processedUnits{};
        std::function<void(Unit* insertedUnit)> insert = [&](Unit* insertedUnit) { processedUnits.insertLast().data_ = insertedUnit; };
        
        size_t i{ 1 };
        this->currDesc(i);

        int errInput;
        do
        {
            errInput = 0;
            char input;
            InputCheck().checkInput(input, "Zadajte: presun na vrchol [v] | obsahuje [o] | zaèína na [z] | je typu [t] | má aspoò n materských škôl [m] | koniec [x]: ", "Nevhodný vstup. Zadajte znova: ",
                [&input]() -> bool { return input != 'x' && input != 'o' && input != 'z' && input != 't' && input != 'v' && input != 'm'; });
            
            try
            {
                switch (input)
                {
                case 'x':
                    cont = 0;
                    break;

                case 'o':
                case 'z':
                {
                    std::string searchedStr;
                    std::cout << "Zadajte h¾adaný reazec: ";
                    std::getline(std::cin, searchedStr);
                        
                    if (input == 'o')
                    {
                        std::function predicateContains = [&](Unit* testedUnit) -> bool { return (*testedUnit).containsStr(searchedStr); };
                        // prehliadka na containsStr
                        algorithm.findAndProcess(currIter, lastIter, predicateContains, insert);
                    }
                    else
                    {
                        std::function predicateStartsWith = [&](Unit* testedUnit) -> bool { return (*testedUnit).startsWithStr(searchedStr); };
                        // prehliadka na stratsWithStr
                        algorithm.findAndProcess(currIter, lastIter, predicateStartsWith, insert);
                    }

                    this->toSortOrNotToSort(processedUnits);
                    std::cout << '\n';

                    break;
                }
                case 't':
                {
                    size_t type;
                    InputCheck().checkInput(type, "Zadajte typ: kraj [1] | okres [2] | obec [3]: ", "Nevhodný zadaný typ. Zadajte znova: ",
                        [&type]() -> bool { return type != 1 && type != 2 && type != 3; });

                    std::function predicateHasType = [&](Unit* testedUnit) -> bool { return (*testedUnit).hasType(type); };
                    algorithm.findAndProcess(currIter, lastIter, predicateHasType, insert);

                    this->toSortOrNotToSort(processedUnits);
                    std::cout << '\n';

                    break;
                }
                case 'm':
                {
                    size_t kindergartenNum{};
                    std::string numInput;
                    std::cout << "Zadajte minimálny poèet materských škôl: ";
                    std::getline(std::cin, numInput);
                    kindergartenNum = std::stoi(numInput);

                    std::function<bool(Unit* testedUnit)> atLeastNKindergartens = [&](Unit* testedUnit) { return (*testedUnit).getKindergartenNum() >= kindergartenNum; };

                    algorithm.findAndProcess(currIter, lastIter,
                        atLeastNKindergartens,
                        insert);

                    this->toSortOrNotToSort(processedUnits);
                    std::cout << '\n';

                    break;
                }
                case 'v':
                {
                    this->whereDoIGo(hierarchy, currBlock, i);
                    std::cout << '\n';

                    break;
                }
                }
            } catch (std::exception& ex)
            {
                std::cout << ex.what();
                errInput = 1;
            }
        } while (errInput);
    }

    std::cout << "\n=== KONIEC ÚROVNE 2 ===\n\n";
}

template<typename ISType>
void HierarchySVK<ISType>::toSortOrNotToSort(ds::amt::ImplicitSequence<Unit*>& processed)
{
    char sortIn;
    InputCheck().checkInput(sortIn, "Zadajte: vypísa neutriedené [n] | utriedi [u]: ", "Nesprávny vstup. Zadajte znova: ",
        [&sortIn]() -> bool { return sortIn != 'n' && sortIn != 'u'; });

    if (sortIn == 'u')
    {
        sort.chooseSort(processed);
    }
    else
    {
        for (auto unit : processed)
        {
            std::cout << '\t' << *unit << '\n';
        }
    }
}

template<typename ISType>
void HierarchySVK<ISType>::whereDoIGo(ds::amt::MultiWayExplicitHierarchy<Unit*>& hierarchy, ds::amt::MWEHBlock<Unit*>*& currBlock, size_t& i)
{
    std::string nodeInput;
    std::cout << "Zadajte index vrchola: ";
    std::getline(std::cin, nodeInput);

    while (std::stoi(nodeInput) < 0 || (std::stoi(nodeInput) == 0 && hierarchy.isRoot(*currBlock)) || std::stoi(nodeInput) >= i)
    {
        std::cout << "Index mimo rozsahu. Zadajte znova: ";
        std::getline(std::cin, nodeInput);
    }

    if (std::stoi(nodeInput) == 0)
    {
        // presun na otca
        IteratorWrapper().moveIter(hierarchy, currBlock, -1);
    }
    else
    {
        // prasun na (std::stoi(input) - 1)-ho syna
        IteratorWrapper().moveIter(hierarchy, currBlock, std::stoi(nodeInput) - 1);
    }
}

template <typename ISType>
HierarchySVK<ISType>::~HierarchySVK()
{
    delete hierarchy.accessRoot()->data_;
    hierarchy.clear();	// pridane
}