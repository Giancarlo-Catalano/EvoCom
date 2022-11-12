//
// Created by gian on 11/11/22.
//

#ifndef EVOCOM_STREAMINGCLUSTERER_HPP
#define EVOCOM_STREAMINGCLUSTERER_HPP
#include "../utilities.hpp"
#include <functional>


namespace GC {
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
            return metric(item, clusterHead) < thresholdDistance;
        }

        void addToCluster(const T& item) {
            currentCluster.emplace_back(item);
        }

        void finishCurrentClusterAndRestartFrom(const T& item) {
            passToHandler();
            startNewCluster(item);
        }

        void processNewItem(const T& newItem) {
            //attempts to add currentItem
            //if currentItem close enough to the head, addToCluster(currentItem);currentItem = newItem;
            //otherwise, then it may be pardoned if newItem is close enough to the head
                //in this case, currentItem will be added, and then currentItem = newItem


            if (!hasProcessedFirstItem) {
                startNewCluster(newItem);
                hasProcessedFirstItem = true;
                return;
            }

            if (!holdsCurrentItem) {
                currentItem = newItem;
                holdsCurrentItem = true;
                return;
            }

            if (isCloseEnoughToHead(currentItem)) {
                addToCluster(currentItem);
                sinceLastPardonOrClusterStart++;
                currentItem = newItem;
            }
            else {
                if (sinceLastPardonOrClusterStart >= cooldownPeriod && isCloseEnoughToHead(newItem))  //we can pardon!
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
