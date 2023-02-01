//
// Created by gian on 29/01/23.
//

#ifndef EVOCOM_JSON2CSV_HPP
#define EVOCOM_JSON2CSV_HPP
#include "../../Dependencies/nlohmann/json.hpp"
#include "../../Evolver/Individual/CCodes.hpp"
#include "../../Evolver/Individual/TCodes.hpp"
#include "../../EvolutionaryFileCompressor/EvoCompressorSettings/EvoComSettings.hpp"
#include "../../BlockReport/BlockReport.hpp"
#include <unordered_set>

#include <fstream>
using json = nlohmann::json;

namespace GC {

    class JSON2CSV {

    public: //headers and similar
        using Headers = std::vector<std::string>;
        const Headers dataOriginHeaders = { "SourceFile", "FileExtension", "DataCategory"};
        const Headers AlgorithmSettingsHeaders = {"SegmMethod", "Generations", "PopulationSize", "MutationRate", "CompCrossoverRate", "Annealing", "EliteSize", "TournamentSize", "MutationThreshold", "VariatonThreshold", "Asynch"};
        const Headers BlockReportShortHeaders = {"Length", "Entropy"};
        const Headers BlockReportLongHeaders = {"Length", "Entropy", "Unit_Average", "Unit_StdDev", "Unit_Min", "Unit_FirstQ", "Unit_Median", "Unit_"}; //TODO continue this
        const Headers TransformHeaders = {"Transform"};
        const Headers CompressionHeaders = {"Compression"};


    public: //types
        //using CCode, TCode
        using DataCategory = std::string;
        using FileName = std::string;
        using CSVStream = std::ofstream;
        using FileExtension = std::string;
        using SegmentationMethod = EvoComSettings::SegmentationMethod;
        static const char separator = '\t';
        static const char fileSlash = '\\';
        static const char fileExtensionDot = '.';

        struct SourceInfo {
            FileName sourceFile;
            FileExtension fileExtension;
            DataCategory dataCategory;

            DataCategory getDataCategory(const FileName& fileName) {
                using ExtensionSet = std::unordered_set<FileExtension>;
                const ExtensionSet imageExtensions = {"jpg", "jpeg", "png", "tiff", "gif", "raw"};
                const ExtensionSet videoExtensions = {"mp4"};
                const ExtensionSet audioExtensions = {"mp3", "wav"};
                const ExtensionSet textExtensions  = {"txt", "json", "html", "cpp", "hpp"};
                const ExtensionSet documentExtensions = {"pdf", "pptx", "docx"};

                const FileExtension fileExtension = getFileExtension(fileName);
                auto extensionIsInSet = [&fileExtension](const ExtensionSet& set) {
                    return set.count(fileExtension) > 0;
                };

                if      (extensionIsInSet(imageExtensions)) return "image";
                else if (extensionIsInSet(videoExtensions)) return "video";
                else if (extensionIsInSet(audioExtensions)) return "audio";
                else if (extensionIsInSet(textExtensions))  return "text";
                else if (extensionIsInSet(documentExtensions)) return "doc";
                else return "other";
            }

            SourceInfo(const FileName& filePath) :
            sourceFile(getJustFileName(filePath)),
            fileExtension(getFileExtension(filePath)){
                dataCategory = getDataCategory(fileExtension);
            }

            void fromJSON(const json& fileItem) {
                *this = SourceInfo(fileItem.at("fileName"));
            }
        };

        struct CompressionAlgoSettings {
            SegmentationMethod segmentationMethod;
            size_t generationCount;
            size_t populationSize;
            double mutationRate;
            double crossoverRate;
            bool usesAnnealing;
            size_t eliteSize;
            size_t tournamentSelectionSize;
            double excessiveMutationThreshold;
            double excessiveVariationThreshold;
            bool isAsynch;

            void fromJSON(const json& root) {
                const json settings = root.at("Settings");
                segmentationMethod = settings.at("segmentationMethod");
                generationCount = settings.at("generations");
                populationSize = settings.at("populationSize");
                mutationRate = settings.at("mutationRate");
                crossoverRate = settings.at("compressionCrossoverRate");
                usesAnnealing = settings.at("usesAnnealing");
                eliteSize = settings.at("eliteSize");
                tournamentSelectionSize = settings.at("tournamentSelectionSize");
                excessiveMutationThreshold = settings.at("excessiveMutationThreshold");
                excessiveVariationThreshold = settings.at("unstabilityThreshold");
                isAsynch = settings.at("isAsynch");
            }
        };

        struct BlockReportInfo {
            BlockReport br;
        };



    private:

        json getJSONFromFile(const FileName fileName) {
            std::ifstream readingStream(fileName);
            return json::parse(readingStream);
        }

        static std::string getWhatsAfterTheLastOf(const std::string& input, const char delimiter) {
            const size_t position = input.find_last_of(delimiter);
            return input.substr(position+1, input.size());
        }

        static FileName getJustFileName(const std::string& filePath) {
            return getWhatsAfterTheLastOf(filePath, fileSlash);
        }

        static FileExtension getFileExtension(const FileName& fileName) {
            return getWhatsAfterTheLastOf(fileName, fileExtensionDot); //TODO it should force it to be lowercase
        }


        using BlockInitialState = BlockReport;

#define pushIntoCSV(item) csv<<item<<separator;

        void addFullBlockReportInfo(const BlockReport& br, CSVStream& csv) {
            const auto addStatisticalFeatures = [&](const StatisticalFeatures& features) {
                pushIntoCSV(features.average);
                pushIntoCSV(features.standardDeviation);
                pushIntoCSV(features.minimum);
                pushIntoCSV(features.firstQuantile);
                pushIntoCSV(features.median);
                pushIntoCSV(features.thirdQuantile);
                pushIntoCSV(features.maximum);
            };
            pushIntoCSV(br.size);
            pushIntoCSV(br.entropy);
            addStatisticalFeatures(br.unitFeatures);
            addStatisticalFeatures(br.deltaFeatures);
            addStatisticalFeatures(br.frequencyFeatures);
        };

        void addShortBlockReportInfo(const BlockReport& br, CSVStream & csv) {
            pushIntoCSV(br.size);
            pushIntoCSV(br.entropy);
        }

        struct BlockIntermediateState {
            BlockReport report;
            std::string transform;
        };

        struct BlockFinalState {
            BlockReport report;
            std::string compression;
            size_t finalSizeInBits;
        };

        struct BeforeAndAfterTransforms {
            BlockInitialState before;
            BlockFinalState after;
        };

        struct WhatOperationComesNext {
            BlockReport state;
            std::string operation;
        };

        std::string to_string(const SegmentationMethod segm) {
            return (segm ==  EvoComSettings::Fixed ? "Fixed" : "Clustered");
        }

        void pushItemIntoCSV(const std::string& str, CSVStream& csv) {
            csv<<str<<separator;
        }

        void finishRow(CSVStream& csv) {
            csv<<"\n";
        }
    };

} // GC

#endif //EVOCOM_JSON2CSV_HPP
