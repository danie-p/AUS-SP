#pragma once

#define LIBDS_USE_HEAP_MONITOR

#include <libds/mm/memory_manager.h>
#include <libds/mm/memory_omanip.h>
#include <libds/constants.h>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <algorithm>
#include <iostream>

namespace ds::mm {

	template<typename BlockType>
	class CompactMemoryManager : public MemoryManager<BlockType> {
	public:
		CompactMemoryManager();
		CompactMemoryManager(size_t size);
		CompactMemoryManager(const CompactMemoryManager<BlockType>& other);
		~CompactMemoryManager() override;

		// vela sposobov alokovania a mazania (podla toho, ako to chceme spravit)
		BlockType* allocateMemory() override;
		BlockType* allocateMemoryAt(size_t index);												// pridel pamat na indexe
		void releaseMemory(BlockType* pointer) override;										// uvolni pamat tam, kde ukazuje pointer
		void releaseMemoryAt(size_t index);														// uvolni pamat na indexe
		void releaseMemory();

		size_t getCapacity() const;

		CompactMemoryManager<BlockType>& assign(const CompactMemoryManager<BlockType>& other);	// prirad don iny
		void changeCapacity(size_t newCapacity);												// zmen velkost na novu velkost
		void shrinkMemory();																	// zmensi pamat
		void clear();
		bool equals(const CompactMemoryManager<BlockType>& other) const;						// porovnaj s inym
		void* calculateAddress(const BlockType& data);											// vypocitaj adresu na zaklade bloku dat, vrat pointer
		size_t calculateIndex(const BlockType& data);											// vypocitaj poradie, index bloku dat
		BlockType& getBlockAt(size_t index);													// daj blok pamate na indexe
		void swap(size_t index1, size_t index2);												// vymen bloky dat na dvoch indexoch

		void print(std::ostream& os);

	private:
		size_t getAllocatedBlocksSize() const;													// daj velkost alokovanych blokov
		size_t getAllocatedCapacitySize() const;												// daj velkost alokovanej kapacity

	private:
		BlockType* base_;
		BlockType* end_;
		BlockType* limit_;

		static const size_t INIT_SIZE = 4;
	};

	template<typename BlockType>
    CompactMemoryManager<BlockType>::CompactMemoryManager() :
		CompactMemoryManager(INIT_SIZE)													// inicializuj SP s pociatocnou velkostou
	{
	}

	template<typename BlockType>
    CompactMemoryManager<BlockType>::CompactMemoryManager(size_t size) :
		base_(static_cast<BlockType*>(std::calloc(size, sizeof(BlockType)))),			// alokuj pole v pamati: pocet prvkov, dlzka kazdeho prvku (v bajtoch); vrati pointer na alokovany priestor
		end_(base_),
		limit_(base_ + size)
	{
	}

	template<typename BlockType>
    CompactMemoryManager<BlockType>::CompactMemoryManager(const CompactMemoryManager<BlockType>& other):
		CompactMemoryManager(other.getAllocatedBlockCount())							// inicializuj spravcu pamate inym spravcom pamate
	{
		this->assign(other);
	}

	template<typename BlockType>
    CompactMemoryManager<BlockType>::~CompactMemoryManager()	// destruktor
	{
		// zaisti, ze sa zavola destuktor nad kazdym objektom
		CompactMemoryManager<BlockType>::releaseMemory(base_);	// uvolni pamat (aby nam noestala stratena a nenastali memory leaky)
		std::free(base_);										// vrat pamat systemu

		// pomocka, ak to spadne => budeme vidiet, ze ukazujeme na nullptr
		base_ = nullptr;
		end_ = nullptr;
		limit_ = nullptr;
	}

	template<typename BlockType>
    BlockType* CompactMemoryManager<BlockType>::allocateMemory()
	{
		return this->allocateMemoryAt(end_ - base_);
	}

	template<typename BlockType>
    BlockType* CompactMemoryManager<BlockType>::allocateMemoryAt(size_t index)	// pridel pamat na indexe
	{
		if (end_ == limit_)		// ak doteraz "zaplnena" pamat konci na hranici alkovanej pamate (na limite)
		{
			this->changeCapacity(2 * this->getAllocatedBlockCount());		// zvacsi kapacitu na dvojnasobok doterajsieho poctu blokov (nasa expanzna strategia je 2x)
																			// mohli by sme expanznu strategiu zovseobecnit pomocou virtualnej metody / lambdy
		}

							// nech index je typu velkost rozdielu pointrov
		if (end_ - base_ > static_cast<std::ptrdiff_t>(index))	// ak sa snazime alokovat pamat na indexe, kde uz mame "zaplnenu" pamat (uz tam nieco je)
		{
			// vsetko potlacim o 1 doprava
			std::memmove(										// presun pamat (tu uz zaplnenu)
				base_ + index + 1,								// na index o jeden blok dalej
				base_ + index,									// z jej aktualneho indexu
				(end_ - base_ - index) * sizeof(BlockType)		// vieme, ze tato pamat je o velkosti blokov iducich od base + index az po end
			);
		}

		MemoryManager<BlockType>::allocatedBlockCount_++;
		end_++;

		// placement new nevie heap monitor spracovat a sledovat ako objekty vytvorene pomocou obycajneho new

#ifdef LIBDS_USE_HEAP_MONITOR
#pragma push_macro("new")
#undef new
#endif

		// vrati pointer na miesto, ktore sme prave alokovali
		return new (base_ + index) BlockType();		// placement new: vytvor TypBloku() na mieste urcenom bazou + indexom (presne urcime adresu v uz vyalokovanej pamati, kam vlozi blok)

#ifdef LIBDS_USE_HEAP_MONITOR
#pragma pop_macro("new")
#endif
	}

	template<typename BlockType>
    void CompactMemoryManager<BlockType>::releaseMemory(BlockType* pointer)		// uvolni pamat pocunc param pointrom az po koniec alokovanej pamate (end)
	{
		if (pointer < base_ || pointer > end_)
		{
			throw std::invalid_argument("Pointer out of allocated memory!");
		}

		BlockType* ptr = pointer;

		while (ptr != end_)
		{
			ptr->~BlockType();		// explicitne zavolam destruktor postupne nad kazdym blokom (finalizuj ptr)
			ptr++;
		}

		end_ = pointer;				// param pointer nastavim ako novy koniec (kedze za nim sa vsetka pamat uvolnila)
		MemoryManager<BlockType>::allocatedBlockCount_ = end_ - base_;	// novy pocet alokovanych blokov je rozdiel noveho konca od zaciatku
	}

	template<typename BlockType>
    void CompactMemoryManager<BlockType>::releaseMemoryAt(size_t index)		// uvolni pamat pocnuc param indexom
	{
		// ten isty postup ako pri alokacii robime pri dealokacii (len v opacnom poradi)

		this->getBlockAt(index).~BlockType();		// uvolni pamat na indexe (zavolaj destruktor nad blokom danym indexom)

		std::memmove(												// presun pamat naspat
			base_ + index,											// na param index
			base_ + index + 1,										// z bloku, ktory je o jeden dalej od indexu
			(end_ - base_ - index - 1) * sizeof(BlockType)			// pre velkost presuvanej pamate, ktora ide od base + index + 1 az po koniec
		);
		
		end_--;									// zniz koniec o jeden blok
		this->allocatedBlockCount_--;			// zniz pocet alokovanych blokov o 1
	}

	template<typename BlockType>
    void CompactMemoryManager<BlockType>::releaseMemory()
	{
		this->releaseMemory(end_ - 1);		// uvolni pamat pocnuc jednym blokom pred koncom az po koniec
	}

	template<typename BlockType>
    size_t CompactMemoryManager<BlockType>::getCapacity() const
	{
		return limit_ - base_;				// celkova kapacita alokovanej pamate (ktoru mame k dispozicii)
	}

	template<typename BlockType>
    CompactMemoryManager<BlockType>& CompactMemoryManager<BlockType>::assign(const CompactMemoryManager<BlockType>& other)
	{
		// nepouzivam *this != other (porovnavanie cez hodnoty) => lebo niekedy sa mozu objeky rovnat, aj ked sa nejedna o ten isty (nie su na rovnakej adrese)
		//	to by sa len pytalo: sme taki isti?

		// rychlejsie je porovnavanie cez adresy
		if (this != &other)		// sme ten isty?
		{
			this->releaseMemory(base_);
			// hovorime, ze to je atribut triedy MemoryManager
			MemoryManager<BlockType>::allocatedBlockCount_ = other.MemoryManager<BlockType>::allocatedBlockCount_;

			void* newBase = std::realloc(base_, other.getAllocatedCapacitySize());

			if (newBase == nullptr)
			{
				throw std::bad_alloc();
			}

			base_ = static_cast<BlockType*>(newBase);
			end_ = base_ + MemoryManager<BlockType>::allocatedBlockCount_;
			limit_ = base_ + (other.limit_ - other.base_);

			// potrebujeme si spravit kopiu objektov
			for (size_t i = 0; i < other.getAllocatedBlockCount(); i++)
			{
				placement_copy(base_ + i, *(other.base_ + i));	// placement copy pomocou copy construtora: potebujeme vytvorit kopiu
			}
		}

		return *this;
	}

	template<typename BlockType>
    void CompactMemoryManager<BlockType>::shrinkMemory()
	{
		size_t newCapacity = static_cast<size_t>(end_ - base_);

		if (newCapacity < CompactMemoryManager<BlockType>::INIT_SIZE)
		{
			newCapacity = CompactMemoryManager<BlockType>::INIT_SIZE;
		}

		this->changeCapacity(newCapacity);
	}

	// zmen kapacitu na novu velkost (mensia / vacsia ako aktualna)
	template<typename BlockType>
    void CompactMemoryManager<BlockType>::changeCapacity(size_t newCapacity)	// newCapacity definuje nasu expanznu strategiu
	{
		if (newCapacity == this->getCapacity()) {
			return;
		}

		if (newCapacity < this->getAllocatedBlockCount()) {
			this->releaseMemory(base_ + newCapacity);
		}

		void* newBase = std::realloc(base_, newCapacity * sizeof(BlockType));

		if (newBase == nullptr) {
			throw std::bad_alloc();
		}

		base_ = static_cast<BlockType*>(newBase);
		end_ = base_ + MemoryManager<BlockType>::allocatedBlockCount_;
		limit_ = base_ + newCapacity;
	}

	template<typename BlockType>
    void CompactMemoryManager<BlockType>::clear()
	{
		this->releaseMemory(base_);		// uvolni vsetku pamat od zaciatku az po koniec
	}

	template<typename BlockType>
    bool CompactMemoryManager<BlockType>::equals(const CompactMemoryManager<BlockType>& other) const
	{
		// 0 == std::memcmp(void*, void*, size_t) // spravi XOR dvoch blokov a ak su rovnake, vrati 0
		return this == &other ||
				(this->getAllocatedBlockCount() == other.getAllocatedBlockCount() &&
				std::memcmp(base_, other.base_, this->getAllocatedBlocksSize()) == 0);
	}

	// void* je akykolvek pointer (mozme donho vlozit cokolvek), neda sa s nim robit aritmetika
	// nemozeme spravit return len tak (lebo dany pointer je const)
	// nutne musime pouzit cyklus
	template<typename BlockType>
    void* CompactMemoryManager<BlockType>::calculateAddress(const BlockType& data)
	{
		BlockType* ptr = base_;

		while (ptr != end_ && ptr != &data)
		{
			ptr++;
		}

		return ptr == end_ ? nullptr : ptr;
	}

	template<typename BlockType>
    size_t CompactMemoryManager<BlockType>::calculateIndex(const BlockType& data)
	{
		return &data < end_ && &data >= base_ ? &data - base_ : INVALID_INDEX;
	}

	template<typename BlockType>
    BlockType& CompactMemoryManager<BlockType>::getBlockAt(size_t index)
	{
		return *(base_ + index);
	}

	template<typename BlockType>
    void CompactMemoryManager<BlockType>::swap(size_t index1, size_t index2)
	{
		std::swap(getBlockAt(index1), getBlockAt(index2));
	}

	// size = velkost v bajtoch
	// - vrati, o kolko typovych jednotiek su vzdialene pointre (napr kolko intov/osob/typov bloku sa zmesti)
	// + 1 pointer sa posunie o velkost jedneho prvku (NIE O 1 BAJT!!), napr o velkost intu
	// sizeof(TypBloku) vrati velkost bloku v bajtoch

	// velkost uz naukladanych blokov
	template<typename BlockType>
    size_t CompactMemoryManager<BlockType>::getAllocatedBlocksSize() const		// allocated blocks size = velkost uz zaplnenej pamate
	{
		return (end_ - base_) * sizeof(BlockType);
	}

	// velkost alokovanej kapacity
    template<typename BlockType>
    size_t CompactMemoryManager<BlockType>::getAllocatedCapacitySize() const	// allocated capacity size = velkost celkovej kapacity pamate, ktoru mame alokovanu a k dispozicii
	{
		return (limit_ - base_) * sizeof(BlockType);
	}

	template<typename BlockType>
    void CompactMemoryManager<BlockType>::print(std::ostream& os)
	{
		os << "first = " << base_ << std::endl;
		os << "last = " << end_ << std::endl;
		os << "limit = " << limit_ << std::endl;
		os << "block size = " << sizeof(BlockType) << "B" << std::endl;

		BlockType* ptr = base_;
		while (ptr != limit_) {
			std::cout << ptr;
			os << PtrPrintBin<BlockType>(ptr);

			if (ptr == base_) {
				os << "<- first";
			}
			else if (ptr == end_) {
				os << "<- last";
			}
			os << std::endl;
			ptr++;
		}

		os << limit_ << "|<- limit" << std::endl;
	}

}