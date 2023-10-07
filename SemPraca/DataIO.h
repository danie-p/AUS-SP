#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>

template <typename DataType>
class DataIO
{
public:
    void dataInput(std::string unitName, typename std::function<void(DataType*)> insert);
};

template <typename DataType>
void DataIO<DataType>::dataInput(std::string unitType, typename std::function<void(DataType*)> insert)
{
    std::ifstream input{ unitType + ".csv" };
    std::string line{};

    std::ifstream inputExtra{ unitType + "_extra.csv" };
    std::string lineExtra{};

    if (!input || !inputExtra)
    {
        throw std::runtime_error("Chyba pri èítaní zo súboru!");
    }

    std::getline(input, line);                      // pri nacitavani sa 1. riadok (nadpisy) vynecha
    std::getline(inputExtra, lineExtra);

    while (std::getline(input, line) && std::getline(inputExtra, lineExtra))
    {
        std::istringstream ssline(line);
        std::string word{};
        std::vector<std::string> words{};

        std::istringstream sslineExtra(lineExtra);
        std::string wordExtra{};
        std::vector<std::string> wordsExtra{};

        while (std::getline(ssline, word, ';'))
        {
            words.push_back(word);
        }

        while (std::getline(sslineExtra, wordExtra, ';'))
        {
            wordsExtra.push_back(wordExtra);
        }

        std::stringstream ssnum(words[0]);
        size_t num{};
        ssnum >> num;

        std::stringstream ssnumExtra(wordsExtra[0]);
        size_t numExtra{};
        ssnumExtra >> numExtra;

        DataType* dataUnit = new DataType(num, words[1], words[2], words[3], words[4], words.size() < 6 ? "" : words[5], numExtra, wordsExtra.size() < 2 ? "" : wordsExtra[1]);

        insert(dataUnit);
    }
}