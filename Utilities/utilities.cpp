//
// Created by gian on 15/09/22.
//
#ifndef DISS_SIMPLEPROTOTYPE_UTILITIES_CPP
#define DISS_SIMPLEPROTOTYPE_UTILITIES_CPP


#include "utilities.hpp"
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sstream>


#endif //DISS_SIMPLEPROTOTYPE_UTILITIES_CPP

std::string showChar(const Byte &byte) {
    auto isPrintable = [](const Byte &c) {
        return (c >= ' ' && c <= '~');
    };

    auto charToString = [](const Byte &c) {
        return std::string(1, c);
    };

    auto unprintableToString = [&](const Byte &c) {
        switch (c) {
            case '\n':
                return "\\n";
            case '\t':
                return "\\t";
            default :
                return "[?]";
        }
    };

    if (isPrintable(byte)) return charToString(byte);
    else return unprintableToString(byte);
}

void printNewLine() {
    std::cout<<"\n";
}

//Note that his crashes if the input is 0
size_t floor_log2(const size_t input) {  //this can be definitely optimised
    if (input == 0) return 0;
    size_t pos = sizeof(decltype(input))*8-1;
    for (size_t tester = 1ULL<<pos; tester != 0; tester>>=1) {
        if (tester & input) return pos;
        pos--;
    }
}

size_t ceil_log2(const size_t input) {
    if (input == 0) return 0;
    return floor_log2(input-1)+1;
}


std::string getErrorMessage() {
    //uses errno
    return std::strerror(errno);
}

template <class T>
bool getBitAt(const T item, const size_t pos) {  //pos is 0 indexed from the right
    return (item>>pos)&1;
}

template <class T>
std::string toBinaryString(const T item) {
    const size_t amountOfBits = bitsInType<T>();
    std::stringstream ss;
    for (int i=amountOfBits-1;i>=0;i--)
        ss << getBitAt(item, i);
    return ss.str();
}

void dumpFile(const std::string& fileName) {
    std::ifstream inStream(fileName, std::ios_base::binary);
    if (!inStream) {
        LOG("There was an error opening the file (for reading)! ", getErrorMessage());
    }

    std::size_t unitCount = 0;
    std::size_t unitsPerLine = 10;

    size_t inCurrentLine = 0;
    while (inStream) {
        Byte tempByte = inStream.get();
        if (inStream.eof()) {
            LOG("\nRead", unitCount, "units");
            break;
        }
        LOG_NONEWLINE_NOSPACES("[",toBinaryString(tempByte), "]");
        if (inCurrentLine >= unitsPerLine-1) {
            LOG("");
            inCurrentLine = 0;
        }
        else
            inCurrentLine++;
        unitCount++;
    }
}




size_t getFileSize(const std::string &fileName) {
    std::ifstream in(fileName, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

//if divisor is 0, this breaks
size_t ceil_div(const size_t input, const size_t divisor) {
    return (input+divisor-1)/divisor;
}

size_t greaterMultipleOf(const size_t input, const size_t multipleOf) {
    if (input == 0)
        return 0;
    else
        return (((input-1)/multipleOf)+1)*multipleOf;
}

std::vector<std::string> getLinesFromFile(const std::string &fileName) {
    std::ifstream inputFile(fileName);
    if (!inputFile)  {
        return {};
    }

    std::string temp;
    std::vector<std::string> lines;

    auto isUnwantedCharacter = [&](const char c) { return (c==' ') ||  (c=='\r') || (c=='\n'); };
    auto getLastCharater = [&](const std::string& str) {return str.at(str.size()-1);};

    auto cleanLine = [&](const std::string& str) -> std::string {
        std::string temp = str;
        while (isUnwantedCharacter(getLastCharater(temp)))
            temp = temp.substr(0, temp.size()-1);

        return temp;
    };

    while (getline(inputFile, temp))
        lines.push_back(cleanLine(temp));

    return lines;
}


