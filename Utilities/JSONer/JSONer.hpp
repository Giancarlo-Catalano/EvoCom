//
// Created by gian on 27/12/22.
//

#ifndef EVOCOM_JSONER_HPP
#define EVOCOM_JSONER_HPP

#include <sstream>
#include <stack>
#include <iomanip>
#include "../utilities.hpp"

namespace GC {

    class JSONer {
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
            if (!isFirstItem())
                ss<<", ";
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


        template <class T>
        void pushVar(const std::string varName, T varValue) {
            addCommaIfNecessary();
            pushValue(varName); //will push the string
            ss<<": ";
            pushValue(varValue);
            firstItemWasAdded();
        }

        JSONer(const std::string className):
        ss(){
            increaseNesting();
            pushValue(className); //will push the string
            ss<<": {";
        }

        std::string end() {
            ss<<"}";
            return ss.str();
        }

    };

} // GC

#endif //EVOCOM_JSONER_HPP
