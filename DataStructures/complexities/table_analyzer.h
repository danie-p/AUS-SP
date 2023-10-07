#pragma once
#include <complexities/complexity_analyzer.h>
#include <random>

// skusit zmenit step size / count...

namespace ds::utils
{
	template<typename Table>
	class TableAnalyzer : public ComplexityAnalyzer<Table>
	{
	protected:
		TableAnalyzer(const std::string& name);

		size_t getRandomKey() const { return key_; }
		int getRandomData() const { return data_; }

	private:
		void insertNElements(Table& table, size_t n);

	protected:
		size_t key_;
		size_t index_;
		int data_;
		std::default_random_engine rngKey_;
		std::default_random_engine rngIndex_;
		std::default_random_engine rngData_;
	};

	template<typename Table>
	class TableInsertAnalyzer : public TableAnalyzer<Table>
	{
	public:
		TableInsertAnalyzer(const std::string& name);

		void beforeOperation(Table& table) override
		{
			key_ = rngKey_();
			data_ = rngData_();
		};

		void executeOperation(Table& table) override
		{
			table.insert(this->getRandomKey(), this->getRandomData());
		}
	};

	template<typename Table>
	class TableFindAnalyzer : public TableAnalyzer<Table>
	{
	public:
		TableFindAnalyzer(const std::string& name);

		void beforeOperation(Table& table) override
		{
			ds::amt::ImplicitSequence<size_t> validKeys;
			for (auto tabElement : table)
			{
				validKeys.insertLast().data_ = tabElement.key_;
			}

			std::uniform_int_distribution<size_t> indexDist(0, validKeys.size() - 1);
			index_ = indexDist(this->rngIndex_);
			key_ = validKeys.access(index_)->data_;
		}

		void executeOperation(Table& table) override
		{
			table.find(this->getRandomKey());
		}
	};

	class TablesAnalyzer : public CompositeAnalyzer
	{
	public:
		TablesAnalyzer() :
			CompositeAnalyzer("Tables")
		{
			this->addAnalyzer(std::make_unique<TableInsertAnalyzer<ds::adt::ModifiedTreap<int, ds::adt::ImplicitList<int>, int>>>("modified-treap-insert"));
			this->addAnalyzer(std::make_unique<TableFindAnalyzer<ds::adt::ModifiedTreap<int, ds::adt::ImplicitList<int>, int>>>("modified-treap-find"));
		}
	};

	//--------- impl

	template<typename Table>
	TableAnalyzer<Table>::TableAnalyzer(const std::string& name) :
	ComplexityAnalyzer<Table>
		(
			name,
			[&](Table& table, size_t n) {
				this->insertNElements(table, n);
			}
		),
		rngKey_(std::random_device()()),
		rngIndex_(std::random_device()()),
		rngData_(std::random_device()()),
		key_(0),
		index_(0),
		data_(0)
	{
	}

	template<typename Table>
	void TableAnalyzer<Table>::insertNElements(Table& table, size_t n)
	{
		for (size_t i = 0; i < n; i++)
		{
			size_t key = rngKey_();
			table.insert(key, rngData_());
			// validKeys_.insertLast().data_ = key;
		}
	}

	template<typename Table>
	TableInsertAnalyzer<Table>::TableInsertAnalyzer(const std::string& name) :
		TableAnalyzer<Table>(name)
	{
	}

	template<typename Table>
	TableFindAnalyzer<Table>::TableFindAnalyzer(const std::string& name) :
		TableAnalyzer<Table>(name)
	{
	}
}