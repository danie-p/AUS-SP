#pragma once
#include <iterator>
#include <functional>

template <typename DataType, typename IteratorType>
class Algorithm
{
public:
    void findAndProcess(IteratorType begin, IteratorType end, typename std::function<bool(DataType&)> predicate, typename std::function<void(DataType&)> process);
};

template<typename DataType, typename IteratorType>
void Algorithm<DataType, IteratorType>::findAndProcess(IteratorType begin, IteratorType end, typename std::function<bool(DataType&)> predicate, typename std::function<void(DataType&)> process)
{
   while (begin != end)
    {
        if (predicate(*begin))
        {
            process(*begin);
        }
        ++begin;
    }
}