//
// Created by gian on 27/12/22.
//

#ifndef EVOCOM_LOGGER_HPP
#define EVOCOM_LOGGER_HPP

#include <sstream>
#include <stack>
#include <iomanip>
#include "../utilities.hpp"

namespace GC {

    class Logger {
    private:
        std::stringstream ss;
        std::stack<bool> nestingStack; //the values determine if we're currently in the first item of an object

        void pushValue(std::string str) {
            ss<<"\""<<str<<"\"";
        }

        void pushValue(const char* str) {
            ss<<"\""<<str<<"\"";
        }

        void pushValue(size_t val) {
            ss<<val;
        }

        void pushValue(int val) {
            ss<<val;
        }

        void pushValue(double val) {
            ss<<std::setprecision(3)<<val;
        }

        void pushValue(bool val) {
            ss<<(val ? "true" : "false");
        }

        void increaseNesting() {
            nestingStack.push(true);
        }

        bool isFirstItem() {
            return nestingStack.top();
        }

        void decreaseNesting() {
            nestingStack.pop();
        }

        void firstItemWasAdded() {
            nestingStack.top() = false;
        }

        void addCommaIfNecessary() {
            if (!isFirstItem()) {
                ss << ", ";
            }

            newLine();
            indent();
        }

        void indent() {
            repeat(nestingStack.size(), [&](){ss<<"\t";});
        }

        void newLine() {
            ss<<"\n";
        }

    public:

        void beginObject(const std::string& varName) {
            addCommaIfNecessary();
            pushValue(varName);
            ss<<": {";
            increaseNesting();
        }

        void endObject() {
            ss<<"}";
            decreaseNesting();
            firstItemWasAdded();
        }

        void beginList(const std::string& varName) {
            addCommaIfNecessary();
            pushValue(varName);
            ss<<": [";
            increaseNesting();
        }

        void endList() {
            ss<<"]";
            decreaseNesting();
            firstItemWasAdded();
        }

        template <class T>
        void addListItem(T varValue) {
            addCommaIfNecessary();
            pushValue(varValue);
            firstItemWasAdded();
        }



        template <class T>
        void addVar(const std::string varName, T varValue) {
            addCommaIfNecessary();
            pushValue(varName); //will push the string
            ss<<": ";
            pushValue(varValue);
            firstItemWasAdded();
        }

        Logger():
        ss(){
            increaseNesting();
            ss<<"{";
        }

        std::string end() {
            ss<<"}";
            return ss.str();
        }

    };

} // GC

#endif //EVOCOM_LOGGER_HPP
