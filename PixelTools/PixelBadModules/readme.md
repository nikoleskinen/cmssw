# PixelBadModules.cc

## Outline
This class provides functions for searching overlappling detector areas that contains
detectors that are not working.


## Call graph and program progress
![Call graph](callGraph/callGraph.png)

Above figure illustrates function call dependencies.
Following procedure is conducted while searching overlapping inactive detector groups.
1. First fetch pixel quality, tracker geometry and topology information into `edm::ESHandle`s
    1. Detectors' raw IDs into `pixelDetsBarrel` and `pixelDetsEndcap`
    2. Inactive detectors' raw IDs into `badPixelDetsBarrel` and `badPixelDetsEndcap`
2. Partition inactive detectors into `detGroup`s. One partition i.e. `detGroup` 
contains detectors that are inactive and adjecent to some other inactive detector 
in same `detGroup`.
Therefore, in one detGroup it is possible to travel from one detector to other just using 
adjecent detectors. Hence, we can use breadth-first search for finding these `detGroups`
    1. Pick one inactive detector that is not yet included into `detGroup`
    2. Find all inactive detectors that are reachable from this detector 
    i.e. do breadth-first search (`reachableDetGroup`)
    3. Form `detGroup` from found detectors
    4. Go to first step if any unpartitioned detectors are left
3. Find spans for `detGroups`. These spans are analogic to singular detector's spans
([surface class](http://cmsdoxygen.web.cern.ch/cmsdoxygen/CMSSW_9_2_0/doc/html/de/d10/classSurface.html))
(`detGroupSpans`)
    1. z and r spans are quite easy, but with phi span we use a little modulus arithmetics in order to 
    find where detGroup starts and ends in phi dimension
4. Compare `detGroup`s with each other so we find which one overlap while we look from z-axis, inside pixel detector.
(`overlappingSpans`)
    1. First check if phiSpans overlap
    2. if they overlap, check if there is range in z-axis where `detGroup`s overlap
    in z-r-dimension.
    3. If there is some range, check if range is within some threshold (default=inf).
    If it is, add span to container that will be returned as output

***

## Data containers and aliases
* `using det_t = uint32_t`
    * detectors type as raw id
* `using Span_t = std::pair<float,float>`
    * type of detGroup's span equivalent to detectors span (phiSpan,zSpan,rSpan)
* `using DetContainer = std::vector<uint32_t>`
    * vector of detectors' raw ids 
* `using DetGroup = std::vector<uint32_t>`
    * detGroup of detectors are represented as vector of raw det ids
* `using DetGroupContainer = std::vector<DetGroup>`
    * collection of detGroups
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
* `void detGroupSpanInfo(const DetGroupSpan & cspan, Stream & ss)`
* `void printPixelDets()`
* `void printBadPixelDets()`
* `void printBadDetGroups()`
* `void printBadDetGroupSpans()`

## Functions for finding bad detGroups
* `static bool phiRangesOverlap(const float x1,const float x2, const float y1,const float y2)`
* `static bool phiRangesOverlap(const Span_t&phiSpanA, const Span_t&phiSpanB)`
    * Check if given spans `phiSpanA` and `phiSpanB` overlaps. It is assumed that `phiSpan.first` is more clockwise than `phiSpan.second`
* `bool detWorks(det_t det)`
    * check if det whether works or not
* `DetGroup badAdjecentDetsBarrel(const det_t & det)`
    * returns list of adjecent barrel detectors that are not working ie next and previous in same module or next or previous in different ladders with same module number
* `DetGroup badAdjecentDetsEndcap(const det_t & det)`
    * returns list of bad endcap detectors that overlap in phi dimension with `det` detector and are in the same disk
* `DetGroup reachableDetGroup(const det_t & initDet, DetectorSet & foundDets)`
    * returns list of detectors that are reachable from `det` detector using `badAdjecentDets*()` function. This is basically implemented as breadth-first search for a graph where detectors are vertices and there is edge between two detectors `detA` and `detB` if `detA` is in `badAdjecentDets*(detB)` 
* `DetGroupContainer badDetGroupsBarrel()`
    * return list of barrel detGroups (bad detectors that are adjecent to each other)
* `DetGroupContainer badDetGroupsEndcap()`
    * same for endcap detectors

## Functions for finding ranges that detGroups cover
* `static bool phiMoreClockwise(float phiA, float phiB)`
    * return true if `phiA` is more clockwise than `phiB` while considering only that half circle that angles are located
* `static bool phiMoreCounterclockwise(float phiA, float phiB)`
    * same for other direction
* `void getPhiSpanBarrel(const DetGroup & detGroup, DetGroupSpan & cspan)`
    * gets limits of barrel detGroup in phi dimension using ladder numbers
        1. First creates unique list of ladder numbers
        2. Infer first and last ladder in phi dimension using modulus arithmetics
        3. Get corresponding phi values for first and last ladder
* `void getPhiSpanEndcap(const DetGroup & detGroup, DetGroupSpan & cspan)`
    * this is does same for endcap detGroup than previous function did for barrels.
    Implementation is much more naive/bruteforce than previous and would propably be faster when implemented in same way than barrel counterpart.
    * Function:
        1. gets one work detector from detGroups and starts comparing it to other detectors.
        2. if function counters other detector that overlaps with work detector in phi dimension 
        and is more clockwise direction, function uses this detector as work detector 
        and starts comparing it to the other detectors 
        3. If no detectors is found that overlaps and is more clockwise, the starting detector is
        found
        4. Same is done for counter clockwise direction for the ending detector

* `void getZSpan(const DetGroup & detGroup, DetGroupSpan & cspan)`
    * gets smallest and greatest z values in detGroup
* `void getRSpan(const DetGroup & detGroup, DetGroupSpan & cspan)`
    * gets smallest and greatest r values in detGroup
* `void getSpan(const DetGroup & detGroup, DetGroupSpan & cspan)`
    * wrapper for phi, r and z getSpan functions
* `DetGroupSpanContainerPair detGroupSpans()`
    * returns container pair of detGroup spans. First value is containes barrel detGroup spans and
    and second endcap spans

## Functions for findind overlapping detGroups
* `static float zAxisIntersection(const float zrPointA[2], const float zrPoint[2])`
    * returns z-axis value where line that goes via two points intersects z axis.
    Points and line are in z-x (z-r) plane
* `bool getZAxisOverlapRangeBarrel(const DetGroupSpan & cspanA, const DetGroupSpan & cspanB,std::pair<float,float> & range)`
    * returns true if there is some range in z axis where detGroups A and B overlaps.
    Function also retrun this range via `range` reference parameter
* `bool getZAxisOverlapRangeEndcap(const DetGroupSpan & cspanA, const DetGroupSpan & cspanB,std::pair<float,float> & range)`
    * same for endcap detGroups, but with assumption that we are only interested in ranges that are inside pixel detector
* `bool getZAxisOverlapRangeBarrelEndcap(const DetGroupSpan & cspanA, const DetGroupSpan & cspanB,std::pair<float,float> & range)`
    * same for detGroups where other is barrel detGroup and other is endcap
* `void compareDetGroupSpansBarrel()`

## Curvature
* `bool getCirclesViaTwoPoints(const Point & p1, const Point & p2, const float r, Circle & c1, Circle & c2)`
    * Function is not included into other code in any way
    * It calculates two possible circles that go via points p1 and p2 if possible
    * Centers of these circles must be in line that is perpendicular to line that goes via p1 and p2. Line must also go via midpoint of p1 and p2. Distance from midpoint to center of circle can be calculated using pythagoras


## N.B.
Program does not care if detGroups are adjecent or not so overlappingSpans return spans even
if detGroups are for example in layers or disks 1 and 3. Adjecency check can be implemented 
in `overlappingSpans` function, in beginning of the for-loops

## TODO
* Curvature of particle tracks is not considered, thought there is one function for this purpose
(`getCirclesViaTwoPoints`).
    * One possibility is to replace `zAxisIntersection`-function with function that calculates where circle intersects with z-axis 
