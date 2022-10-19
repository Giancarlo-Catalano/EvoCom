//
// Created by gian on 17/08/22.
//

#ifndef EVOCOM_HUFFMANCODER_HPP
#define EVOCOM_HUFFMANCODER_HPP


#include <sstream>
#include <queue>
#include <array>
#include <memory>
#include <variant>
#include <map>
#include <algorithm>
#include <stack>
#include <functional>

/**
 *      The Huffman coder is used for encoding a set of symbol with a given probability for each of them.
 *      Declare a huffmanCoder:
*    HuffmanCoder hc(setOfSymbols): where setOfSymbols is a vector of pairs <Symbol, Frequency>.
 *
 *      Get an encoder:
*    HuffmanCoder::Encoder enc = hc.getEncoder(handler); where handler::bool -> void, what is to be done with an encoded bit
 *   for_each(list.begin(), list.end(), [&](const Symbol& symbol){enc.encode(symbol);}
 *
 *
 *      Get a decoder:
*    HuffmanCoder::Decoder dec = hc.getDecoder(handler, requester); where handler:: Symbol -> void, requester:: void -> bool
*    dec.decodeAmountofSymbols(symbolAmount);
 */

namespace GC {
    template <class Data>
    class BinaryTree;

    template <class Data>
    class BinaryTree {
    private:
        using Branch = BinaryTree<Data>;
        using BranchRef = std::shared_ptr<Branch>;
        using Weight = std::size_t;

        struct PairOfBranches {BranchRef leftBranch, rightBranch;};

        Weight weight;
        std::variant<Data,PairOfBranches> contents;

    public:
        BinaryTree() : weight(0), contents() {};
        BinaryTree(const Weight _weight, const Data& _data): weight(_weight), contents(_data){};
        BinaryTree(const Weight _weight, const Branch& leftBranch, const Branch& rightBranch) :
                weight(_weight)
        {
            auto leftRef = std::make_shared<Branch>(leftBranch);
            auto rightRef = std::make_shared<Branch>(rightBranch);
            contents = PairOfBranches{leftRef, rightRef};
        }
        const bool isLeaf() const {std::holds_alternative<Data>(contents);}
        const Weight& getWeight() const {return weight;}
        const Branch& getLeftBranch() const {return *std::get<PairOfBranches>(contents).leftBranch;}
        const Branch& getRightBranch() const {return *std::get<PairOfBranches>(contents).rightBranch;}
        const Data& getData() const {return std::get<Data>(contents);}

        std::string to_string() const {
            std::stringstream ss;
            if (isLeaf())
                ss << ((size_t)getData());
            else
                ss << "W=" << getWeight() << ", {"<<getLeftBranch().to_string()<<", "<<getRightBranch().to_string()<<"}";
            return ss.str();
        };

        friend bool operator==(const BinaryTree<Data>& a, const BinaryTree<Data>& b) {
            if (a.isLeaf() != b.isLeaf())
                return false;
            if (a.isLeaf())
                return a.getData() == b.getData();

            //if they are both branches
            if (a.getLeftBranch() != b.getLeftBranch())
                return false;

            return (a.getRightBranch() == b.getLeftBranch());
        }

        friend bool operator>(const BinaryTree<Data>& a, const BinaryTree<Data>& b) {
            return a.getWeight() > b.getWeight();
        }
    };

    template <class Symbol, class Weight>
    class HuffmanCoder {
        using Tree = BinaryTree<Symbol>;
        using Trees = std::priority_queue<Tree, std::vector<Tree>, std::greater<Tree>>;
        using BitVector = std::vector<bool>;
        using Symbols = std::vector<Symbol>;
        using SymbolWithWeight = std::pair<Symbol,Weight>;

        Symbols symbols;
        const std::size_t symbolAmount;
        Tree decoderTree;
        std::map<Symbol, BitVector> encoderMap;


    public:
        HuffmanCoder(const std::vector<SymbolWithWeight>& symbolsAndWeights) :
                symbolAmount(symbolsAndWeights.size()),
                symbols()
        {
            storeSymbols(symbolsAndWeights);
            initializeDecoderTree(symbolsAndWeights);
            initializeEncoderMap(decoderTree);
        }

        std::string to_string() const {
            std::stringstream ss;
            ss<<"{tree = "<<(decoderTree.to_string())<<"; ";
            ss<<"encoderMap = {";
            auto dumpBitVector = [&](BitVector& bv) { std::for_each(bv.begin(), bv.end(), [&](bool b){ss << b;}); };
            auto dumpPair = [&](Symbol key, BitVector value) {ss<<"["<<((size_t)key)<<"]->"; dumpBitVector(value);};
            bool isFirst = true;
            for (auto&[key, value]: encoderMap) {
                if (!isFirst)
                    ss << ", ";
                isFirst = false;
                dumpPair(key, value);
            }
            ss<<"}}";
            return ss.str();
        }

        void storeSymbols(const std::vector<SymbolWithWeight>& symbolsAndWeights) {
            for (const auto& item: symbolsAndWeights)
                symbols.push_back(item.first);

        }

        void initializeDecoderTree(const std::vector<SymbolWithWeight>& symbolsAndWeights) {
            Trees trees;
            auto addTree = [&](const Tree& tree) {trees.push(tree);};
            auto addTrivialTree = [&](Weight w, Symbol& s) { addTree(Tree(w, s)); };
            auto popSmallest = [&]() -> Tree {
                auto smallest = trees.top();
                trees.pop();
                return smallest;
            };
            auto makeTreeOfSmallest2 = [&]() {
                auto smallest = popSmallest();
                auto secondSmallest = popSmallest();
                Tree newTree(smallest.getWeight() + secondSmallest.getWeight(), smallest, secondSmallest);
                addTree(newTree);
            };

            for (auto item : symbolsAndWeights)
                addTrivialTree(item.second, item.first);

            while (trees.size() > 1)
                makeTreeOfSmallest2();

            decoderTree = trees.top();
        }

        void initializeEncoderMap(const Tree& tree) {
            std::stack<bool> path;
            std::stack<Tree> treeStack; //I know, copies...
            int visitedCount = 0;

            auto pathIntoBitVector = [](std::stack<bool> localPath) -> BitVector {
                BitVector result;
                while (!localPath.empty()) {
                    result.push_back(localPath.top());
                    localPath.pop();
                }
                std::reverse(result.begin(), result.end());
                return result;
            };

            auto lastTree = [&]() -> Tree& {return(treeStack.top());};
            auto lastTreeIsLeaf = [&]() -> bool { return lastTree().isLeaf(); };
            auto pushItemIntoMap = [&](int leafData) {encoderMap[leafData] = pathIntoBitVector(path);};
            auto backtrack = [&]() {path.pop(); treeStack.pop();};
            auto visitedSecondChild = [&]() {return path.top();};
            auto visitFirstChild = [&]() {path.push(0);treeStack.push(lastTree().getLeftBranch());};
            auto visitSecondChild = [&]() {backtrack();path.push(1);treeStack.push(lastTree().getRightBranch());};

            treeStack.push(tree);
            while (true) { //there is a break condition inside!!
                if (lastTreeIsLeaf()) {
                    pushItemIntoMap(lastTree().getData());
                    visitedCount++;
                    if (visitedCount == symbolAmount) break;
                    while (visitedSecondChild())
                        backtrack();
                    visitSecondChild();
                }
                else
                    visitFirstChild();
            }
        }


    public:
        struct Encoder {
            using EncoderMap = std::map<Symbol, BitVector>;
            using Handler = std::function<void(const std::vector<bool>&)>;
            const EncoderMap& map;
            const Handler& handler;

            Encoder(const EncoderMap& _map, const Handler& _handler) : map(_map), handler(_handler){}
            void encodeSymbol(const Symbol& symbol) const {
                BitVector bitVector = map.at(symbol);
                handler(bitVector);
            }

            template <class Container>  //TODO test this
            void encodeAll(const Container& container) const {
                std::for_each(container.begin(), container.end(), [&](auto s){encodeSymbol(s);});
            }
        };

        struct Decoder {
            using DecoderTree = BinaryTree<Symbol>;
            using Handler = std::function<void(const Symbol&)>;
            using Requester = std::function<bool(void)>; //TODO find a better name for the requester
            const DecoderTree& tree;
            BinaryTree<Symbol> currentTree;
            const Handler handler;
            const Requester requester;


            void resetCurrentTree() {currentTree = tree; }

            Decoder(const DecoderTree& _tree, const Handler& _handler, const Requester& _requester) :
                tree(_tree),
                handler(_handler),
                requester(_requester)
                { resetCurrentTree();}

            bool decodeBit(bool b) { //returns true if it pushed a new symbol
                currentTree = b ? currentTree.getRightBranch() : currentTree.getLeftBranch();
                if (currentTree.isLeaf()) {
                    handler(currentTree.getData());
                    resetCurrentTree();
                    return true;
                }
                return false;
            }

            void decodeAmountOfSymbols(std::size_t amount) {
                std::size_t decodedSoFar = 0;
                while (decodedSoFar< amount)
                    decodedSoFar += decodeBit(requester());
            }

            void decodeAmountOfBits(std::size_t amount) {
                for (std::size_t i = 0; i< amount; i++)
                    decodeBit(requester());
            }

        };

        Encoder getEncoder(const typename Encoder::Handler handler) const {
            return Encoder(encoderMap, handler);
        }

        Decoder getDecoder(const std::function<void(const Symbol&)> handler, const std::function<bool(void)> requester) const {
            return Decoder(decoderTree, handler, requester);
        }
    };
}

#endif //EVOCOM_HUFFMANCODER_HPP
