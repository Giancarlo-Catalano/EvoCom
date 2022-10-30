//
// Created by gian on 15/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_UTILITIES_HPP
#define DISS_SIMPLEPROTOTYPE_UTILITIES_HPP

#include <iostream>
#include <assert.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>
#include "../names.hpp"

#define GC_DEBUG 1


#define GC_NODISCARD [[nodiscard]]


#if GC_DEBUG == 1
#define LOG(...) printMany(true, __VA_ARGS__);printNewLine()
#define LOG_NONEWLINE(...) printMany(true, __VA_ARGS__)
#define LOG_NOSPACES(...) printMany(false, __VA_ARGS__);printNewLine()
#define LOG_NONEWLINE_NOSPACES(...) printMany(false, __VA_ARGS__)
#define ASSERT(var) assert(var)
#define ASSERT_EQUALS(var, expected) assert(var == expected)
#define ASSERT_NOT_EQUALS(var, notexpected) assert(var != notexpected)
#define ASSERT_NOT_EMPTY(var) ASSERT_NOT_EQUALS(var.size(), 0)
#define ASSERT_GREATER(var, min_accepted) ASSERT(var >= min_accepted)
#define ASSERT_LOWER(var, max_accepted) ASSERT(var <= max_accepted)
#define ASSERT_WITHIN(var, min_accepted, max_accepted) ASSERT((var >= min_accepted)&(var >= max_accepted))
#define ASSERT_NOT_REACHED() LOG("This area should be inaccessible!");ASSERT(false);
#define ERROR_NOT_IMPLEMENTED(message) LOG("ERROR! NOT IMPLEMENTED:", message); ASSERT(false);

#define SINGLE_EVAL(M) M
#define EVAL(SOME_WEIRD_MACRO) SINGLE_EVAL(SINGLE_EVAL(SOME_WEIRD_MACRO))
#endif


    template<class T>
    constexpr size_t typeVolume() {
        return 1ULL << (sizeof(T) * 8);
    }

    template <class T>
    constexpr size_t highestUnsignedValue() {
        return typeVolume<T>()-1;
    }

    template <class T>
    constexpr size_t lowestUnsignedValue() {
        return 0;
    }

    template <class T>
    size_t getUnsignedValue(T x) {return static_cast<size_t>(x);}


    template<class C>
    std::string containerToString(const C &c, int maximumToShow = -1) {
        std::stringstream ss;
        ss << "{" << std::setprecision(2) << std::fixed;
        bool isFirst = true;
        size_t shownSoFar = 0;
        if (maximumToShow < 0)
            maximumToShow = c.size();

        for (const auto &item: c) {
            if (!isFirst)
                ss << ", ";

            if (sizeof(item) == 1)
                ss << ((int) item);
            else
                ss << item;

            isFirst = false;
            shownSoFar++;

            if (shownSoFar >= maximumToShow)
                break;
        }
        if (shownSoFar < c.size())
            ss << "...";
        ss << "}";
        return ss.str();
    }

    template<class T>
    constexpr size_t bitsInType() {
        return 8 * sizeof(T);
    }

    std::string showChar(const Byte &byte);


    void printNewLine();


    template<class Printable>
    void printMany(bool withSpaces, const Printable single) { std::cout << single; }

    template<class Printable, typename...Many>
    void printMany(bool withSpaces, const Printable head, const Many... rest) {
        std::cout << head;
        if (withSpaces)
            std::cout << " ";
        printMany(withSpaces, rest...);
    }

    template <class Function>
    void repeat(const size_t amountOfTimes, const Function func) {
        for (size_t i=0;i<amountOfTimes;i++)
            func();
    }

    template <class T>
    T ceil_division(const T a, const T b) {
        if (b==0) return 0;
        if (a%b==0) return a/b;
        else        return (a/b)+1;
    }

    template <class Predicate, class Function>
    void repeatWhileFalse(const Predicate pred, const Function func) {
        while (!pred())
            func();
    }

size_t floor_log2(const size_t input);
size_t ceil_log2(const size_t input);
size_t ceil_div(const size_t input, const size_t divisor);


void dumpFile(const std::string& fileName);

size_t getFileSize(const std::string& fileName);


template <class T>
void concatenate(std::vector<T>& A, const std::vector<T>& B) {
    A.insert(A.end(), B.begin(), B.end());
}





template<class Item, class ScoreFunction>
Item getMinimumBy(const std::vector<Item> &list, const ScoreFunction func) {
    //I just don't like how the standard function works, because that requires
    // forward iterators begin and end, IDGAF about that
    // compare function, not usually what I need
    //plus it gets recalculated over and over, it's very inefficient for my purposes
    ASSERT_NOT_EMPTY(list);

    Item lowestSoFar = list[0];
    auto lowestScore = func(lowestSoFar);
    auto updateLower = [&](const Item newItem) {
        auto newScore = func(newItem);
        if (newScore < lowestScore) {
            lowestSoFar = newItem;
            lowestScore = newScore;
        }
    };

    for (size_t i=1;i<list.size();i++) updateLower(list[i]);

    return lowestSoFar;
}


template <class T>
T square(const T x) {return x*x;}



#endif //DISS_SIMPLEPROTOTYPE_UTILITIES_HPP


