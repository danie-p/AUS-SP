#pragma once

#include <libds/amt/abstract_memory_type.h>
#include <libds/amt/hierarchy.h>
#include <cmath>

// todo
// postorder
// iterator postorderu
// implicitna hierarchia

namespace ds::amt {

	template<typename DataType, size_t K>
	class ImplicitHierarchy :
		virtual public KWayHierarchy<MemoryBlock<DataType>, K>,
		public ImplicitAMS<DataType>
	{
	public:
		ImplicitHierarchy();
		~ImplicitHierarchy() override;

		size_t level(const MemoryBlock<DataType>& node) const override;
		size_t level(size_t index) const;
		size_t degree(const MemoryBlock<DataType>& node) const override;
		size_t degree(size_t index) const;
		// Overload with one parameter hides overload with no parameter. We need to explicitly 'include' it.
		using Hierarchy<MemoryBlock<DataType>>::nodeCount;
		size_t nodeCount(const MemoryBlock<DataType>& node) const override;

		MemoryBlock<DataType>* accessRoot() const override;
		MemoryBlock<DataType>* accessParent(const MemoryBlock<DataType>& node) const override;
		MemoryBlock<DataType>* accessSon(const MemoryBlock<DataType>& node, size_t sonOrder) const override;
		MemoryBlock<DataType>* accessLastLeaf() const;

		MemoryBlock<DataType>& emplaceRoot() override; // throw(unavailable_function_call)
		void changeRoot(MemoryBlock<DataType>* newRoot) override; // throw(unavailable_function_call)

		MemoryBlock<DataType>& emplaceSon(MemoryBlock<DataType>& parent, size_t sonOrder) override; // throw(unavailable_function_call)
		void changeSon(MemoryBlock<DataType>& parent, size_t sonOrder, MemoryBlock<DataType>* newSon) override; // throw(unavailable_function_call)
		void removeSon(MemoryBlock<DataType>& parent, size_t sonOrder) override; // throw(unavailable_function_call)

		MemoryBlock<DataType>& insertLastLeaf();
		void removeLastLeaf();

		size_t indexOfParent(const MemoryBlock<DataType>& node) const;
		size_t indexOfParent(size_t index) const;
		size_t indexOfSon(const MemoryBlock<DataType>& node, size_t sonOrder) const;
		size_t indexOfSon(size_t indexOfParent, size_t sonOrder) const;
	};

	template<typename DataType, size_t K>
	using IH = ImplicitHierarchy<DataType, K>;

	//----------

	template<typename DataType>
	class BinaryImplicitHierarchy :
		public BinaryHierarchy<MemoryBlock<DataType>>,
		public ImplicitHierarchy<DataType, 2>
	{
	};

	template<typename DataType>
	using BinaryIH = BinaryImplicitHierarchy<DataType>;

	//----------

	template<typename DataType, size_t K>
    ImplicitHierarchy<DataType, K>::ImplicitHierarchy()
	{
	}

	template<typename DataType, size_t K>
    ImplicitHierarchy<DataType, K>::~ImplicitHierarchy()
	{
	}

	// tu prebera param vrchol
	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::level(const MemoryBlock<DataType>& node) const
	{
		// SKP nam vie vratit index param vrchola; nad nim uz vieme zavolat metodu na zistenie urovne, ktora prebera index
		return this->level(this->getMemoryManager()->calculateIndex(node));
	}

	// tu prebera param index
	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::level(size_t index) const
	{
		return static_cast<size_t>(std::floor(std::log((K - 1) * (index + 1)) / std::log(K)));
	}

	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::degree(const MemoryBlock<DataType>& node) const
	{
		return degree(this->getMemoryManager()->calculateIndex(node));
	}

	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::degree(size_t index) const
	{
		// porovnavame s rodicom posledneho listu!
		
		size_t currentLevel = level(index);
		size_t indexOfLast = this->size() - 1;
		size_t depth = level(indexOfLast);

		if (currentLevel == depth)
		{
			return 0;
		}
		else if (currentLevel == depth - 1)
		{
			size_t indexOfParentOfLast = indexOfParent(indexOfLast);
			if (index < indexOfParentOfLast)
			{
				return K;
			}
			else if (index > indexOfParentOfLast)
			{
				return 0;
			}
			else
			{
				// celkovu velkost struktury podelim Kckom a zistim modulo
				size_t mod = (this->size() - 1) % K;
				return mod == 0 ? K : mod;
			}
		}
		else
		{
			return K;
		}
	}

	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::nodeCount(const MemoryBlock<DataType>& node) const
	{
		return this->getMemoryManager()->calculateIndex(node) == 0 ? this->size() : Hierarchy<MemoryBlock<DataType>>::nodeCount(node);
	}

	template<typename DataType, size_t K>
    MemoryBlock<DataType>* ImplicitHierarchy<DataType, K>::accessRoot() const
	{
		// ak struktura obsahuje aspon jeden prvok, SKP vrati blok na 0. indexe
		return this->size() > 0 ? &(this->getMemoryManager()->getBlockAt(0)) : nullptr;
	}

	template<typename DataType, size_t K>
    MemoryBlock<DataType>* ImplicitHierarchy<DataType, K>::accessParent(const MemoryBlock<DataType>& node) const
	{
		// ziskame si index rodica param vrchola
		size_t index = indexOfParent(node);
		// nad indexom rodica SKP vie najst prislusny blok a vratit ho
		return index != INVALID_INDEX ? &(this->getMemoryManager()->getBlockAt(index)) : nullptr;
	}

	template<typename DataType, size_t K>
    MemoryBlock<DataType>* ImplicitHierarchy<DataType, K>::accessSon(const MemoryBlock<DataType>& node, size_t sonOrder) const
	{
		size_t index = indexOfSon(node, sonOrder);
		return index < this->size() && index >= 0 ? &this->getMemoryManager()->getBlockAt(index) : nullptr;
	}

	template<typename DataType, size_t K>
    MemoryBlock<DataType>* ImplicitHierarchy<DataType, K>::accessLastLeaf() const
	{
		// SKP najde posledny PLATNY! blok v strukture a vrati ho
		return this->size() != 0 ? &(this->getMemoryManager()->getBlockAt(this->size() - 1)) : nullptr;
	}

	template<typename DataType, size_t K>
    MemoryBlock<DataType>& ImplicitHierarchy<DataType, K>::emplaceRoot()
	{
		throw unavailable_function_call("Method emplace_root() unavailable in implicit hierarchies!");
	}

	template<typename DataType, size_t K>
    void ImplicitHierarchy<DataType, K>::changeRoot(MemoryBlock<DataType>* newRoot)
	{
		throw unavailable_function_call("Method changeRoot() unavailable in implicit hierarchies!");
	}

	template<typename DataType, size_t K>
    MemoryBlock<DataType>& ImplicitHierarchy<DataType, K>::emplaceSon(MemoryBlock<DataType>& parent, size_t sonOrder)
	{
		throw unavailable_function_call("Method emplaceSon() unavailable in implicit hierarchies!");
	}

	template<typename DataType, size_t K>
    void ImplicitHierarchy<DataType, K>::changeSon(MemoryBlock<DataType>& parent, size_t sonOrder, MemoryBlock<DataType>* newSon)
	{
		throw unavailable_function_call("Method changeSon() unavailable in implicit hierarchies!");
	}

	template<typename DataType, size_t K>
    void ImplicitHierarchy<DataType, K>::removeSon(MemoryBlock<DataType>& parent, size_t sonOrder)
	{
		throw unavailable_function_call("Method removeSon() unavailable in implicit hierarchies!");
	}

	template<typename DataType, size_t K>
    MemoryBlock<DataType>& ImplicitHierarchy<DataType, K>::insertLastLeaf()
	{
		// poziadame SKP o alokovanie bloku pamate na konci struktury a vratime referenciu na tento blok
		return *(this->getMemoryManager()->allocateMemory());
	}

	template<typename DataType, size_t K>
    void ImplicitHierarchy<DataType, K>::removeLastLeaf()
	{
		// poziadame SKP o uvolnenie pamate na konci struktury
		this->getMemoryManager()->releaseMemory();
	}

	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::indexOfParent(const MemoryBlock<DataType>& node) const
	{
		return indexOfParent(this->getMemoryManager()->calculateIndex(node));
	}

	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::indexOfParent(size_t index) const
	{
		// ak sa snazime ziskat index otca korena, je NEplatny
		return index == 0 ? INVALID_INDEX : (index - 1) / K;
	}

	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::indexOfSon(const MemoryBlock<DataType>& node, size_t sonOrder) const
	{
		// SKP vypocita index a zavolame metodu indexOfSon, kde posielame index rodica
		return indexOfSon(this->getMemoryManager()->calculateIndex(node), sonOrder);
	}

	template<typename DataType, size_t K>
    size_t ImplicitHierarchy<DataType, K>::indexOfSon(size_t indexOfParent, size_t sonOrder) const
	{
		return K * indexOfParent + sonOrder + 1;
	}

}