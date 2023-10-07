#pragma once

#include <libds/amt/abstract_memory_type.h>
#include <libds/amt/sequence.h>

namespace ds::amt {

	// TODO
	// urobit analyzu zlozitosti vkladania a mazania z 1. miesta v implicitnej sekvencii

	template<typename DataType>
	class ImplicitSequence :
		public Sequence<MemoryBlock<DataType>>,
		public ImplicitAMS<DataType>
	{
	public:
		using BlockType = MemoryBlock<DataType>;

		ImplicitSequence();
		ImplicitSequence(size_t capacity, bool initBlocks);
		ImplicitSequence(const ImplicitSequence<DataType>& other);
		~ImplicitSequence() override;

		size_t calculateIndex(MemoryBlock<DataType>& block) override;

		BlockType* accessFirst() const override;
		BlockType* accessLast() const override;
		BlockType* access(size_t index) const override;
		BlockType* accessNext(const BlockType& block) const override;
		BlockType* accessPrevious(const BlockType& block) const override;

		BlockType& insertFirst() override;
		BlockType& insertLast() override;
		BlockType& insert(size_t index) override;
		BlockType& insertAfter(BlockType& block) override;
		BlockType& insertBefore(BlockType& block) override;

		void removeFirst() override;
		void removeLast() override;
		void remove(size_t index) override;
		void removeNext(const MemoryBlock<DataType>& block) override;
		void removePrevious(const MemoryBlock<DataType>& block) override;

		void reserveCapacity(size_t capacity);

		// ide o implicitnu strukturu
		virtual size_t indexOfNext(size_t currentIndex) const;		// index + 1; pripadne upravit ak prekryvame pre cyklicku
		virtual size_t indexOfPrevious(size_t currentIndex) const;	// index - 1

	public:
		// vnorena trieda
		class ImplicitSequenceIterator {
		public:
			ImplicitSequenceIterator(ImplicitSequence<DataType>* sequence, size_t index);
			ImplicitSequenceIterator(const ImplicitSequenceIterator& other);
			ImplicitSequenceIterator& operator++();							// vpred (PREfixovy zapis)
			ImplicitSequenceIterator operator++(int);						// POSTfixovy zapis inkrementovacieho operatora (vpred)
			bool operator==(const ImplicitSequenceIterator& other) const;	// je rovnaky
			bool operator!=(const ImplicitSequenceIterator& other) const;	// je rozny
			DataType& operator*();											// spristupni

			// pridane
			bool hasNext();
			bool hasPrevious();
			bool operator<(const ImplicitSequenceIterator& other) const;
			bool operator>(const ImplicitSequenceIterator& other) const;
			ImplicitSequenceIterator& operator--();							// vzad PREfix
			ImplicitSequenceIterator operator--(int);						// vzad POSTfix
			void operator+(int count);										// posun sa o int

		private:
			ImplicitSequence<DataType>* sequence_;
			size_t position_;
		};

		ImplicitSequenceIterator begin();
		ImplicitSequenceIterator end();

		using IteratorType = ImplicitSequenceIterator;
	};

	template<typename DataType>
	using IS = ImplicitSequence<DataType>;

	//----------

	template<typename DataType>
	class CyclicImplicitSequence : public IS<DataType>
	{
	public:
		CyclicImplicitSequence();
		CyclicImplicitSequence(size_t initSize, bool initBlocks);

		size_t indexOfNext(size_t currentIndex) const override;
		size_t indexOfPrevious(size_t currentIndex) const override;
	};

	template<typename DataType>
	using CIS = CyclicImplicitSequence<DataType>;

	//----------

	template<typename DataType>
	ImplicitSequence<DataType>::ImplicitSequence()
	{
	}

	template<typename DataType>
	ImplicitSequence<DataType>::ImplicitSequence(size_t initialSize, bool initBlocks):
		ImplicitAMS<DataType>(initialSize, initBlocks)
	{
	}

	template<typename DataType>
	ImplicitSequence<DataType>::ImplicitSequence(const ImplicitSequence<DataType>& other):
		ImplicitAMS<DataType>::ImplicitAbstractMemoryStructure(other)
	{
	}

	template<typename DataType>
	ImplicitSequence<DataType>::~ImplicitSequence()
	{
	}

	template<typename DataType>
	size_t ImplicitSequence<DataType>::calculateIndex(BlockType& block)			// vypocitaj index
	{
		// neponukne (kvoli sablonam) metodu, ale prelozit to vie
		return this->getMemoryManager()->calculateIndex(block);
	}

	template<typename DataType>
	MemoryBlock<DataType>* ImplicitSequence<DataType>::accessFirst() const		// spristupni prvy
	{
		// ak je v nasej sekvencii nejaky prvok (size > 0)
		// SKP nam da blok pamate na 0. indexe

		// size() je overridnuta v predkovi AbstractMemoryStructure a vracia pocet alokovanych blokov pamate :))
		return this->size() > 0 ? &(this->getMemoryManager()->getBlockAt(0)) : nullptr;
	}

	template<typename DataType>
	MemoryBlock<DataType>* ImplicitSequence<DataType>::accessLast() const		// spristupni posledny
	{
		// SKP da blok pamate na indexe velkost nasej sekvencie (pocet prvkov v nej) - 1
		return this->size() > 0 ? &(this->getMemoryManager()->getBlockAt(this->size() - 1)) : nullptr;
	}

	template<typename DataType>
	MemoryBlock<DataType>* ImplicitSequence<DataType>::access(size_t index) const	// spristupni na indexe
	{
		// vraciame ukazovatel (aby sme mohli pripadne vlozit nullptr, ak je to neplatne) => dame & PRED return hodnotu
		// size_t je nazaporny typ => nemusime kontrolovat >= 0
		return index < this->size() ? &(this->getMemoryManager()->getBlockAt(index)) : nullptr;
		// navratova hodnota z tohto selektora je BLOK PAMATE, ktory nam da spravca kompaktnej pamate
	}

	template<typename DataType>
	MemoryBlock<DataType>* ImplicitSequence<DataType>::accessNext(const MemoryBlock<DataType>& block) const		// spristupni nasledujuci
	{
		// namiesto typename vieme pouzit auto (jednoduchsie) - pri typename musime uviest nazov triedy a jej clena, ktory je tym datovym typom
		
		// SKP vypocita index poslaneho bloku; nad nim zavolame operaciu najdenia indexa nasledovnika (ako param prebera aktualny index)
		size_t index = this->indexOfNext(this->getMemoryManager()->calculateIndex(block));
		// ak sme dostali platny index (mensi ako size, teda pocet prvkov v sekvencii, teda index takeho prvku, ktory ma nasledovnika), vrati nasledujuci blok
		return index >= 0 && index < this->size() ? &(this->getMemoryManager()->getBlockAt(index)) : nullptr;
	}

	template<typename DataType>
	MemoryBlock<DataType>* ImplicitSequence<DataType>::accessPrevious(const MemoryBlock<DataType>& block) const	// spristupni predchadzajuci
	{
		size_t index = this->indexOfPrevious(this->getMemoryManager()->calculateIndex(block));
		// ak sme dostali platny index prvku, ktory moze mat predchodcu
		// vrati adresu predchadzajuceho bloku
		return index >= 0 && index < this->size() ? &(this->getMemoryManager()->getBlockAt(index)) : nullptr;
	}

	template<typename DataType>
	MemoryBlock<DataType>& ImplicitSequence<DataType>::insertFirst()			// vloz prvy
	{
		// SKP alokuje pamat pre prvy prvok (, pripadne posunie vsetky uz umiestnene prvky) a vlozi ho na nulty index
		// chceme vracat referenciu => dostaneme pointer, musime ho DEreferencovat (*)
		return *(this->getMemoryManager()->allocateMemoryAt(0));
	}

	template<typename DataType>
	MemoryBlock<DataType>& ImplicitSequence<DataType>::insertLast()				// vloz posledny
	{
		// SKP alokuje pamat za aktualnym koncom
		return *(this->getMemoryManager()->allocateMemory());
	}

	template<typename DataType>
	MemoryBlock<DataType>& ImplicitSequence<DataType>::insert(size_t index)		// vloz na indexe
	{
		// SKP alokuje pamat na mieste danom indexom 
		return *(this->getMemoryManager()->allocateMemoryAt(index));
		// ! vkladanie do stredu moze sposobit expanziu a pomale kopirovanie pamate !
	}

	template<typename DataType>
	MemoryBlock<DataType>& ImplicitSequence<DataType>::insertAfter(MemoryBlock<DataType>& block)	// vloz za
	{
		// SKP alokuje pamat na mieste za danym blokom
		// vrati dereferencovany pointer s adresou, kde sme vlozili blok
		//	dereferencovana hodnota je samotny vlozeny blok (referencia nan)
		return *(this->getMemoryManager()->allocateMemoryAt(this->getMemoryManager()->calculateIndex(block) + 1));
	}

	template<typename DataType>
	MemoryBlock<DataType>& ImplicitSequence<DataType>::insertBefore(MemoryBlock<DataType>& block)	// vloz pred
	{
		// SKP alokuje pamat na mieste daneho bloku a samotny blok posunie o jeden dalej
		// NEovplyvnuje blok PRED danym blokom!
		return *(this->getMemoryManager()->allocateMemoryAt(this->getMemoryManager()->calculateIndex(block)));
	}

	template<typename DataType>
	void ImplicitSequence<DataType>::removeFirst()				// zrus prvy
	{
		// SKP uvolni pamat na 0. indexe
		this->getMemoryManager()->releaseMemoryAt(0);
	}

	template<typename DataType>
	void ImplicitSequence<DataType>::removeLast()				// zrus posledny
	{
		// SKP uvolni pamat na poslednom indexe (v podstate size (koniec) - 1)
		this->getMemoryManager()->releaseMemory();
	}

	template<typename DataType>
	void ImplicitSequence<DataType>::remove(size_t index)		// zrus na indexe
	{
		this->getMemoryManager()->releaseMemoryAt(index);
		// ! mazanie zo stredu sposobi pomale kopirovanie pamate !
	}

	template<typename DataType>
	void ImplicitSequence<DataType>::removeNext(const MemoryBlock<DataType>& block)			// zrus nasledovnika
	{
		// vypocitame index (poradie) param bloku
		// najdeme index nasledovnika
		// SKP uvolni pamat na indexe nasledovnika
		this->getMemoryManager()->releaseMemoryAt(this->indexOfNext(this->getMemoryManager()->calculateIndex(block)));
	}

	template<typename DataType>
	void ImplicitSequence<DataType>::removePrevious(const MemoryBlock<DataType>& block)		// zrus predchodcu
	{
		// SKP uvolni pamat na indexe predchodcu daneho bloku
		this->getMemoryManager()->releaseMemoryAt(this->indexOfPrevious(this->getMemoryManager()->calculateIndex(block)));
	}

	template<typename DataType>
	void ImplicitSequence<DataType>::reserveCapacity(size_t capacity)					// rezervuj kapacitu
	{
		this->getMemoryManager()->changeCapacity(capacity);
	}

	template<typename DataType>
	size_t ImplicitSequence<DataType>::indexOfNext(size_t currentIndex) const			// index nasledovnika
	{
		return currentIndex >= this->size() - 1 ? -1 : currentIndex + 1;
	}

	template<typename DataType>
	size_t ImplicitSequence<DataType>::indexOfPrevious(size_t currentIndex) const		// index predchodcu
	{
		return currentIndex <= 0 ? -1 : currentIndex - 1;
	}

	// iteratory implicitnych sekvencii mozeme implementovat ako iteratory s nahodnym pristupom
	template <typename DataType>
	ImplicitSequence<DataType>::ImplicitSequenceIterator::ImplicitSequenceIterator
		(ImplicitSequence<DataType>* sequence, size_t index) :
		// vlastnosti iteratora:
			sequence_(sequence),	// referencia na strukturu (tu je to implicitna sekvencia)
			position_(index)		// referencia na aktualny prvok (vo forme indexu)
	{
	}

	template <typename DataType>
	ImplicitSequence<DataType>::ImplicitSequenceIterator::ImplicitSequenceIterator
		(const ImplicitSequenceIterator& other) :
			sequence_(other.sequence_),
			position_(other.position_)
	{
	}

	template <typename DataType>
	auto ImplicitSequence<DataType>::ImplicitSequenceIterator::operator++() -> ImplicitSequenceIterator&					// vpred prefix
	{
		// posun iteratora je realizovany prepoctom hodnoty aktualneho prvku (na indexe) => velmi efektivny O(1)
		position_++;
		return *(this);
	}

	template <typename DataType>								// sipkova syntax navratovej hodnoty: prekladac berie namespace ImplicitSequence
	auto ImplicitSequence<DataType>::ImplicitSequenceIterator::operator++(int) -> ImplicitSequenceIterator					// vpred postfix
	{
		ImplicitSequenceIterator tmp(*this);
		operator++();
		return tmp;
	}

	template <typename DataType>
	bool ImplicitSequence<DataType>::ImplicitSequenceIterator::operator==(const ImplicitSequenceIterator& other) const		// je rovnaky
	{
		return sequence_ == other.sequence_ && position_ == other.position_;
	}

	template <typename DataType>
	bool ImplicitSequence<DataType>::ImplicitSequenceIterator::operator!=(const ImplicitSequenceIterator& other) const		// je rozny
	{
		return !(*this == other);	// vyuzijeme negaciu operatora ==
	}

	template <typename DataType>
	DataType& ImplicitSequence<DataType>::ImplicitSequenceIterator::operator*()		// spristuni
	{
		// spristupni data sekvencie (struktury) na aktualnej pozicii
		return sequence_->access(position_)->data_;
	}

// zaciatok pridanych metod
	template<typename DataType>
	inline bool ImplicitSequence<DataType>::ImplicitSequenceIterator::hasNext()
	{
		return position_ < sequence_->size();
	}

	template<typename DataType>
	inline bool ImplicitSequence<DataType>::ImplicitSequenceIterator::hasPrevious()
	{
		return position_ > 0;
	}

	template<typename DataType>
	inline bool ImplicitSequence<DataType>::ImplicitSequenceIterator::operator<(const ImplicitSequenceIterator& other) const	// je pred
	{
		return sequence_ == other.sequence_ && position_ < other.position_;
	}

	template<typename DataType>
	inline bool ImplicitSequence<DataType>::ImplicitSequenceIterator::operator>(const ImplicitSequenceIterator& other) const	// je za
	{
		return sequence_ == other.sequence_ && position_ > other.position_;
	}

	template<typename DataType>
	auto ImplicitSequence<DataType>::ImplicitSequenceIterator::operator--()->ImplicitSequenceIterator&
	{
		position_--;
	}

	template<typename DataType>
	auto ImplicitSequence<DataType>::ImplicitSequenceIterator::operator--(int)->ImplicitSequenceIterator
	{
		ImplicitSequenceIterator tmp(*this);
		operator--();
		return tmp;
	}

	template<typename DataType>
	inline void ImplicitSequence<DataType>::ImplicitSequenceIterator::operator+(int count)
	{
		position_ = position_ + count;
	}
// koniec pridanych metod

	// index prveho PLATNEHO prvku je 0
	template <typename DataType>
	auto ImplicitSequence<DataType>::begin() -> ImplicitSequenceIterator		// prvy iterator
	{
		return ImplicitSequenceIterator(this, 0);
	}

	// index prveho NEPLATNEHO prvku je pocet (size)
	template <typename DataType>
	auto ImplicitSequence<DataType>::end() -> ImplicitSequenceIterator			// posledny iterator
	{
		return ImplicitSequenceIterator(this, ImplicitAbstractMemoryStructure<DataType>::size());
	}

	template<typename DataType>
	CyclicImplicitSequence<DataType>::CyclicImplicitSequence():
		IS<DataType>()
	{
	}

	template<typename DataType>
	CyclicImplicitSequence<DataType>::CyclicImplicitSequence(size_t initCapacity, bool initBlocks):
		IS<DataType>(initCapacity, initBlocks)
	{
	}

	template<typename DataType>
	size_t CyclicImplicitSequence<DataType>::indexOfNext(size_t currentIndex) const			// index nasledovnika
	{
		const size_t size = this->size();

		if (size != 0)
		{
			// ak je aktualny prvok koncom sekvencie => jeho nasledovnik je prvy prvok
			return currentIndex >= size - 1 ? 0 : currentIndex + 1;
		}
		return INVALID_INDEX;
	}

	template<typename DataType>
	size_t CyclicImplicitSequence<DataType>::indexOfPrevious(size_t currentIndex) const		// index predchodcu
	{
		const size_t size = this->size();
		if (size != 0)
		{
			// ak je aktualny prvok zaciatkom sekvencie => jeho predchodca je posledny prvok
			return currentIndex <= 0 ? size - 1 : currentIndex - 1;
		}
		return INVALID_INDEX;
	}

}