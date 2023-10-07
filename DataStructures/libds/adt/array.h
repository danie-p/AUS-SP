// QUESTION

#pragma once

#include <libds/adt/abstract_data_type.h>
#include <libds/amt/implicit_sequence.h>

namespace ds::adt {

	class Dimension
	{
	public:
		// Q: preco je base long long? nejaky konkretny dovod?
		Dimension(long long base, size_t size);

		long long getBase() const;
		size_t getSize() const;

		bool operator==(const Dimension& other) const;
		bool operator!=(const Dimension& other) const;

	private:
		long long base_;
		size_t size_;
	};

	//----------

	template <typename T>
	class Array :
		public ADS<T>
	{
	public:
		using IteratorType = typename amt::IS<T>::IteratorType;

	public:
		// v poli musi vzdy ostat fixna velkost = zakazeme modifikacie sekvencie
		// pole nad impicitnou sekvenciou (efektivne)
		Array(size_t size);
		Array(Dimension dimension);
		Array(const Array<T>& other);

		// prekryvame, lebo niektore operacie NIE su povolene
		ADT& assign(const ADT& other) override;		// priradovane pole musi mat rovnaku velkost
		void clear() override;
		bool equals(const ADT& other) override;
		bool isEmpty() const override;				// pole NEmoze byt prazdne => isEmpty bude VZDY vracat false
		size_t size() const override;
		long long getBase() const;

		T access(long long index) const;
		void set(T element, long long index);

		IteratorType begin();
		IteratorType end();

	private:
		bool validateIndex(long long index) const;	// valid index patri do <baza, baza + index)    baza + index tam uz NEpatri
		size_t mapIndex(long long index) const;		// index - baza
		amt::IS<T>* getSequence() const;

	private:
		long long base_;
	};

	//----------

	template <typename T>
	class Matrix :
		virtual public ADT
	{
	public:
		virtual T access(long long index1, long long index2) const = 0;
		virtual void set(T element, long long index1, long long index2) = 0;
	};

	//----------

	template <typename T>
	class CompactMatrix :
		public Matrix<T>,
		public ADS<T>
	{
	public:
		CompactMatrix(size_t size1, size_t size2);
		CompactMatrix(Dimension dimension1, Dimension dimension2);
		CompactMatrix(const CompactMatrix& other);

		ADT& assign(const ADT& other) override;
		void clear() override;
		bool equals(const ADT& other) override;
		bool isEmpty() const override;
		size_t size() const override;
		Dimension getDimension1() const;
		Dimension getDimension2() const;

		T access(long long index1, long long index2) const override;
		void set(T element, long long index1, long long index2) override;

	private:
		bool validateIndices(long long index1, long long index2) const;
		size_t mapIndices(long long index1, long long index2) const;
		amt::IS<T>* getSequence() const;

	private:
		Dimension dimension1_;
		Dimension dimension2_;
	};

	//----------

	inline Dimension::Dimension(long long base, size_t size) :
		base_(base),
		size_(size)
	{
	}

	inline long long Dimension::getBase() const
	{
		return base_;
	}

	inline size_t Dimension::getSize() const
	{
		return size_;
	}

	inline bool Dimension::operator==(const Dimension& other) const
	{
		return base_ == other.base_ && size_ == other.size_;
	}

	inline bool Dimension::operator!=(const Dimension& other) const
	{
		return !(*this == other);
	}

	//----------

	template<typename T>
	Array<T>::Array(size_t size) :
		Array(Dimension(0, size))
	{
	}

	template<typename T>
	Array<T>::Array(Dimension dimension) :
		ADS<T>(new amt::IS<T>(dimension.getSize(), true)),
		base_(dimension.getBase())
	{
	}

	template<typename T>
	Array<T>::Array(const Array<T>& other) :
		ADS<T>(new amt::IS<T>(), other),
		base_(other.base_)
	{
	}

	// impl
	template<typename T>
	ADT& Array<T>::assign(const ADT& other)
	{
		const Array<T>* otherArray = dynamic_cast<const Array<T>*>(&other);
		if (!otherArray)
		{
			this->error("Assigned ADT is of incorrect type!");
		}

		if (base_ != otherArray->base_ || size() != otherArray->size())
		{
			this->error("Array dimensions are different!");
		}

		ADS<T>::assign(other);
		return *this;
	}

	template<typename T>
	void Array<T>::clear()
	{
		this->error("Array can't be cleared!");
	}

	template<typename T>
	bool Array<T>::equals(const ADT& other)
	{
		const Array<T>* otherArray = dynamic_cast<const Array<T>*>(&other);
		return otherArray != nullptr &&
			base_ == otherArray->base_ &&
			ADS<T>::equals(other);
	}

	template<typename T>
	bool Array<T>::isEmpty() const
	{
		return false;
	}

	template<typename T>
	size_t Array<T>::size() const
	{
		return this->getSequence()->size();
	}

	template<typename T>
	long long Array<T>::getBase() const
	{
		return base_;
	}

	// impl
	template<typename T>
	T Array<T>::access(long long index) const
	{
		if (!validateIndex(index))
		{
			this->error("Invalid index!");
		}

		return this->getSequence()->access(mapIndex(index))->data_;
	}

	// impl
	template<typename T>
	void Array<T>::set(T element, long long index)
	{
		if (!validateIndex(index))
		{
			this->error("Invalid index!");
		}

		this->getSequence()->access(mapIndex(index))->data_ = element;
	}

	// impl
	template <typename T>
	auto Array<T>::begin() -> IteratorType
	{
		return this->getSequence()->begin();
	}

	// impl
	template <typename T>
	auto Array<T>::end() -> IteratorType
	{
		return this->getSequence()->end();
	}

	template<typename T>
	amt::IS<T>* Array<T>::getSequence() const
	{
		return dynamic_cast<amt::IS<T>*>(this->memoryStructure_);
	}

	// impl
	template<typename T>
	bool Array<T>::validateIndex(long long index) const
	{
		// index < base_ + this->size(); bude problem, ak base_ je int a this->size() je size_t
		return index >= base_ && index < base_ + static_cast<long long>(this->size());
	}

	// impl
	template<typename T>
	size_t Array<T>::mapIndex(long long index) const
	{
		return index - base_;
	}

	//----------

	template<typename T>
	CompactMatrix<T>::CompactMatrix(size_t size1, size_t size2) :
		CompactMatrix<T>(Dimension(0, size1), Dimension(0, size2))
	{
	}

	template<typename T>
	CompactMatrix<T>::CompactMatrix(Dimension dimension1, Dimension dimension2) :
		ADS<T>(new amt::IS<T>(dimension1.getSize()* dimension2.getSize(), true)),
		dimension1_(dimension1),
		dimension2_(dimension2)
	{
	}

	template<typename T>
	CompactMatrix<T>::CompactMatrix(const CompactMatrix<T>& other) :
		ADS<T>(new amt::IS<T>(), other),
		dimension1_(other.dimension1_),
		dimension2_(other.dimension2_)
	{
	}

	template<typename T>
	ADT& CompactMatrix<T>::assign(const ADT& other)
	{
		const CompactMatrix<T>& otherMatrix = dynamic_cast<const CompactMatrix<T>&>(other);
		if (dimension1_ != otherMatrix.dimension1_ || dimension2_ != otherMatrix.dimension2_)
		{
			this->error("CompactMatrix dimensions are different!");
		}
		ADS<T>::assign(otherMatrix);
		return *this;
	}

	template<typename T>
	void CompactMatrix<T>::clear()
	{
		this->error("CompactMatrix can't be cleared!");
	}

	template<typename T>
	bool CompactMatrix<T>::equals(const ADT& other)
	{
		const CompactMatrix<T>* otherMatrix = dynamic_cast<const CompactMatrix<T>*>(&other);
		return otherMatrix != nullptr &&
			dimension1_ == otherMatrix->dimension1_ &&
			dimension2_ == otherMatrix->dimension2_ &&
			ADS<T>::equals(other);
	}

	template<typename T>
	bool CompactMatrix<T>::isEmpty() const
	{
		return false;
	}

	template<typename T>
	size_t CompactMatrix<T>::size() const
	{
		return dimension1_.getSize() * dimension2_.getSize();
	}

	template <typename T>
	Dimension CompactMatrix<T>::getDimension1() const
	{
		return dimension1_;
	}

	template <typename T>
	Dimension CompactMatrix<T>::getDimension2() const
	{
		return dimension2_;
	}

	// impl
	template<typename T>
	T CompactMatrix<T>::access(long long index1, long long index2) const
	{
		if (!validateIndices(index1, index2))
		{
			this->error("Invalid index!");
		}

		size_t mappedIndex = mapIndices(index1, index2);
		return this->getSequence()->access(mappedIndex)->data_;
	}

	// impl
	template<typename T>
	void CompactMatrix<T>::set(T element, long long index1, long long index2)
	{
		if (!validateIndices(index1, index2))
		{
			this->error("Invalid index!");
		}

		size_t mappedIndex = mapIndices(index1, index2);
		this->getSequence()->access(mappedIndex)->data_ = element;
	}

	// impl
	template<typename T>
	bool CompactMatrix<T>::validateIndices(long long index1, long long index2) const
	{
		return index1 >= dimension1_.getBase() &&
			index1 < dimension1_.getBase() + static_cast<long long>(dimension1_.getSize()) &&
			index2 >= dimension2_.getBase() &&
			index2 < dimension2_.getBase() + static_cast<long long>(dimension2_.getSize());
	}

	// impl
	template<typename T>
	size_t CompactMatrix<T>::mapIndices(long long index1, long long index2) const
	{
		return (index1 - dimension1_.getBase()) * static_cast<long long>(dimension2_.getSize()) +
			(index2 - dimension2_.getBase());
	}

	template<typename T>
	amt::IS<T>* CompactMatrix<T>::getSequence() const
	{
		return dynamic_cast<amt::IS<T>*>(this->memoryStructure_);
	}
}