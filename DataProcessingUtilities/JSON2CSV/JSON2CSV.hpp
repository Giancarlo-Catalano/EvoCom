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

    public: //types

        //using CCode, TCode
        using FileName = std::string;
        using CSVStream = std::ofstream;
        using FileExtension = std::string;
        using SegmentationMethod = EvoComSettings::SegmentationMethod;
        static const char separator = '\t';
        static const char fileSlash = '\\';
        static const char fileExtensionDot = '.';

        enum DataCategory {
            Image,
            Text,
            Video,
            Audio,
            Document,
            Other
        };

        struct SourceInfo {
            FileName sourceFile;
            FileExtension fileExtension;
            DataCategory dataCategory;
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
        };

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

        std::string to_string(const DataCategory cat) {
            switch (cat) {
                case DataCategory::Image : return "image";
                case DataCategory::Video : return "video";
                case DataCategory::Audio : return "audio";
                case DataCategory::Text  : return "text";
                case DataCategory::Document: return "doc";
                default: return "other";
            }
        }

        std::string to_string(const SegmentationMethod segm) {
            return (segm ==  EvoComSettings::Fixed ? "Fixed" : "Clustered");
        }

        void pushItemIntoCSV(const std::string& str, CSVStream& csv) {
            csv<<str<<separator;
        }

        void finishRow(CSVStream& csv) {
            csv<<"\n";
        }

        void dumpHeadersIntoFile(CSVStream& csv) {
            const std::vector<std::string> headers = {
                    "SourceFile", "FileExtension", "DataCategory", "SegmentationMethod", "SegmentLength",
                    "GenerationCount", "PopulationSize", "MutationRate", "CrossoverRate",
                    "UsesAnnealing", "EliteSize", "TournamentSize", "MutationThreshold", "VariationThreshold"};
            std::for_each(headers.begin(), headers.end(), [&](auto header) { pushItemIntoCSV(header, csv); });
            finishRow(csv);
        }

    private:

        json getJSONFromFile(const FileName fileName) {
            std::ifstream readingStream(fileName);
            return json::parse(readingStream);
        }

        std::string getWhatsAfterTheLastOf(const std::string& input, const char delimiter) {
            const size_t position = input.find_last_of(delimiter);
            return input.substr(position+1, input.size());
        }

        FileName getJustFileName(const std::string& filePath) {
            return getWhatsAfterTheLastOf(filePath, fileSlash);
        }

        FileExtension getFileExtension(const FileName& fileName) {
            return getWhatsAfterTheLastOf(fileName, fileExtensionDot); //TODO it should force it to be lowercase
        }

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


            if      (extensionIsInSet(imageExtensions)) return Image;
            else if (extensionIsInSet(videoExtensions)) return Video;
            else if (extensionIsInSet(audioExtensions)) return Audio;
            else if (extensionIsInSet(textExtensions))  return Text;
            else if (extensionIsInSet(documentExtensions)) return Document;
            else return Other;
        }
    };

} // GC

#endif //EVOCOM_JSON2CSV_HPP
