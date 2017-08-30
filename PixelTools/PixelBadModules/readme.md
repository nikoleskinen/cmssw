# PixelBadModules.cc

## Outline
This class provides functions for searching overlappling detector areas that contains
detectors that are not working.

Procedure for finding these areas are:
1. Fetch raw IDs of barrel, endcap and bad detector into containers.
2. Find clusters of bad detectors i.e. partition bad detectors into clusters. 
In these clusters each detector is adjecent to some other bad detector in same cluster
3. Abstract away detector data from clusters by finding verges of cluster. 
These verges are represented as phiSpan, rSpan, zSpan in same way that single modules 
are in 
[surface class](http://cmsdoxygen.web.cern.ch/cmsdoxygen/CMSSW_9_2_0/doc/html/de/d10/classSurface.html).
4. Clusters are then compared with each other to find out if there is overlap if we look them from z-axis inside pixel detector.

## Call graph
![Call graph](callGraph/callGraph.png)
***

## Data containers and aliases
* `using det_t = uint32_t`
    * detectors type as raw id
* `using Span_t = std::pair<float,float>`
    * type of cluster's span equivalent to detectors span (phiSpan,zSpan,rSpan)
* `using DetContainer = std::vector<uint32_t>`
    * vector of detectors' raw ids 
* `using Cluster = std::vector<uint32_t>`
    * cluster of detectors are represented as vector of raw det ids
* `using ClusterContainer = std::vector<Cluster>`
    * collection of clusters
* `DetContainer pixelDetsBarrel`
* `DetContainer pixelDetsEndcap`
* `DetContainer badPixelDetsBarrel`
* `DetContainer badPixelDetsEndcap`

## Functions for fetchin data from edm handles
* `void getPixelDetsBarrel()`
    * add barrel detectors from trackerGeometry ES handle into pixelDetsBarrel
* `void getPixelDetsEndcap()`
    * add endcap detectors from trackerGeometry ES handle into pixelDetsBarrel
* `void getBadPixelDets()`
    * add bad detectors from pixelQuality ES handle into badPixelDetsBarrel and badPixelDetsEndcap

## Printing functions
* `void detInfo(const det_t & det, Stream & ss)`
* `void clusterSpanInfo(const ClusterSpan & cspan, Stream & ss)`
* `void printPixelDets()`
* `void printBadPixelDets()`
* `void printBadClusters()`
* `void printBadClusterSpans()`

## Functions for finding bad clusters
* `static bool phiRangesOverlap(const float x1,const float x2, const float y1,const float y2)`
* `static bool phiRangesOverlap(const Span_t&phiSpanA, const Span_t&phiSpanB)`
    * Check if given spans `phiSpanA` and `phiSpanB` overlaps. It is assumed that `phiSpan.first` is more clockwise than `phiSpan.second`
* `bool detWorks(det_t det)`
    * check if det whether works or not
* `Cluster badAdjecentDetsBarrel(const det_t & det)`
    * returns list of adjecent barrel detectors that are not working ie next and previous in same module or next or previous in different ladders with same module number
* `Cluster badAdjecentDetsEndcap(const det_t & det)`
    * returns list of bad endcap detectors that overlap in phi dimension with `det` detector and are in the same disk
* `Cluster reachableCluster(const det_t & initDet, DetectorSet & foundDets)`
    * returns list of detectors that are reachable from `det` detector using `badAdjecentDets*()` function. This is basically implemented as breadth-first search for a graph where detectors are vertices and there is edge between two detectors `detA` and `detB` if `detA` is in `badAdjecentDets*(detB)` 
* `ClusterContainer badClustersBarrel()`
    * return list of barrel clusters (bad detectors that are adjecent to each other)
* `ClusterContainer badClustersEndcap()`
    * same for endcap detectors

## Functions for finding ranges that clusters cover
* `static bool phiMoreClockwise(float phiA, float phiB)`
    * return true if `phiA` is more clockwise than `phiB` while considering only that half circle that angles are located
* `static bool phiMoreCounterclockwise(float phiA, float phiB)`
    * same for other direction
* `void getPhiSpanBarrel(const Cluster & cluster, ClusterSpan & cspan)`
    * gets limits of barrel cluster in phi dimension using ladder numbers
        1. First creates unique list of ladder numbers
        2. Infer first and last ladder in phi dimension using modulus arithmetics
        3. Get corresponding phi values for first and last ladder
* `void getPhiSpanEndcap(const Cluster & cluster, ClusterSpan & cspan)`
    * this is does same for endcap cluster than previous function did for barrels.
    Implementation is much more naive/bruteforce than previous and would propably be faster when implemented in same way than barrel counterpart.
    * Function:
        1. gets one work detector from clusters and starts comparing it to other detectors.
        2. if function counters other detector that overlaps with work detector in phi dimension 
        and is more clockwise direction, function uses this detector as work detector 
        and starts comparing it to the other detectors 
        3. If no detectors is found that overlaps and is more clockwise, the starting detector is
        found
        4. Same is done for counter clockwise direction for the ending detector

* `void getZSpan(const Cluster & cluster, ClusterSpan & cspan)`
    * gets smallest and greatest z values in cluster
* `void getRSpan(const Cluster & cluster, ClusterSpan & cspan)`
    * gets smallest and greatest r values in cluster
* `void getSpan(const Cluster & cluster, ClusterSpan & cspan)`
    * wrapper for phi, r and z getSpan functions
* `ClusterSpanContainerPair clusterSpans()`
    * returns container pair of cluster spans. First value is containes barrel cluster spans and
    and second endcap spans

## Functions for findind overlapping clusters
* `static float zAxisIntersection(const float zrPointA[2], const float zrPoint[2])`
    * returns z-axis value where line that goes via two points intersects z axis.
    Points and line are in z-x (z-r) plane
* `bool getZAxisOverlapRangeBarrel(const ClusterSpan & cspanA, const ClusterSpan & cspanB,std::pair<float,float> & range)`
    * returns true if there is some range in z axis where clusters A and B overlaps.
    Function also retrun this range via `range` reference parameter
* `bool getZAxisOverlapRangeEndcap(const ClusterSpan & cspanA, const ClusterSpan & cspanB,std::pair<float,float> & range)`
    * same for endcap clusters, but with assumption that we are only interested in ranges that are inside pixel detector
* `bool getZAxisOverlapRangeBarrelEndcap(const ClusterSpan & cspanA, const ClusterSpan & cspanB,std::pair<float,float> & range)`
    * same for clusters where other is barrel cluster and other is endcap
* `void compareClusterSpansBarrel()`

