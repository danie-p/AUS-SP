#pragma once

#include <libds/heap_monitor.h>

namespace ds::mm {

	template<typename BlockType>
	class MemoryManager {	// spravca pamate

	public:
		MemoryManager();
		virtual ~MemoryManager();

		virtual BlockType* allocateMemory();
		virtual void releaseMemory(BlockType* pointer);

		void releaseAndSetNull(BlockType*& pointer);

		size_t getAllocatedBlockCount() const;

	protected:
		size_t allocatedBlockCount_;
	};

	template<typename BlockType>
	MemoryManager<BlockType>::MemoryManager() :
		allocatedBlockCount_(0)
	{
	}

	template<typename BlockType>
	MemoryManager<BlockType>::~MemoryManager()
	{
		allocatedBlockCount_ = 0;
	}

	// new a delete mame obalene v allocate a release => mozeme si ich prekryt
	template<typename BlockType>
	BlockType* MemoryManager<BlockType>::allocateMemory()					// zavola new
	{
		allocatedBlockCount_++;
		return new BlockType();
	}

	template<typename BlockType>
	void MemoryManager<BlockType>::releaseMemory(BlockType* pointer)		// zavola delete na pointer
	{
		allocatedBlockCount_--;
		delete pointer;
	}

	// pointer => papierik (sticky note) - hovori, kde je ulozeny kabat
	// referencia => konkretny ten jeden objekt (kabat)
	template<typename BlockType>
	void MemoryManager<BlockType>::releaseAndSetNull(BlockType*& pointer)	// referencia na pointer: "povodny papierik" => znicim aj povodny; inac by na papieriku ostala naplatna adresa => ked znicime referenciu, znici sa ten povodny papierik (adresa na null)
	{
		releaseMemory(pointer);
		pointer = nullptr;
	}

	template<typename BlockType>
	size_t MemoryManager<BlockType>::getAllocatedBlockCount() const
	{
		return allocatedBlockCount_;
	}
}