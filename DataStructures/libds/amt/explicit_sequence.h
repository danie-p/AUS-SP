#pragma once

#include <libds/amt/abstract_memory_type.h>
#include <libds/amt/sequence.h>
#include <functional>
#include <iterator>

// predok AMT (abstract memory type) ma atribut memoryManager_

namespace ds::amt {

	template<typename BlockType>
	class ExplicitSequence :
		public Sequence<BlockType>,
		public ExplicitAMS<BlockType>
	{
	public:
		ExplicitSequence();
		ExplicitSequence(const ExplicitSequence& other);
		~ExplicitSequence() override;

		AMT& assign(const AMT& other) override;
		void clear() override;
		bool equals(const AMT& other) override;

		size_t calculateIndex(BlockType& data) override;

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
		void removeNext(const BlockType& block) override;
		void removePrevious(const BlockType& block) override;

	protected:
		// vytvaranie / rusenie vztahov medzi blokmi
		virtual void connectBlocks(BlockType* previous, BlockType* next);
		virtual void disconnectBlock(BlockType* block);

		BlockType* first_;
		BlockType* last_;

	public:
		using DataType = decltype(BlockType().data_);

		class ExplicitSequenceIterator {
		public:
            explicit ExplicitSequenceIterator(BlockType* position);
			ExplicitSequenceIterator(const ExplicitSequenceIterator& other);
			ExplicitSequenceIterator& operator++();
			ExplicitSequenceIterator operator++(int);
			bool operator==(const ExplicitSequenceIterator& other) const;
			bool operator!=(const ExplicitSequenceIterator& other) const;
			DataType& operator*();

		private:
			BlockType* position_;
		};

		ExplicitSequenceIterator begin();
		ExplicitSequenceIterator end();

		using IteratorType = ExplicitSequenceIterator;
	};

	template<typename BlockType>
	using ES = ExplicitSequence<BlockType>;

	//----------

	template<typename DataType>
	struct SinglyLinkedSequenceBlock :							// BLOK JEDNOSTRANNE ZRETAZENEJ SEKVENCIE
		public MemoryBlock<DataType>
	{

		SinglyLinkedSequenceBlock() : next_(nullptr) {}
		~SinglyLinkedSequenceBlock() { next_ = nullptr; }

		SinglyLinkedSequenceBlock<DataType>* next_;
	};
	template<typename DataType>
	using SLSBlock = SinglyLinkedSequenceBlock<DataType>;

	template<typename DataType>
	class SinglyLinkedSequence :
		public ES<SLSBlock<DataType>>
	{
	public:
		using BlockType = SinglyLinkedSequenceBlock<DataType>;
	};

	template<typename DataType>
	using SinglyLS = SinglyLinkedSequence<DataType>;

	template<typename DataType>
	class SinglyCyclicLinkedSequence :							// JEDNOSTRANNE ZRETAZENA SEKVENCIA
		public SinglyLS<DataType>
	{
	};

	template<typename DataType>
	using SinglyCLS = SinglyCyclicLinkedSequence<DataType>;

	//----------

	template<typename DataType>
	struct DoublyLinkedSequenceBlock :							// BLOK OBOJSTRANNE ZRETAZENEJ SEKVENCIE
		public SLSBlock<DataType>								// dedi blok jednostranne zretazeneho predka
	{

		DoublyLinkedSequenceBlock() : previous_(nullptr) {}
		~DoublyLinkedSequenceBlock() { previous_ = nullptr; }

		DoublyLinkedSequenceBlock<DataType>* previous_;
	};

	template<typename DataType>
	using DLSBlock = DoublyLinkedSequenceBlock<DataType>;

	template<typename DataType>
	class DoublyLinkedSequence :								// OBOJSTRANNE ZRETAZENA SEKVENCIA
		public ES<DLSBlock<DataType>>
	{
	public:
		using BlockType = DLSBlock<DataType>;

		BlockType* access(size_t index) const override;
		BlockType* accessPrevious(const BlockType& block) const override;

		void removeFirst() override;

	protected:
		void connectBlocks(BlockType* previous, BlockType* next) override;	// musime upratat pointer na predchodcu
		void disconnectBlock(BlockType* block) override;
	};

    template<typename DataType>
	using DoublyLS = DoublyLinkedSequence<DataType>;

	template<typename DataType>
	class DoublyCyclicLinkedSequence :							// OBOJSTRANNE CYKLICKY ZRETAZENA SEKVENCIA
		public DoublyLS<DataType>
	{
	};

    template<typename DataType>
	using DoublyCLS = DoublyCyclicLinkedSequence<DataType>;

	// ES pri vzniku nic NEalokuje
	template<typename BlockType>
    ExplicitSequence<BlockType>::ExplicitSequence() :
		first_(nullptr),
		last_(nullptr)
	{
	}

	template<typename BlockType>
    ExplicitSequence<BlockType>::ExplicitSequence(const ExplicitSequence& other) :
	    ExplicitSequence()
	{
		assign(other);
	}

	template<typename BlockType>
    ExplicitSequence<BlockType>::~ExplicitSequence()
	{
		clear();
	}

	template<typename BlockType>
    AMT& ExplicitSequence<BlockType>::assign(const AMT& other)
	{
		if (this != &other)
		{
			clear();

			const ExplicitSequence<BlockType>& otherExplicitSequence = dynamic_cast<const ExplicitSequence<BlockType>&>(other);
			otherExplicitSequence.processAllBlocksForward([&](const BlockType* b)
				{
					this->insertLast().data_ = b->data_;
				});
		}

		return *this;
	}

	// vsetko nastavi na nullptr
	// ! treba si zachovat info o nasledovnikovi predtym nez sa vymaze aktualny blok !
	template<typename BlockType>
    void ExplicitSequence<BlockType>::clear()
	{
		last_ = first_;
		while (first_ != nullptr)
		{
			first_ = this->accessNext(*first_);			// na firste sa bude uchovavat nasledovnik aktualne mazaneho
			this->memoryManager_->releaseMemory(last_);			// pouzivame memory managera (ten zabezpecuje uvolnenie pamate)
			last_ = first_;								// tam, kde ukazuje last, sa bude nastavovat nullptr
		}

		// vo first a last budu na konci nullptr
	}

	template<typename BlockType>
    bool ExplicitSequence<BlockType>::equals(const AMT& other)
	{
		// zistenie, ci sa typy rovnaju: pomocou dynamic castu
		// ak sa neda pretypovat => dynamic cast vrati nullptr
		// ak sa da iny pretypovat na ES => dyn cast vrati ten pretypovany

		if (this == &other)
		{
			return true;
		}

		if (this->size() != other.size())
		{
			return false;
		}

		// ak chceme porovnavat dynamic cast s nullpointrom, treba pouzivat pri castovani pointer!
		const ExplicitSequence<BlockType>* otherExplicitSequence = dynamic_cast<const ExplicitSequence<BlockType>*>(&other);
		if (otherExplicitSequence == nullptr)
		{
			return false;
		}

		BlockType* myCurrent = first_;
		BlockType* otherCurrent = otherExplicitSequence->first_;

		while (myCurrent != nullptr)
		{
			if (!(myCurrent->data_ == otherCurrent->data_))
			{
				return false;
			}
			else {
				myCurrent = this->accessNext(*myCurrent);
				otherCurrent = otherExplicitSequence->accessNext(*otherCurrent);
			}
		}

		return true;
	}

	template<typename BlockType>
    size_t ExplicitSequence<BlockType>::calculateIndex(BlockType& data)
	{
		// najdi blok s vlastnostou (najdi blok, ktory sa zhoduje s param blokom data a ked ho najdes, tak budes poznat jeho index a vratis ho)
		
		size_t index = 0;	// pocitadlo

		// param funkcie findBlockWithProperty ocakava FUNKCIU (predikat vracajuci bool) => my mu tam posleme lambdu
		// hranate: okolity kontext (captures) - ako kopia/referencia => my chceme referenciu, aby lambda aj naozaj upravila konkretny existujuci index
			// ak chceme poznat vsetko okolo ako referenciu: [&]
				// v nasom pripade sa poslu data& (param), index& (lokalka), this
			// ak chceme poznat vsetko okolo ako kopiu: [=]
		// gulate: vstupny typ parametra (OBJEKT, ktory testujeme, ci splna danu vlastnost (tu je testovana vlastnost "zhoda s testedBlockom",
				// kde testedBlock pochadza z metody findBlockWithProperty a porovnava sa voci bloku, ktoreho index hladame tu v tejto metode))
		auto foundBlock = this->findBlockWithProperty([&](BlockType* testedBlock) -> bool
			{
				index++;
				return testedBlock == &data;	// je nas hladany blok rovnaky s aktualne testovanym?
			});

		return foundBlock != nullptr ? index -1 : INVALID_INDEX;
	}

	template<typename BlockType>
    BlockType* ExplicitSequence<BlockType>::accessFirst() const
	{
		return first_;
	}

	template<typename BlockType>
    BlockType* ExplicitSequence<BlockType>::accessLast() const
	{
		return last_;
	}

	template<typename BlockType>
    BlockType* ExplicitSequence<BlockType>::access(size_t index) const		// spristupni blok na indexe
	{
		// SEKVENCNY PRISTUP: musime prejst vsetkymi prvkami (CYKLUS!) pred hladanym
		// ES je struktura so sekvencnym pristupom k prvkom
		// priamo vieme pristupovat len k vybranym prvkom (first, last)

		BlockType* result = nullptr;

		if (index >= 0 && index < this->size())
		{
			result = first_;
			for (size_t i = 0; i < index; i++)
			{
				result = this->accessNext(*result);		// postupne prechadzame od aktualneho bloku k jeho nasledovnikovi,
														// az pokym tento prechod neyvkoname index-krat =>
														// dosli sme az ku bloku na danom indexe a mozeme ho vratit
			}
		}
	
		return result;

	}

	template<typename BlockType>
    BlockType* ExplicitSequence<BlockType>::accessNext(const BlockType& block) const
	{
		// block ma v sebe ulozenu referenciu na nasledovnika (to je ten jeho explicitny vztah)
		return static_cast<BlockType*>(block.next_);
	}

	template<typename BlockType>
    BlockType* ExplicitSequence<BlockType>::accessPrevious(const BlockType& block) const
	{
		// hladame taky blok, ktoreho nasledovnik je param block (teda hladame predchodcu tohto param blocku)
		return this->findBlockWithProperty([&](BlockType* b) {return b->next_ == &block; });
	}

	template<typename BlockType>
    BlockType& ExplicitSequence<BlockType>::insertFirst()
	{
		// najskor zistit, ci je toto prazdna sekvencia
		if (this->isEmpty())		// isEmpty() je metoda definovana v predkovi abstract memory type
		{
			// alokujeme pamat a dame first aj last ukazovat na nu
			first_ = last_ = this->memoryManager_->allocateMemory();	// operatory priradenia mozeme dat za seba do 1 riadku
			return *first_;
		}
		else
		{
			return insertBefore(*first_);	// inak vlozit pred doteraz prveho
		}
	}

	template<typename BlockType>
    BlockType& ExplicitSequence<BlockType>::insertLast()
	{
		// najskor zistit, ci je toto prazdna sekvencia
		
		if (this->isEmpty())
		{
			first_ = last_ = this->memoryManager_->allocateMemory();
			return *last_;
		}
		else
		{
			return insertAfter(*last_);		// inak vlozit za doteraz posledneho
		}
	}

	template<typename BlockType>
    BlockType& ExplicitSequence<BlockType>::insert(size_t index)	// vloz novy blok na index
	{
		// najskor kontroly:
		//		- vkladame na index 0? insertFirst
		//		- vkladame na index = size() (teda ZA posledny platny prvok)? insertLast
		//		- inac: ziskaj blok na indexe - 1 (jeden blok pred indexom, na ktory chceme vkladat) a vloz ZAN
		return index == 0 ? insertFirst() : index == this->size() ? insertLast() : insertAfter(*access(index - 1));
	}

	template<typename BlockType>
    BlockType& ExplicitSequence<BlockType>::insertAfter(BlockType& block)	// pojde jednoducho vzdy (jedno aj obojstranna)
	{
		// ideme vlozit novy blok medzi param blok a jeho nasledovnika (ak ho ma)

		BlockType* nextBlock = accessNext(block);				// ziskame nasledujuci blok param bloku => novy blok budeme vkladat PRED neho
		BlockType* newBlock = this->memoryManager_->allocateMemory();		// pridelime pamat novemu bloku

		connectBlocks(&block, newBlock);						// spojime param blok a novy blok 
																	//	- nasledovnik param bloku = novy blok
																	//	- predchodca noveho bloku = param blok
		connectBlocks(newBlock, nextBlock);					// spojime novy blok a jeho nasledovnika

		if (last_ == &block)	// ak vkladame za doteraz posledny blok
		{
			last_ = newBlock;	// musime aktualizovat novy posledny blok
		}

		return *newBlock;
	}

	template<typename BlockType>
    BlockType& ExplicitSequence<BlockType>::insertBefore(BlockType& block)	// vyzaduje O(n) v jednostrannej (potrebuje ziskat predchodcu)
	{
		BlockType* previousBlock = accessPrevious(block);
		BlockType* newBlock = this->memoryManager_->allocateMemory();

		connectBlocks(previousBlock, newBlock);
		connectBlocks(newBlock, &block);

		if (first_ == &block)
		{
			first_ = newBlock;
		}

		return *newBlock;
	}

	template<typename BlockType>
    void ExplicitSequence<BlockType>::removeFirst()
	{
		// ak je v skevencii jediny prvok (prvy = posledny)
		if (first_ == last_)
		{
			this->memoryManager_->releaseMemory(first_);
			first_ = last_ = nullptr;
		}
		else
		{
			BlockType* newFirst = accessNext(*first_);	// najskor ziskame nasledovnika aktualne prveho prvku (!! aby sme NEstratili odkaz nan pri mazani a mohli ho potom nastavit ako noveho firsta !!)

			this->memoryManager_->releaseMemory(first_);		// az potom vymazeme prvy prvok
			first_ = newFirst;							// ako novy prvy prvok nastavime nasledovnika vymazaneho prveho prvku
		}
	}

	template<typename BlockType>
    void ExplicitSequence<BlockType>::removeLast()
	{
		if (first_ == last_)
		{
			this->memoryManager_->releaseMemory(last_);
			first_ = last_ = nullptr;
		}
		else
		{
			BlockType* newLast = accessPrevious(*last_);
			this->memoryManager_->releaseMemory(last_);
			last_ = newLast;
			last_->next_ = nullptr;
		}
	}

	template<typename BlockType>
    void ExplicitSequence<BlockType>::remove(size_t index)
	{
		if (index == 0)
		{
			removeFirst();
		}
		else
		{
			// spristupni PREDCHODCU bloku na indexe a odstran jeho nasledovnika (teda ten blok na param indexe)
			removeNext(*access(index - 1));
		}
	}

	template<typename BlockType>
    void ExplicitSequence<BlockType>::removeNext(const BlockType& block)
	{
		BlockType* removedBlock = accessNext(block);		// mazany blok je nasledovnik param bloku

		if (removedBlock == last_)							// kontrola: je mazany blok posledny?
		{
			removeLast();
		}
		else {
			disconnectBlock(removedBlock);					// inac odpoj blok od jeho predchodcu a nasledovnika
			this->memoryManager_->releaseMemory(removedBlock);	// a uvolni pamat nad odtranovanym blokom
		}
	}

	template<typename BlockType>
    void ExplicitSequence<BlockType>::removePrevious(const BlockType& block)
	{
		BlockType* removedBlock = accessPrevious(block);	// mazany blok je predchodca param bloku

		if (removedBlock == first_)							// kontrola: je mazany blok prvy?
		{
			removeFirst();
		}
		else
		{
			disconnectBlock(removedBlock);
			this->memoryManager_->releaseMemory(removedBlock);
		}
	}

	template<typename BlockType>
    void ExplicitSequence<BlockType>::connectBlocks(BlockType* previous, BlockType* next)
	{
		if (previous != nullptr)
		{
			// nastav nasledovnika predchodcu
			previous->next_ = next;
		}
	}

	template<typename BlockType>
    void ExplicitSequence<BlockType>::disconnectBlock(BlockType* block)
	{
		// najskor spoj predchodcu a nasledovnika odpajaneho param bloku
		connectBlocks(accessPrevious(*block), accessNext(*block));
		// a potom zrus vazbu odpajaneho bloku, teda zrus jeho nasledovnika
		block->next_ = nullptr;
	}

    template <typename BlockType>
    ExplicitSequence<BlockType>::ExplicitSequenceIterator::ExplicitSequenceIterator(BlockType* position) :
		position_(position)
    {
    }

    template <typename BlockType>
    ExplicitSequence<BlockType>::ExplicitSequenceIterator::ExplicitSequenceIterator(
        const ExplicitSequenceIterator& other
	) :
		position_(other.position_)
    {
    }

    template <typename BlockType>
    auto ExplicitSequence<BlockType>::ExplicitSequenceIterator::operator++() -> ExplicitSequenceIterator&
    {
		// nasledovnika pozicie (ta je typu BlockType*) treba static castnut
		position_ = static_cast<BlockType*>(position_->next_);
		return *this;
    }

    template <typename BlockType>
    auto ExplicitSequence<BlockType>::ExplicitSequenceIterator::operator++(int) -> ExplicitSequenceIterator
    {
		ExplicitSequenceIterator tmp(*this); operator++();
	    return tmp;
    }

    template <typename BlockType>
    bool ExplicitSequence<BlockType>::ExplicitSequenceIterator::operator==(const ExplicitSequenceIterator& other) const
    {
		return position_ == other.position_;
    }

    template <typename BlockType>
    bool ExplicitSequence<BlockType>::ExplicitSequenceIterator::operator!=(const ExplicitSequenceIterator& other) const
    {
		return !(*this == other);
    }

    template <typename BlockType>
    typename ExplicitSequence<BlockType>::DataType& ExplicitSequence<BlockType>::ExplicitSequenceIterator::operator*()
    {
		// spristupnenie toho, co sa nachadza v bloku position_, teda DAT v tomto bloku
		return position_->data_;
    }

    template <typename BlockType>
    typename ExplicitSequence<BlockType>::ExplicitSequenceIterator ExplicitSequence<BlockType>::begin()
    {
		return ExplicitSequenceIterator(first_);
    }

    template <typename BlockType>
    typename ExplicitSequence<BlockType>::ExplicitSequenceIterator ExplicitSequence<BlockType>::end()
    {
		return ExplicitSequenceIterator(nullptr);
    }

    template<typename DataType>
    DLSBlock<DataType>* DoublyLinkedSequence<DataType>::access(size_t index) const		// spristupni blok na indexe
	{
		// kedze mame obojstranne zretazenu sekvenciu, mozeme sa pohybovat obomi smermi => podla toho, ktory nam je vyhodnejsi v zavislosti od param indexu
		BlockType* result = nullptr;

		if (index >= 0 && index < this->size())
		{
			// ak sa pozadovany index nachadza v prvej polovici sekvencie
			if (index < this->size() / 2)
			{
				// je vyhodnejsie ist od zaciatku
				result = this->first_;
				// a postupne prechadzame sekvenciou pomocou nasledovnikov index-krat
				for (size_t i = 1; i <= index; i++)
				{
					// blokmi prechadzame pomocou nasledovnikov
					// musime ale static castnut next na BlockType*
					result = static_cast<BlockType*>(result->next_);
				}
			}
			// inac ak sa pozadovany index nachadza v druhej polovici sekvencie
			else
			{
				// je vyhodnejsie ist od konca
				result = this->last_;
				// a postupne SPATNE prechadzame sekvenciou pomocou predchodcov
				for (size_t i = this->size() - index; i >= 2; i--)
				{
					result = result->previous_;
				}
			}
		}

		return result;
	}

	template<typename DataType>
    DLSBlock<DataType>* DoublyLinkedSequence<DataType>::accessPrevious(const BlockType& block) const
	{
		return block.previous_;
	}

	template<typename DataType>
    void DoublyLinkedSequence<DataType>::removeFirst()
	{
		ExplicitSequence<BlockType>::removeFirst();

		if (ExplicitSequence<BlockType>::first_ != nullptr)
		{
			ExplicitSequence<BlockType>::first_->previous_ = nullptr;
		}
	}

	template<typename DataType>
    void DoublyLinkedSequence<DataType>::connectBlocks(BlockType* previous, BlockType* next)
	{
		// vo vseobecnom pripade sa len nastavi novy nasledovnik predchodcu
		ExplicitSequence<BlockType>::connectBlocks(previous, next);

		if (next != nullptr)
		{
			// pri obojstrannom zretazeni musime navyse nastavit aj predchodcu nasledovnika
			next->previous_ = const_cast<BlockType*>(previous);
		}
	}

	template<typename DataType>
    void DoublyLinkedSequence<DataType>::disconnectBlock(BlockType* block)
	{
		// vo vseobecnom pripade sa odpoji len vazba na nasledovnika
		ExplicitSequence<BlockType>::disconnectBlock(block);
		// pri obojstrannom zretazeni musime navyse odstranit aj vazbu na predchodcu
		block->previous_ = nullptr;
	}

}