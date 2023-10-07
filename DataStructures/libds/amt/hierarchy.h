#pragma once

#include <libds/amt/abstract_memory_type.h>
#include <libds/amt/explicit_sequence.h>
#include <functional>

namespace ds::amt {

	template<typename BlockType>
	class Hierarchy :
		virtual public AMT
	{
	public:
		virtual size_t level(const BlockType& node) const;
		virtual size_t degree(const BlockType& node) const = 0;
		virtual size_t nodeCount() const;
	    virtual size_t nodeCount(const BlockType& node) const;

		virtual BlockType* accessRoot() const = 0;
		virtual BlockType* accessParent(const BlockType& node) const = 0;
		virtual BlockType* accessSon(const BlockType& node, size_t sonOrder) const = 0;

		virtual bool isRoot(const BlockType& node) const;
		virtual bool isNthSon(const BlockType& node, size_t sonOrder) const;
		virtual bool isLeaf(const BlockType& node) const;
		virtual bool hasNthSon(const BlockType& node, size_t sonOrder) const;

		virtual BlockType& emplaceRoot() = 0;
		virtual void changeRoot(BlockType* newRoot) = 0;

		// metody syna vyzeraju inak pri roznych druhoch hierarchie, lebo maju mnozinu synov rozne implementovane
		virtual BlockType& emplaceSon(BlockType& parent, size_t sonOrder) = 0;
		virtual void changeSon(BlockType& parent, size_t sonOrder, BlockType* newSon) = 0;
		virtual void removeSon(BlockType& parent, size_t sonOrder) = 0;

		void processPreOrder(const BlockType* node, std::function<void(const BlockType*)> operation) const;
		void processPostOrder(BlockType* node, std::function<void(BlockType*)> operation) const;
		void processLevelOrder(BlockType* node, std::function<void(BlockType*)> operation) const;

	protected:
		using DataType = decltype(BlockType().data_);

		// iterator do hlbky
		class DepthFirstIterator
		{
		protected:
			struct DepthFirstIteratorPosition
			{
				DepthFirstIteratorPosition(BlockType* currentNode, DepthFirstIteratorPosition* previousPosition) :
					currentNode_(currentNode),
					currentSon_(nullptr),
					currentSonOrder_(INVALID_INDEX),
					visitedSonCount_(0),
					currentNodeProcessed_(false),
					previousPosition_(previousPosition)
				{}

			    DepthFirstIteratorPosition(const DepthFirstIteratorPosition& other) :
					currentNode_(other.currentNode_),
					currentSon_(other.currentSon_),
					currentSonOrder_(other.currentSonOrder_),
					visitedSonCount_(other.visitedSonCount_),
					currentNodeProcessed_(other.currentNodeProcessed_),
					previousPosition_(other.previousPosition_)
				{}

			    ~DepthFirstIteratorPosition() {
					currentNode_ = nullptr;
					currentSon_ = nullptr;
					currentSonOrder_ = 0;
					visitedSonCount_ = 0;
					currentNodeProcessed_ = false;
					previousPosition_ = nullptr;
				}

				BlockType* currentNode_;
				BlockType* currentSon_;
				size_t currentSonOrder_;
				size_t visitedSonCount_;
				bool currentNodeProcessed_;
				DepthFirstIteratorPosition* previousPosition_;		// ukazovatel na predchadzajuci "ramec"
			};

		public:
			DepthFirstIterator(Hierarchy<BlockType>* hierarchy);
			DepthFirstIterator(const DepthFirstIterator& other);
			~DepthFirstIterator();
			bool operator==(const DepthFirstIterator& other) const;
			bool operator!=(const DepthFirstIterator& other) const;
			DataType& operator*();

		protected:
			void savePosition(BlockType* currentNode);				// vytvori ramec na konci
			void removePosition();									// zrusi ramec na konci
			bool tryFindNextSonInCurrentPosition();

			Hierarchy<BlockType>* hierarchy_;
			DepthFirstIteratorPosition* currentPosition_;
		};

	public:
		// iterator v priamom poradi
		class PreOrderHierarchyIterator :
			public DepthFirstIterator
		{
		public:
			// vsetko uz mame naimplementovane
			PreOrderHierarchyIterator(Hierarchy<BlockType>* hierarchy, BlockType* node);
			PreOrderHierarchyIterator(const PreOrderHierarchyIterator& other);
			// musime dodefinovat ako ma ist vpred
			PreOrderHierarchyIterator& operator++();
		};

		//----------

		// iterator v spatnom poradi
		class PostOrderHierarchyIterator :
			public DepthFirstIterator
		{
		public:
			PostOrderHierarchyIterator(Hierarchy<BlockType>* hierarchy, BlockType* node);
			PostOrderHierarchyIterator(const PreOrderHierarchyIterator& other);
			PostOrderHierarchyIterator& operator++();
		};

		//----------

		using IteratorType = PreOrderHierarchyIterator;

		IteratorType begin();
		IteratorType end();
		PreOrderHierarchyIterator beginPre();
		PreOrderHierarchyIterator endPre();
	    PostOrderHierarchyIterator beginPost();
        PostOrderHierarchyIterator endPost();
    };

	//----------

	template<typename BlockType, size_t K>
	class KWayHierarchy :
		virtual public Hierarchy<BlockType>
	{
	};

	//----------

	// binarna hierarchia: prave 2 usporiadani synovia => vieme ich pomenovat ako lavy a pravy
	template<typename BlockType>
	class BinaryHierarchy :
		virtual public KWayHierarchy<BlockType, 2>
	{
	public:
		static const size_t LEFT_SON_INDEX = 0;
		static const size_t RIGHT_SON_INDEX = 1;

		BlockType* accessLeftSon(const BlockType& node) const;
        BlockType* accessRightSon(const BlockType& node) const;

        bool isLeftSon(const BlockType& node) const;
        bool isRightSon(const BlockType& node) const;

        bool hasLeftSon(const BlockType& node) const;
        bool hasRightSon(const BlockType& node) const;

        BlockType& insertLeftSon(BlockType& parent);
        BlockType& insertRightSon(BlockType& parent);

        void changeLeftSon(BlockType& parent, BlockType* newSon);
        void changeRightSon(BlockType& parent, BlockType* newSon);

        void removeLeftSon(BlockType& parent);
        void removeRightSon(BlockType& parent);

        void processInOrder(const BlockType* node, std::function<void(const BlockType*)> operation) const;

		//----------

		// iterator vo vnutornom poradi (len pre binarnu hierarchiu)
		class InOrderHierarchyIterator :
			public Hierarchy<BlockType>::DepthFirstIterator
		{
		public:
			InOrderHierarchyIterator(BinaryHierarchy<BlockType>* hierarchy, BlockType* node);
			InOrderHierarchyIterator(const InOrderHierarchyIterator& other);
			InOrderHierarchyIterator& operator++();

		protected:
			bool tryToGoToLeftSonInCurrentPosition();
			bool tryToGoToRightSonInCurrentPosition();
		};

		using IteratorType = InOrderHierarchyIterator;

		IteratorType begin();
		IteratorType end();
    };

	//----------

	template<typename BlockType>
    size_t Hierarchy<BlockType>::level(const BlockType& node) const
	{
		size_t level = 0;							// zaciname od urovne 0, pretoze koren ma uroven 0
		BlockType* parent = accessParent(node);

		// (prechadzame ako explicitnou sekvenciou) prejdeme na otce aktualneho vrcholu, kym ma aktualny vrchol este otca (teda nie je koren)
		while (parent != nullptr)
		{
			level++;
			parent = accessParent(*parent);			// posun na vyssiu uroven
		}

		return level;
	}

	template<typename BlockType>
    size_t Hierarchy<BlockType>::nodeCount() const
	{
		size_t result = 0;
		// spusti sa preorder prehliadka a operacia je pricitavanie poctu vrcholov o jedna
		// [&result] nam dovoli menit existujuci result v lambde
		// spusti prehliadku nad korenom (teda spocita mohutnost celej hierarchie)
		processPreOrder(accessRoot(), [&result](const BlockType* b) { ++result; });
		return result;
	}

	template<typename BlockType>
    size_t Hierarchy<BlockType>::nodeCount(const BlockType& node) const
	{
		size_t result = 0;
		// spusti preorder z param vrchola => spocita mohutnost podhierarchie, ktorej korenom je param vrchol
		processPreOrder(&node, [&result](const BlockType* b) { ++result; });
		return result;
	}

	template<typename BlockType>
    bool Hierarchy<BlockType>::isRoot(const BlockType& node) const
	{
		// koren je vrchol bez otca
		return accessParent(node) == nullptr;
	}

	template<typename BlockType>
    bool Hierarchy<BlockType>::isNthSon(const BlockType& node, size_t sonOrder) const
	{
		BlockType* parent = accessParent(node);
		// metoda spristupni syna u otca param vrchola na n-tom mieste musi vratit pointer na samotny param vrchol
		return parent != nullptr && accessSon(*parent, sonOrder) == &node;
	}

	template<typename BlockType>
    bool Hierarchy<BlockType>::isLeaf(const BlockType& node) const
	{
		// list ma stupen 0
		return degree(node) == 0;
	}

	template<typename BlockType>
    bool Hierarchy<BlockType>::hasNthSon(const BlockType& node, size_t sonOrder) const
	{
		return accessSon(node, sonOrder) != nullptr;
	}

	template<typename BlockType>
    void Hierarchy<BlockType>::processPreOrder(const BlockType* node, std::function<void(const BlockType*)> operation) const
	{
		// spracuj podhierarchiu vychadzajucu z param vrchola v priamom poradi
		if (node != nullptr)
		{
			operation(node);									// NAJSKOR SPRACUJ VRCHOL
																// potom sa posun na potomkov
			size_t nodeDegree = degree(*node);
			size_t n = 0;								// zacina od 0. syna
			size_t numberOfProcessedSons = 0;					// na zaciatku nie su spracovani ziadni synovia

			while (numberOfProcessedSons < nodeDegree)			// prechadzame, pokial sme nespracovali tolko synov, kolko je stupen vrchola (teda si mozeme byt isti, ze boli spracovani vsetci synovia vrchola)
			{
				BlockType* son = accessSon(*node, n);			// spristupni sa n-ty syn

				if (son != nullptr)								// ak n-ty syn existuje, spustime nad nim preorder prehliadku
				{
					processPreOrder(son, operation);			// rekurziva prehliadka zacinajuca v n-tom synovi
					++numberOfProcessedSons;
				}
				++n;
			}
		}
	}

	template<typename BlockType>
    void Hierarchy<BlockType>::processPostOrder(BlockType* node, std::function<void(BlockType*)> operation) const
	{
		if (node != nullptr)
		{
			size_t nodeDegree = degree(*node);					// NAJPRV SA POSUN NA POTOMKOV
			size_t n = 0;
			size_t numberOfProcessedSons = 0;

			while (numberOfProcessedSons < nodeDegree)
			{
				BlockType* son = accessSon(*node, n);

				if (son != nullptr)
				{
					processPostOrder(son, operation);
					++numberOfProcessedSons;
				}
				++n;
			}
			operation(node);									// potom spracuj vrchol
		}
	}

	template<typename BlockType>
    void Hierarchy<BlockType>::processLevelOrder(BlockType* node, std::function<void(BlockType*)> operation) const
	{
		// spracovanie po urovniach
		if (node != nullptr)
		{
			SinglyLS<BlockType*> sequence;								// vytvorime jednostranne zretazenu sekvenciu NEspracovanych vrcholov
																			// tato sekvencia sa postupne bude naplnat vrcholmi na danej urovni, potom sa prejde na ich synov atd

			sequence.insertFirst().data_ = node;						// do udajovej casti na zaciatku sekvencie vlozime param vrchol (z ktoreho vychadza prehliadka)
			while (!sequence.isEmpty())									// pokial sekvencia nespracovanych vrcholov nie je prazdna
			{
				// tu sa spracuje vrchol zo zaciatku sekvencie
				BlockType* current = sequence.accessFirst()->data_;			// aktualny vrchol = datova cast z 1. prvku sekvencie
				sequence.removeFirst();										// vyhodi 1. prvok zo sekvencie nespracovynch vrcholov (aktualne sa ide spracovat)
				if (current != nullptr)
				{
					operation(current);										// zavola sa OPERACIA nad aktualnym vrcholom

					// tu sa na koniec sekvencie pridavaju vrcholy nasledujucej urovne
					size_t nodeDegree = degree(*current);					// stupen aktulneho vrchola
					size_t n = 0;
					size_t sonsProcessed = 0;
					while (sonsProcessed < nodeDegree)						// pokial neboli spracovani vsetci synovia aktualneho vrchola
					{
						BlockType* son = accessSon(*current, n);			// spristupni sa n-ty syn aktualneho vrchola
						if (son != nullptr)
						{
							sequence.insertLast().data_ = son;				// do udajovej casti na konci sekvencie sa vlozi n-ty syn
							++sonsProcessed;
						}
						++n;
					}
				}
			}
		}
	}

	template<typename BlockType>
    void BinaryHierarchy<BlockType>::processInOrder(const BlockType* node, std::function<void(const BlockType*)> operation) const
	{
		if (node != nullptr)
		{
			this->processInOrder(this->accessLeftSon(*node), operation);	// najprv rekurzivne spusti inorder prehliadku nad LAVYM synom
			operation(node);									// potom SPRACUJ vrchol
			this->processInOrder(this->accessRightSon(*node), operation);	// a potom rekurzivne spusti inorder prehliadku nad PRAVYM synom
		}
	}

	template<typename BlockType>
    Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(Hierarchy<BlockType>* hierarchy) :
		hierarchy_(hierarchy),
		currentPosition_(nullptr)
	{
	}

	template<typename BlockType>
    Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(const DepthFirstIterator& other):	// copy constructor preberajuci iny iterator do hlbky
		DepthFirstIterator(other.hierarchy_)
	{
		DepthFirstIteratorPosition* myPosition = nullptr;
		for (DepthFirstIteratorPosition* otherPosition = other.currentPosition_;
			 otherPosition != nullptr;
			 otherPosition = otherPosition->previousPosition_)
		{
			if (currentPosition_ == nullptr)
			{
				currentPosition_ = new DepthFirstIteratorPosition(*otherPosition);
				myPosition = currentPosition_;
			}
			else
			{
				myPosition->previousPosition_ = new DepthFirstIteratorPosition(*otherPosition);
				myPosition = myPosition->previousPosition_;
			}
		}
	}

	template<typename BlockType>
    Hierarchy<BlockType>::DepthFirstIterator::~DepthFirstIterator()
	{
		while (currentPosition_ != nullptr)
		{
			removePosition();
		}

		hierarchy_ = nullptr;
		currentPosition_ = nullptr;
	}

	template<typename BlockType>
    bool Hierarchy<BlockType>::DepthFirstIterator::operator==(const DepthFirstIterator& other) const
	{
		if (hierarchy_ != other.hierarchy_) { return false; }		// ak sa APS NErovnaju, rovno vrati false

		DepthFirstIteratorPosition* myPosition = currentPosition_;
		DepthFirstIteratorPosition* otherPosition = other.currentPosition_;

		if (myPosition != nullptr && otherPosition != nullptr)
		{
			// aby sa rovnali iteratory, musi sa rovnat AKTUALNY VRCHOL, na ktory ukazuju
				// a musi sa rovnat PORADIE SYNA (vrchola, na ktory iteratory ukazuju) u svojho otca
			if (myPosition->currentNode_ != otherPosition->currentNode_ || myPosition->currentSonOrder_ != otherPosition->currentSonOrder_)
			{
				return false;
			}
		}

		return myPosition == nullptr && otherPosition == nullptr;
	}

	template<typename BlockType>
    bool Hierarchy<BlockType>::DepthFirstIterator::operator!=(const DepthFirstIterator& other) const
	{
		return !(*this == other);
	}

	template<typename BlockType>
    auto Hierarchy<BlockType>::DepthFirstIterator::operator*() -> DataType&
	{
		currentPosition_->currentNodeProcessed_ = true;		// nastavime vrchol na aktualnej bozicii ako spracovany
		return currentPosition_->currentNode_->data_;		// ziskame data z vrcholu na aktualnej pozicii (kde prave iterator ukazuje)
	}

	template<typename BlockType>
    void Hierarchy<BlockType>::DepthFirstIterator::savePosition(BlockType* currentNode)
	{
		// zavola konstruktor a vlozi nully
		// vytvori "ramec" na konci
		currentPosition_ = new DepthFirstIteratorPosition(currentNode, currentPosition_);
	}

	template<typename BlockType>
    void Hierarchy<BlockType>::DepthFirstIterator::removePosition()			// odstranenie (aktualnej) pozicie
	{
		DepthFirstIteratorPosition* positionToRemove = currentPosition_;	// pred vymazanim musime ziskat predchodcu aktualnej pozicie (ktoru chceme vymazat), aby sme nestratili aktualnu poziciu
		currentPosition_ = currentPosition_->previousPosition_;				// aktualizujeme aktualnu poziciu na predchodcu mazanej pozicie
		delete positionToRemove;											// vymazeme poziciu
	}

	template<typename BlockType>
    bool Hierarchy<BlockType>::DepthFirstIterator::tryFindNextSonInCurrentPosition()	// skus ist na dalsieho syna
	{
		// najdi dalsieho nenuloveho syna z miesta, kde aktualne si
		// ak najde, vrati true, inak false
		++currentPosition_->visitedSonCount_;

		size_t currentDegree = hierarchy_->degree(*currentPosition_->currentNode_);
		if (currentPosition_->visitedSonCount_ <= currentDegree)			// ak je pocet navstivenych synov z aktualnej pozicie <= aktualnemu stupnu
		{
			do
			{
				++currentPosition_->currentSonOrder_;						// navys poradie syna na aktualnej pozicii
				currentPosition_->currentSon_ = hierarchy_->accessSon(*currentPosition_->currentNode_, currentPosition_->currentSonOrder_);	// ako syna na aktualnej pozicii nastav syna na aktualizovanom navysenom poradi
			} while (currentPosition_->currentSon_ == nullptr);
			return true;
		}
		else
		{
			currentPosition_->currentSonOrder_ = INVALID_INDEX;
			currentPosition_->currentSon_ = nullptr;
			return false;
		}
	}

	template<typename BlockType>
    Hierarchy<BlockType>::PreOrderHierarchyIterator::PreOrderHierarchyIterator(Hierarchy<BlockType>* hierarchy, BlockType* node):
		Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(hierarchy)
	{
		if (node != nullptr)
		{
			this->savePosition(node);
		}
	}

	template<typename BlockType>
    Hierarchy<BlockType>::PreOrderHierarchyIterator::PreOrderHierarchyIterator(const PreOrderHierarchyIterator& other):
		Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(other)
	{
	}

	template<typename BlockType>
    typename Hierarchy<BlockType>::PreOrderHierarchyIterator& Hierarchy<BlockType>::PreOrderHierarchyIterator::operator++()
	{
		// simulacia callstacku -> iterator si uchovava sekvenciu "ramcov": ak uz nema kde ist, ramec zahodi; ak este ma nejakych synov na spracovanie, prida ramec
		// funguje ako callstack, ale ramce vytvarame na HALDE (pomocou new) -> aby pri velkych rekurziach nedoslo ku stack overflow
		
		if (this->tryFindNextSonInCurrentPosition())
		{
			this->savePosition(this->currentPosition_->currentSon_);		// ak najde este dalsich synov, prida ramec
		}
		else
		{
			this->removePosition();									// ak uz nenajde synov, odstrani ramec
			if (this->currentPosition_ != nullptr)
			{
				++(*this);		// respektive zavolame len operator++();	// rekurzia
			}
		}
		return *this;
	}

	template<typename BlockType>
    Hierarchy<BlockType>::PostOrderHierarchyIterator::PostOrderHierarchyIterator(Hierarchy<BlockType>* hierarchy, BlockType* node) :
		Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(hierarchy)
	{
		if (node != nullptr)
		{
			this->savePosition(node);
			++(*this);
		}
	}

	template<typename BlockType>
    Hierarchy<BlockType>::PostOrderHierarchyIterator::PostOrderHierarchyIterator(const PreOrderHierarchyIterator& other) :
		Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(other)
	{
	}

	template<typename BlockType>
    typename Hierarchy<BlockType>::PostOrderHierarchyIterator& Hierarchy<BlockType>::PostOrderHierarchyIterator::operator++()
	{
		// ak aktualne spracovavany vrchol na aktualnej pozicii je nullptr A ZAROVEN sa podari najst dalsieho nenulloveho syna z aktualnej pozicie
		if (!this->currentPosition_->currentNodeProcessed_ && this->tryFindNextSonInCurrentPosition())
		{
			// tak uloz poziciu aktualneho syna na aktualnej pozicii
			this->savePosition(this->currentPosition_->currentSon_);
			// a posun sa vpred v iteracii
			++(*this);
		}
		else
		{
			// inac ak aktualne spracovavany vrchol na aktualnej pozicii NIE JE nullptr (je platny)
			if (this->currentPosition_->currentNodeProcessed_)
			{
				// tak odstran posledneho a nastav currPos na jeho predchodcu
				this->removePosition();
				// ak aktualna pozicia (predchodca odstraneneho) je este platna
				if (this->currentPosition_ != nullptr)
				{
					// tak sa posun vprad v iteracii
					++(*this);
				}
			}
		}

		return *this;
	}

    template <typename BlockType>
    auto Hierarchy<BlockType>::begin() -> IteratorType
	{
	    return PreOrderHierarchyIterator(this, accessRoot());
	}

    template <typename BlockType>
    auto Hierarchy<BlockType>::end() -> IteratorType
	{
	    return PreOrderHierarchyIterator(this, nullptr);
	}

    template <typename BlockType>
    typename Hierarchy<BlockType>::PreOrderHierarchyIterator Hierarchy<BlockType>::beginPre()
    {
		return PreOrderHierarchyIterator(this, accessRoot());
    }

    template <typename BlockType>
    typename Hierarchy<BlockType>::PreOrderHierarchyIterator Hierarchy<BlockType>::endPre()
    {
		return PreOrderHierarchyIterator(this, nullptr);
    }

    template <typename BlockType>
    typename Hierarchy<BlockType>::PostOrderHierarchyIterator Hierarchy<BlockType>::beginPost()
    {
        return PostOrderHierarchyIterator(this, accessRoot());
    }

    template <typename BlockType>
    typename Hierarchy<BlockType>::PostOrderHierarchyIterator Hierarchy<BlockType>::endPost()
    {
        return PostOrderHierarchyIterator(this, nullptr);
    }

    template <typename BlockType>
    BlockType* BinaryHierarchy<BlockType>::accessLeftSon(const BlockType& node) const
	{
	    return this->accessSon(node, LEFT_SON_INDEX);
	}

    template <typename BlockType>
    BlockType* BinaryHierarchy<BlockType>::accessRightSon(const BlockType& node) const
	{
	    return this->accessSon(node, RIGHT_SON_INDEX);
	}

    template <typename BlockType>
    bool BinaryHierarchy<BlockType>::isLeftSon(const BlockType& node) const
	{
	    return this->isNthSon(node, LEFT_SON_INDEX);
	}

    template <typename BlockType>
    bool BinaryHierarchy<BlockType>::isRightSon(const BlockType& node) const
	{
	    return this->isNthSon(node, RIGHT_SON_INDEX);
	}

    template <typename BlockType>
    bool BinaryHierarchy<BlockType>::hasLeftSon(const BlockType& node) const
	{
	    return this->hasNthSon(node, LEFT_SON_INDEX);
	}

    template <typename BlockType>
    bool BinaryHierarchy<BlockType>::hasRightSon(const BlockType& node) const
	{
	    return this->hasNthSon(node, RIGHT_SON_INDEX);
	}

    template <typename BlockType>
    BlockType& BinaryHierarchy<BlockType>::insertLeftSon(BlockType& parent)
	{
	    return this->emplaceSon(parent, LEFT_SON_INDEX);
	}

    template <typename BlockType>
    BlockType& BinaryHierarchy<BlockType>::insertRightSon(BlockType& parent)
	{
	    return this->emplaceSon(parent, RIGHT_SON_INDEX);
	}

    template <typename BlockType>
    void BinaryHierarchy<BlockType>::changeLeftSon(BlockType& parent, BlockType* newSon)
	{
	    this->changeSon(parent, LEFT_SON_INDEX, newSon);
	}

    template <typename BlockType>
    void BinaryHierarchy<BlockType>::changeRightSon(BlockType& parent, BlockType* newSon)
	{
	    this->changeSon(parent, RIGHT_SON_INDEX, newSon);
	}

    template <typename BlockType>
    void BinaryHierarchy<BlockType>::removeLeftSon(BlockType& parent)
	{
	    this->removeSon(parent, LEFT_SON_INDEX);
	}

    template <typename BlockType>
    void BinaryHierarchy<BlockType>::removeRightSon(BlockType& parent)
	{
	    this->removeSon(parent, RIGHT_SON_INDEX);
	}

    template<typename BlockType>
    BinaryHierarchy<BlockType>::InOrderHierarchyIterator::InOrderHierarchyIterator(BinaryHierarchy<BlockType>* hierarchy, BlockType* node):
		Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(hierarchy)
	{
		if (node != nullptr)
		{
			this->savePosition(node);
			++(*this);
		}
	}

	template<typename BlockType>
    BinaryHierarchy<BlockType>::InOrderHierarchyIterator::InOrderHierarchyIterator(const InOrderHierarchyIterator& other):
		Hierarchy<BlockType>::DepthFirstIterator::DepthFirstIterator(other)
	{
	}

	template<typename BlockType>
    typename BinaryHierarchy<BlockType>::InOrderHierarchyIterator& BinaryHierarchy<BlockType>::InOrderHierarchyIterator::operator++()
	{
		// ak na aktualnej pozicii nie je spracovavany vrchol
		if (!this->currentPosition_->currentNodeProcessed_)
		{
			// ak index aktualneho syna z aktualnej pozicie je rozny od indexu laveho syna A ZAROVEN je mozne sa z aktualnej pozicie posunut na laveho syna
			if (this->currentPosition_->currentSonOrder_ != LEFT_SON_INDEX && tryToGoToLeftSonInCurrentPosition())
			{
				// uloz poziciu aktualneho syna (nelaveho)
				this->savePosition(this->currentPosition_->currentSon_);
				// posun sa vpred
				++(*this);
			}
		}
		else
		{
			// ak index aktualneho syna je rozny od indexu praveho syna A ZAROVEN jemozne sa z aktualnej pozicie posunut na praveho syna
			if (this->currentPosition_->currentSonOrder_ != RIGHT_SON_INDEX && tryToGoToRightSonInCurrentPosition())
			{
				// uloz poziciu aktualneho syna (nepraveho)
				this->savePosition(this->currentPosition_->currentSon_);
				++(*this);
			}
			else {
				// inac odstran posledny prvok
				this->removePosition();
				// a ak predchodca odstraneneho prvku, teda aktualne posledy prvok nie je null
				if (this->currentPosition_ != nullptr)
				{
					// tak sa posun vpred
					++(*this);
				}
			}
		}

		return *this;
	}

	template<typename BlockType>
    bool BinaryHierarchy<BlockType>::InOrderHierarchyIterator::tryToGoToLeftSonInCurrentPosition()
	{
		this->currentPosition_->currentSon_ = this->hierarchy_->accessSon(*this->currentPosition_->currentNode_, BinaryHierarchy<BlockType>::LEFT_SON_INDEX);
		if (this->currentPosition_->currentSon_ != nullptr)
		{
			this->currentPosition_->currentSonOrder_ = BinaryHierarchy<BlockType>::LEFT_SON_INDEX;
			return true;
		}
		else
		{
			this->currentPosition_->currentSonOrder_ = INVALID_INDEX;
			return false;
		}
	}

	template<typename BlockType>
    bool BinaryHierarchy<BlockType>::InOrderHierarchyIterator::tryToGoToRightSonInCurrentPosition()
	{
		this->currentPosition_->currentSon_ = this->hierarchy_->accessSon(*this->currentPosition_->currentNode_, BinaryHierarchy<BlockType>::RIGHT_SON_INDEX);
		if (this->currentPosition_->currentSon_ != nullptr)
		{
			this->currentPosition_->currentSonOrder_ = BinaryHierarchy<BlockType>::RIGHT_SON_INDEX;
			return true;
		}
		else
		{
			this->currentPosition_->currentSonOrder_ = INVALID_INDEX;
			return false;
		}
	}

    template <typename BlockType>
    auto BinaryHierarchy<BlockType>::begin() -> IteratorType
	{
	    return InOrderHierarchyIterator(this, this->accessRoot());
	}

    template <typename BlockType>
    auto BinaryHierarchy<BlockType>::end() -> IteratorType
	{
	    return InOrderHierarchyIterator(this, nullptr);
	}
}
