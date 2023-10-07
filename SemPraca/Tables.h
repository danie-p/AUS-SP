#pragma once
#include <libds/adt/table.h>
#include <libds/adt/list.h>
#include "Unit.h"

template <typename DataType, typename ISType>
class Tables
{
private:
	ds::adt::ModifiedTreap<std::string, ds::adt::ImplicitList<DataType*>, DataType*> tabRegions{};
	ds::adt::ModifiedTreap<std::string, ds::adt::ImplicitList<DataType*>, DataType*> tabDistricts{};
	ds::adt::ModifiedTreap<std::string, ds::adt::ImplicitList<DataType*>, DataType*> tabMunicipalities{};

public:
	Tables(ISType& ISregions, ISType& ISdistricts, ISType& ISmunicipalities);
	~Tables();
	void displayUnitInfo();
	//void loadKindergartenNums();
};

template <typename DataType, typename ISType>
Tables<DataType, ISType>::Tables(ISType& ISregions, ISType& ISdistricts, ISType& ISmunicipalities)
{
	for (DataType* dataUnit : ISregions)
	{
		tabRegions.insert(dataUnit->getOfficialTitle(), dataUnit);
	}

	for (DataType* dataUnit : ISdistricts)
	{
		tabDistricts.insert(dataUnit->getOfficialTitle(), dataUnit);
	}

	for (DataType* dataUnit : ISmunicipalities)
	{
		tabMunicipalities.insert(dataUnit->getOfficialTitle(), dataUnit);
	}

	//this->loadKindergartenNums();
}

template <typename DataType, typename ISType>
Tables<DataType, ISType>::~Tables()
{
	for (auto data : tabRegions)
	{
		delete data.data_;
	}

	for (auto data : tabDistricts)
	{
		delete data.data_;
	}

	for (auto data : tabMunicipalities)
	{
		delete data.data_;
	}

	tabRegions.clear();
	tabDistricts.clear();
	tabMunicipalities.clear();
}

template<typename DataType, typename ISType>
void Tables<DataType, ISType>::displayUnitInfo()
{
	std::cout << "\n=== ÚROVEÒ 3 ===\n\n";
	size_t cont = 1;
	while (cont)
	{
		
		size_t unitType{};
		InputCheck().checkInput(unitType, "Typ územnej jednotky: kraje [1] | okresy [2] | obce [3]: ", "Nesprávny vstup. Zadajte znova: ", [&unitType]() -> bool { return unitType != 1 && unitType != 2 && unitType != 3; });

		std::string nazov{};
		std::cout << "Názov jednotky: ";
		std::getline(std::cin, nazov);
		
		try
		{
			switch (unitType)
			{
			case 1:
				for (auto dataUnit : *tabRegions.find(nazov))
				{
					std::cout << '\t' << *dataUnit << '\n';
				}
				break;
			case 2:
				for (auto dataUnit : *tabDistricts.find(nazov))
				{
					std::cout << '\t' << *dataUnit << '\n';
				}
				break;
			case 3:
				for (auto dataUnit : *tabMunicipalities.find(nazov))
				{
					std::cout << '\t' << *dataUnit << '\n';
				}
				break;
			}
		}
		catch (const std::exception& err)
		{
			std::cout << err.what() << "\n\n";
		}

		InputCheck().checkInput(cont, "Pokraèova? [0/1]: ", "Nesprávny vstup. Zadajte znova: ", [&cont]() -> bool { return cont != 0 && cont != 1; });

		std::cout << '\n';
	}

	std::cout << "=== KONIEC ÚROVNE 3 ===\n\n";
}

//template<typename DataType, typename ISType>
//void Tables<DataType, ISType>::loadKindergartenNums()
//{
//	std::ifstream input{ "kindergarten_mun.csv" };
//	std::string line{};
//
//	if (!input)
//	{
//		throw std::runtime_error("Chyba pri èítaní zo súboru!");
//	}
//
//	while (std::getline(input, line))
//	{
//		std::istringstream ssline(line);
//		std::string word{};
//		std::vector<std::string> words{};
//
//		while (std::getline(ssline, word, ';'))
//		{
//			words.push_back(word);
//		}
//
//		std::stringstream ssnum(words[1]);
//		size_t num{};
//		ssnum >> num;
//
//		try
//		{
//			ds::adt::ImplicitList<Unit*>* single = this->tabMunicipalities.find(words[0]);
//			(*single).accessLast()->setAltTitle(words[0]);
//			(*single).accessLast()->setKindergartenNum(num);
//		}
//		catch (ds::adt::structure_error&)
//		{
//			std::istringstream ssalt(words[0]);
//			std::string altTitle{};
//			std::getline(ssalt, altTitle, ',');
//
//			ds::adt::ImplicitList<Unit*>* synonyms = this->tabMunicipalities.find(altTitle);
//
//			size_t ithSynonym{ 0 };
//			while (!(*synonyms).access(ithSynonym)->getAltTitle().empty())
//			{
//				++ithSynonym;
//			}
//
//			(*synonyms).access(ithSynonym)->setAltTitle(words[0]);
//			(*synonyms).access(ithSynonym)->setKindergartenNum(num);
//		}
//	}
//}
