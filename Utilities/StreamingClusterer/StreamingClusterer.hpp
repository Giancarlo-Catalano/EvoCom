//
// Created by gian on 11/11/22.
//

#ifndef EVOCOM_STREAMINGCLUSTERER_HPP
#define EVOCOM_STREAMINGCLUSTERER_HPP
#include "../utilities.hpp"
#include <functional>


namespace GC {

    //Prerequisite: T must have a default constructor
    template<class T, class Field>
    class StreamingClusterer {
    public: //types
        using Distance = Field;
        using Cluster = std::vector<T>;
        using ClusterHandler = std::function<void(const Cluster &)>;
        using Metric = std::function<Field (const T&, const T&)>;

    private:
        const Metric metric;
        const ClusterHandler handler;
        const Distance thresholdDistance; //maximum distance between the cluster head and its elements
        const size_t cooldownPeriod; //assumed to be at least 1

        size_t sinceLastPardonOrClusterStart;
        bool hasProcessedFirstItem;
        bool holdsCurrentItem;
        Cluster currentCluster;

        T clusterHead;
        T currentItem;

        const size_t maxClusterLength = 65536; //that's the amount of items supplied, not the bytes

    public:

        StreamingClusterer(const Metric metric, const ClusterHandler handler, const Distance maxDistanceFromClusterHead, const size_t cooldown) :
                metric(metric),
                handler(handler),
                thresholdDistance(maxDistanceFromClusterHead),
                currentCluster(),
                cooldownPeriod(cooldown),
                sinceLastPardonOrClusterStart(0),
                hasProcessedFirstItem(false),
                holdsCurrentItem(false){}

        void pushItem(const T &newItem) {
            processNewItem(newItem);
        }

        void finish() {
            if (hasProcessedFirstItem && holdsCurrentItem) { //ie has a current item that still needs to be processed
                //forcefully add the last item onto a cluster
                addToCluster(currentItem);
                passToHandler();
            }
            else if (hasProcessedFirstItem)
                passToHandler();
        }

    private:
        void passToHandler() {
            handler(currentCluster);
        }

        void startNewCluster(const T& item) {
            currentCluster = {item};
            clusterHead = item;
        }

        bool isCloseEnoughToHead(const T& item) {
            const Field distance = metric(item, clusterHead);
            return distance < thresholdDistance;
        }

        void addToCluster(const T& item) {
            currentCluster.emplace_back(item);
        }

        void finishCurrentClusterAndRestartFrom(const T& item) {
            passToHandler();
            startNewCluster(item);
        }

        void processNewItem(const T& newItem) {

            //special behaviour if it's the very first item in the stream
            if (!hasProcessedFirstItem) {
                startNewCluster(newItem);
                hasProcessedFirstItem = true;
                return;
            }

            //special behaviour for the second item in the stream
            if (!holdsCurrentItem) {
                currentItem = newItem;
                holdsCurrentItem = true;
                return;
            }

            //for any other item in the stream:
            if (currentCluster.size() >= maxClusterLength) {  //if the current cluster is too big, don't even consider adding it
                finishCurrentClusterAndRestartFrom(currentItem);
                currentItem = newItem;
                sinceLastPardonOrClusterStart = 0;
            }
            else if (isCloseEnoughToHead(currentItem)) {  //if it's close enough, add it to the cluster
                addToCluster(currentItem);
                sinceLastPardonOrClusterStart++;
                currentItem = newItem;
            }
            else { //it's not close enough, but it might still get added if we can pardon
                if (sinceLastPardonOrClusterStart >= cooldownPeriod && isCloseEnoughToHead(newItem))  // if it's been long enough since the last pardon, and the item after qualifies..
                    addToCluster(currentItem);
                else  //cannot be forgiven, start a new cluster
                    finishCurrentClusterAndRestartFrom(currentItem);
                currentItem = newItem;
                sinceLastPardonOrClusterStart = 0;
            }
        }
    };



}

#endif //EVOCOM_STREAMINGCLUSTERER_HPP
