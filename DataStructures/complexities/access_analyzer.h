#pragma once

#include <complexities/complexity_analyzer.h>
#include <random>

namespace ds::utils
{
	template<typename Structure>
	class AccessAnalyzer : public ComplexityAnalyzer<Structure>
	{
	public:
		AccessAnalyzer
		(
			const std::string& name,
			std::function<void(Structure&, size_t)> insertN
		) :
			ComplexityAnalyzer(name, insertN)	// volanie konstruktora
			{
			}

		void executeOperation(Structure& structure) override
		{
			structure.access(randomIndex_);
		}

		// nechceme, aby do merania zaratavalo aj generovanie nah. indexu
		// vygenerujeme pred spustenim merania, ulozime do atributu
		void beforeOperation(Structure& structure) override
		{
			// nepouzivat rand() !!
			// structure.access(std::rand() % structure.size());

			std::uniform_int_distribution<size_t> distribution(0, structure.size() - 1);		// rovnomerne rozdelenie <a,b> = <0, size() - 1>
			randomIndex_ = distribution(generator_);		// tento vyraz vrati nahodne cislo zo zvoleneho rozdelenia
		};

	private:
		size_t randomIndex_;
		std::default_random_engine generator_;		// vrati nahodne cislo, ktore este potrebujeme napasovat na zvolene rozdelenie
			// dame si ho ako atribut, pretoze je implementovany tak, aby postupne vytvaral sekvenciu nahodnych cisel a navazoval na ne,
			// takze ho nechceme zakazdym nanovo vytvarat ako lokalku
	};

	template<typename List>
	class ListAccessAnalyzer : public AccessAnalyzer<List>	// chceme posielat ImplicitList aj SinglyLinkedList => sablona
	{
	public:
		ListAccessAnalyzer(const std::string& name) :
			// za dvojbodkou konstruktora piseme volanie konstruktora predka
			AccessAnalyzer(name, [](List& list, size_t n)	// param insertN posielame ako lambda funkciu
				{
					for (size_t i = 0; i < n; i++)
					{
						// n-krat vlozime lubovolny prvok (nezalezi na jeho udajoch)
						list.insertLast(67);
					}
				})
		{

		}
	};
}