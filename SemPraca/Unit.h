#pragma once
#include <iostream>
#include <string>

struct InputCheck
{
    size_t checkInput(size_t& input, const std::string_view& inputRequest, const std::string_view& inputWarning, std::function<bool()> condition)
    {
        std::string inputStr{};
        bool first{ 1 };

        do
        {
            first ? std::cout << inputRequest : std::cout << inputWarning;
            std::getline(std::cin, inputStr);
            if (!inputStr.empty())
            {
                input = inputStr.front() - '0';
            }
            first = 0;
        } while (condition());

        return input;
    }

    char checkInput(char& input, const std::string_view& inputRequest, const std::string_view& inputWarning, std::function<bool()> condition)
    {
        std::string inputStr{};
        bool first{ 1 };

        do
        {
            first ? std::cout << inputRequest : std::cout << inputWarning;
            std::getline(std::cin, inputStr);
            if (!inputStr.empty())
            {
                input = inputStr.front();
            }
            first = 0;
        } while (condition());

        return input;
    }
};

class Unit
{
private:
    size_t sortNumber{};
    std::string code{};
    std::string officialTitle{};
    std::string mediumTitle{};
    std::string shortTitle{};
    std::string note{};
    size_t kindergartenNum{};
    std::string altTitle{};

public:
    Unit(size_t sortNumber, const std::string& code, const std::string& officialTitle, const std::string& mediumTitle, const std::string& shortTitle, const std::string& note, size_t kindergartenNum, const std::string& altTitle) :
        sortNumber(sortNumber), code(code), officialTitle(officialTitle), mediumTitle(mediumTitle), shortTitle(shortTitle), note(note), kindergartenNum(kindergartenNum), altTitle(altTitle) {};

    bool containsStr(const std::string& searched)
        { return this->officialTitle.find(searched) != std::string::npos; };

    bool startsWithStr(const std::string& searched)
        { return this->officialTitle.rfind(searched, 0) == 0; };

    bool hasType(const size_t type);

    size_t& getSortNumber() { return this->sortNumber; };
    std::string& getCode() { return this->code; };
    std::string& getOfficialTitle() { return this->officialTitle; };
    std::string& getMediumTitle() { return this->mediumTitle; };
    std::string& getShortTitle() { return this->shortTitle; };
    std::string& getNote() { return this->note; };
    std::string& getAltTitle() { return this->altTitle; };
    size_t& getKindergartenNum() { return this->kindergartenNum; };

    std::string district();
    size_t vowelsCount();

    friend std::ostream& operator<<(std::ostream& output, const Unit& unit);
};

bool Unit::hasType(const size_t type)
{
    if (type == 1)
    {
        return this->code.length() == 1;
    }
    else if (type == 2)
    {
        return this->code.length() == 5 || (this->code.length() == 6 && this->sortNumber != 2929);
    }
    return this->code.length() == 12 || (this->code.length() == 6 && this->sortNumber == 2929);
}

std::string Unit::district()
{
    return this->code.substr(3, 3);
}

size_t Unit::vowelsCount()
{
    size_t vowelCount{ 0 };
    std::string vowels{ "aáäeéiíoóôuúyýAÁÄEÉIÍOÓÔUÚYÝ" };

    for (auto letter : this->officialTitle)
    {
        if (vowels.find(letter) != std::string::npos)
        {
            ++vowelCount;
        }
    }

    return vowelCount;
}

std::ostream& operator<<(std::ostream& output, const Unit& unit)
{
    output << std::left << std::setw(12) << unit.code << ' ' << std::setw(46) << unit.altTitle << "| mš: " << std::setw(4) << unit.kindergartenNum;
    return output;
}