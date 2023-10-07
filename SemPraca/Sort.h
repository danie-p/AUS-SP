#pragma once
#include <libds/adt/sorts.h>
#include <locale>
#include "Unit.h"

struct CompareAlphabetical
{
	bool operator()(Unit* unit1, Unit* unit2) const
	{
		return std::locale("Slovak_Slovakia.1250")(unit1->getOfficialTitle(), unit2->getOfficialTitle());
	}
};

struct CompareVowelsCount
{
	bool operator()(Unit* unit1, Unit* unit2) const
	{
		return unit1->vowelsCount() < unit2->vowelsCount();
	}
};

struct CompareHierarchically
{
	bool operator()(Unit* unit1, Unit* unit2) const
	{
		if (unit1->getCode().substr(3, 2) == unit2->getCode().substr(3, 2))
		{
			return unit1->getCode().substr(5, 1)[0] < unit2->getCode().substr(5, 1)[0];
		}
		return unit1->district() < unit2->district();
	}
};

template <typename T>
class Sort
{
public:
	Sort();
	~Sort();
	void sort(ds::amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare);
	void chooseSort(ds::amt::ImplicitSequence<T>& is);

private:
	ds::adt::MergeSort<T>* mergeSort;
};

template <typename T>
Sort<T>::Sort()
{
	mergeSort = new ds::adt::MergeSort<T>;
}

template <typename T>
Sort<T>::~Sort()
{
	delete mergeSort;
}

template<typename T>
void Sort<T>::sort(ds::amt::ImplicitSequence<T>& is, std::function<bool(const T&, const T&)> compare)
{
	mergeSort->sort(is, compare);
}

template<typename T>
void Sort<T>::chooseSort(ds::amt::ImplicitSequence<T>& is)
{
	std::cout << "\n=== ÚROVEÒ 4 ===\n\n";
	char cmpIn;
	InputCheck().checkInput(cmpIn, "Utriedi: v abecednom poradí [a] | pod¾a poètu samohlások [s]: ", "Nesprávny vstup. Zadajte znova: ",
		[&cmpIn]() -> bool { return cmpIn != 'a' && cmpIn != 's'; });

	if (cmpIn == 'a')
	{
		this->sort(is, CompareAlphabetical());
		for (auto unit : is)
		{
			std::cout << '\t' << *unit << " | " << unit->getOfficialTitle() << '\n';
		}
	}
	else
	{
		this->sort(is, CompareVowelsCount());
		for (auto unit : is)
		{
			std::cout << '\t' << *unit << " | " << unit->vowelsCount() << '\n';
		}
	}

	std::cout << "\n=== KONIEC ÚROVNE 4 ===\n\n";
}
