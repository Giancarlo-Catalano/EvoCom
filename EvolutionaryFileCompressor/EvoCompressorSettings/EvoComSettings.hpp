//
// Created by gian on 25/11/22.
//

#ifndef EVOCOM_EVOCOMSETTINGS_HPP
#define EVOCOM_EVOCOMSETTINGS_HPP

#include <map>
#include <fstream>
#include "../../Utilities/utilities.hpp"
#include "../../Utilities/Logger/Logger.hpp"
#include <sstream>

namespace GC {

    class EvoComSettings {
    public:
        using Dictionary = std::unordered_map<std::string, std::string>;
        using FileName = std::string;
        using Param = std::string;

    public:
        enum Mode {Compress, Decompress, CompressionDataCollection, DecompressionDataCollection, EvolverConvergenceDataCollection} mode;
        FileName inputFile;
        FileName configFile;
        FileName testSetFile;
        std::vector<FileName> testSet;

        enum SegmentationMethod {Fixed, Clustered} segmentationMethod;
        size_t fixedSegmentSize;
        double clusteredSegmentThreshold;
        size_t clusteredSegmentCooldown;

        size_t generations;
        size_t population;
        double mutationRate;
        double compressionCrossoverRate;
        bool usesAnnealing;
        size_t eliteSize;

        size_t tournamentSelectionSize;
        double excessiveMutationThreshold;
        double unstabilityThreshold; //unused

        size_t minTransformAmount, maxTransformAmount;

        bool async;



    private:
        Dictionary getEmptyDictionary() {return Dictionary();}

        Dictionary makeDictionaryFromFile(const FileName& configFile) {
            std::ifstream inStream(configFile);
            if (!inStream) {
                LOG_NOSPACES("The config file \"", configFile, "\" could not be opened, aborting");
                return Dictionary();
            }

            Dictionary dict;
            while (inStream) {
                std::string parameterName;
                std::string parameterValue;
                inStream >> parameterName;
                inStream >> parameterValue;
                toUpper(parameterName);
                dict[parameterName] = parameterValue;
            }

            return dict;
        }

        Dictionary makeDictionaryFromCommandLineArguments(const int argc, char** argv) {
            Dictionary dict;
            for (size_t i=1;i<argc;i+=2) {
                std::string parameterWithDash = (argv[i]);
                if (parameterWithDash.size() > 0 && parameterWithDash[0]=='-') {
                    std::string parameter = parameterWithDash.substr(1, parameterWithDash.size()-1);
                    toUpper(parameter);
                    dict[parameter] = argv[i+1];
                }
                else
                    LOG_NOSPACES("The parameter \"", parameterWithDash, "\" was not recognised");
            }

            return dict;
        }


        template <class ValueType>
        static bool containsParam(const std::unordered_map<Param, ValueType>& dict, const Param& param) {
            return dict.count(param) > 0;
        }

        static std::string getStringFromDict(const Dictionary& dict, const Param& param, const std::string def) {
            if (containsParam(dict, param)) return dict.at(param);
            return def;
        };

        template <class EnumT>
        EnumT getEnumFromDict(const Dictionary& dict, const Param& param, const std::unordered_map<std::string, EnumT>& mapping, const EnumT def) {
            if (containsParam(dict, param)) {
                std::string stringValue = dict.at(param);
                if (containsParam(mapping, stringValue)) {
                    return mapping.at(stringValue);
                }
            }
            return def;
        }

        int getIntFromDict(const Dictionary& dict, const Param& param, const int def) {
            if (containsParam(dict, param)) {
                return std::stoi(dict.at(param));
            }
            return def;
        }

        double getDoubleFromDict(const Dictionary& dict, const Param& param, const double def) {
            if (containsParam(dict, param)) {
                return std::stod(dict.at(param));
            }
            return def;
        }

        bool getBoolFromDict(const Dictionary& dict, const Param& param, bool def) {
            if (containsParam(dict, param)) {
                std::string valueAsString = dict.at(param);
                if (valueAsString == "true") return true;
                else if (valueAsString == "false") return false;
            }
            return def;
        }

        static void toUpper(std::string& s) {
            std::transform(s.begin(), s.end(), s.begin(), std::ptr_fun<int, int>(std::toupper));
        }

        EvoComSettings(const Dictionary& dict){
            mode = getEnumFromDict<Mode>(dict, "MODE", {{"compress",                         Compress},
                                                        {"decompress",                       Decompress},
                                                        {"compressionDataCollection",        CompressionDataCollection},
                                                        {"decompressionDataCollection",      DecompressionDataCollection},
                                                        {"evolverConvergenceDataCollection", EvolverConvergenceDataCollection}}, Compress);
            configFile = getStringFromDict(dict, "CONFIG", "../build/simple.config");

            if (mode == Compress || mode == CompressionDataCollection || mode == EvolverConvergenceDataCollection) {

                if (mode == CompressionDataCollection) {
                    testSetFile = getStringFromDict(dict, "TESTSET", "../build/testset.txt");
                    testSet = getLinesFromFile(testSetFile);
                }
                else if (mode == Compress || mode == EvolverConvergenceDataCollection)
                    inputFile = getStringFromDict(dict, "FILE", "input");

                segmentationMethod = getEnumFromDict(dict, "SEGMENT_TYPE", {{"fixed", Fixed}, {"clustered", Clustered}}, Fixed);
                if (segmentationMethod == Fixed) {
                    fixedSegmentSize = getIntFromDict(dict, "FIXED_SEGMENT_SIZE", 256);
                }
                else {
                    clusteredSegmentCooldown = getIntFromDict(dict, "CLUSTERED_SEGMENT_COOLDOWN", 2);
                    clusteredSegmentThreshold = getDoubleFromDict(dict, "CLUSTERED_SEGMENT_THRESHOLD", 0.1);
                }

                generations = getIntFromDict(dict, "GENERATIONS", 36);
                population = getIntFromDict(dict, "POPULATION", 36);
                mutationRate = getDoubleFromDict(dict, "MUTATION_RATE", 0.1);
                compressionCrossoverRate = getDoubleFromDict(dict, "COMPRESSION_CROSSOVER_RATE", 0.3);
                usesAnnealing = getBoolFromDict(dict, "USES_ANNEALING", true);
                eliteSize = getIntFromDict(dict, "ELITE_SIZE", 3);

                tournamentSelectionSize = getIntFromDict(dict, "TOURNAMENT_SELECTION_SIZE", 1);
                excessiveMutationThreshold = getDoubleFromDict(dict, "EXCESSIVE_MUTATION_THRESHOLD", 0.75);
                unstabilityThreshold = getDoubleFromDict(dict, "UNSTABILITY_THRESHOLD", 0.4);

                minTransformAmount = getIntFromDict(dict, "MIN_TRANSFORM_AMOUNT", 0);
                maxTransformAmount = getIntFromDict(dict, "MAX_TRANSFORM_AMOUNT", 6);

                async = getBoolFromDict(dict, "ASYNC", true);
            }
            else if (mode == Decompress) {
                inputFile = getStringFromDict(dict, "FILE", "input");
            }


        }

        //top overwrites bottom
        Dictionary overwriteDictionary(const Dictionary& top, const Dictionary& bottom) {
            Dictionary result = top;
            for (const auto& [key, value] : bottom) {
                if (!containsParam(result, key)) {
                    result[key] = value;
                }
            }
            return result;
        }

        static void logDict(const Dictionary& dict) {
            for (const auto& [key, value] : dict) {
                LOG("[", key, "] = ", value);
            }

            LOG("---------End of dictionary-----");
        }


    public:
        EvoComSettings(int argc, char** argv) {
            Dictionary commandLineSettings = makeDictionaryFromCommandLineArguments(argc, argv);

            FileName configFile = getStringFromDict(commandLineSettings, "CONFIG", "");
            if (configFile.empty()) {
                *this = EvoComSettings(commandLineSettings);
            }
            else {
                Dictionary configFileSettings = makeDictionaryFromFile(configFile);
                Dictionary overwrittenSettings = overwriteDictionary(commandLineSettings, configFileSettings);
                *this = EvoComSettings(overwrittenSettings);
            }
        }

        std::string to_string() const {
            Logger logger;
            log(logger);
            return logger.end();
        }

        void log(Logger& logger) const {
            logger.beginObject("Settings");
            const std::map<EvoComSettings::Mode, std::string> modesAsStrings{{Compress, "Compress"}, {Decompress, "Decompress"}, {CompressionDataCollection, "CompressionDataCollection"}, {DecompressionDataCollection, "DecompressionDataCollection"}, {EvolverConvergenceDataCollection, "EvolverConvergenceDataCollection"}};

            logger.addVar("mode", modesAsStrings.at(mode));
            logger.addVar("configFile", configFile);

            if ((mode == Compress) || (mode == CompressionDataCollection) || (mode==EvolverConvergenceDataCollection) ) {
                if ((mode == Compress) || (mode == EvolverConvergenceDataCollection))
                    logger.addVar("inputFile", inputFile);
                else if (mode == CompressionDataCollection) {
                    logger.addVar("testSetFile", testSetFile);
                    logger.beginList("testSet");
                    std::for_each(testSet.begin(), testSet.end(), [&](auto testFile) {logger.addListItem(testFile);});
                    logger.endList();
                }
                if (segmentationMethod == Fixed) {
                    logger.addVar("segmentationMethod", "fixed");
                    logger.addVar("segmentSize", fixedSegmentSize);
                } else {
                    logger.addVar("segmentationMethod", "clustered");
                    logger.addVar("clusteringThreshold", clusteredSegmentThreshold);
                    logger.addVar("clusteringPardonCooldown", clusteredSegmentCooldown);
                }

                logger.addVar("generations", generations);
                logger.addVar("populationSize", population);
                logger.addVar("mutationRate", mutationRate);
                logger.addVar("compressionCrossoverRate", compressionCrossoverRate);
                logger.addVar("usesAnnealing", usesAnnealing);
                logger.addVar("eliteSize", eliteSize);
                logger.addVar("tournamentSelectionSize", tournamentSelectionSize);
                logger.addVar("excessiveMutationThreshold", excessiveMutationThreshold);
                logger.addVar("asynchronous", async);
            }
            logger.endObject(); //ends settings
        }
    };

} // GC

#endif //EVOCOM_EVOCOMSETTINGS_HPP
