#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelQuality.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "CondFormats/DataRecord/interface/SiPixelQualityRcd.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include <string>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <queue>
#include <functional>
#include <tuple>
#include <set>
#include <limits>
#include <ctime>
#include <chrono>
class PixelBadModules : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
    public:
        //analyzer functions
        explicit PixelBadModules(const edm::ParameterSet&);
        virtual void beginJob() override;
        virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
        virtual void endJob() override;
        ~PixelBadModules();
        // Output types
        struct ClusterSpan {
            int subdetId;
            std::pair<float,float> phiSpan;
            std::pair<float,float> zSpan;
            std::pair<float,float> rSpan;
            unsigned int layer;
            unsigned int disk;
            ClusterSpan():
                subdetId(0),
                phiSpan(0,0),
                zSpan(0,0),
                rSpan(0,0),
                layer(0),disk(0)
            {}
        };
        // Output type aliases
        using ClusterSpanContainer = std::vector<ClusterSpan>;
        using ClusterSpanContainerPair = std::pair<ClusterSpanContainer,ClusterSpanContainer>;
        using OverlapSpans = std::vector<ClusterSpan>;
        using OverlapSpansContainer = std::vector<OverlapSpans>;
    private:
        // static data members
        const static unsigned int nLayer1Ladders = 12;
        const static unsigned int nLayer2Ladders = 28;
        const static unsigned int nLayer3Ladders = 44;
        const static unsigned int nLayer4Ladders = 64;
        const static unsigned int nModulesPerLadder = 8;
        // type aliases
        using det_t = uint32_t;
        using Span_t = std::pair<float,float>;
        using DetContainer = std::vector<uint32_t>;
        using Cluster = std::vector<uint32_t>;
        using ClusterContainer = std::vector<Cluster>;
        using DetectorSet = std::set<uint32_t>;
        using Stream = std::stringstream;
        // data handles and containers
        edm::ESHandle<SiPixelQuality> pixelQuality;
        edm::ESHandle<TrackerGeometry> trackerGeometry;
        edm::ESHandle<TrackerTopology> trackerTopology;
        DetContainer pixelDetsBarrel;
        DetContainer pixelDetsEndcap;
        DetContainer badPixelDetsBarrel;
        DetContainer badPixelDetsEndcap;
        // functions for fetching date from handles
        void getPixelDetsBarrel();
        void getPixelDetsEndcap();
        void getBadPixelDets();
        // Printing functions
        void detInfo(const det_t & det, Stream & ss);
        void clusterSpanInfo(const ClusterSpan & cspan, Stream & ss);
        void printPixelDets();
        void printBadPixelDets();
        void printBadClusters();
        void printBadClusterSpans();
        void printOverlapSpans();
        // Functions for finding bad clusters
        static bool phiRangesOverlap(const float x1,const float x2, const float y1,const float y2);
        static bool phiRangesOverlap(const Span_t&phiSpanA, const Span_t&phiSpanB);
        bool detWorks(det_t det);
        Cluster badAdjecentDetsBarrel(const det_t & det);
        Cluster badAdjecentDetsEndcap(const det_t & det);
        Cluster reachableCluster(const det_t & initDet, DetectorSet & foundDets);
        ClusterContainer badClustersBarrel();
        ClusterContainer badClustersEndcap();
        // Functions for finding ranges that clusters cover
        static bool phiMoreClockwise(float phiA, float phiB);
        static bool phiMoreCounterclockwise(float phiA, float phiB);
        void getPhiSpanBarrel(const Cluster & cluster, ClusterSpan & cspan);
        void getPhiSpanEndcap(const Cluster & cluster, ClusterSpan & cspan);
        void getZSpan(const Cluster & cluster, ClusterSpan & cspan);
        void getRSpan(const Cluster & cluster, ClusterSpan & cspan);
        void getSpan(const Cluster & cluster, ClusterSpan & cspan);
        ClusterSpanContainerPair clusterSpans();
        // Functions for findind overlapping functions
        static float zAxisIntersection(const float zrPointA[2], const float zrPoint[2]);
        bool getZAxisOverlapRangeBarrel(const ClusterSpan & cspanA, const ClusterSpan & cspanB,std::pair<float,float> & range);
        bool getZAxisOverlapRangeEndcap(const ClusterSpan & cspanA, const ClusterSpan & cspanB,std::pair<float,float> & range);
        bool getZAxisOverlapRangeBarrelEndcap(const ClusterSpan & cspanA, const ClusterSpan & cspanB,std::pair<float,float> & range);
        void compareClusterSpansBarrel();
        OverlapSpansContainer overlappingSpans(float zAxisThreshold = std::numeric_limits<float>::infinity());
};
//analyzer functions
PixelBadModules::PixelBadModules(const edm::ParameterSet& iConfig){}
PixelBadModules::~PixelBadModules(){}
void PixelBadModules::beginJob(){}
void PixelBadModules::endJob() {}
void PixelBadModules::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){
    // Set data to handles
    iSetup.get<SiPixelQualityRcd>().get(pixelQuality);
    iSetup.get<TrackerDigiGeometryRecord>().get(trackerGeometry);
    iSetup.get<TrackerTopologyRcd>().get(trackerTopology);
    // assign data to instance variables
    this->getPixelDetsBarrel();
    this->getPixelDetsEndcap();
    this->getBadPixelDets();
    //timer
    std::chrono::time_point<std::chrono::system_clock> start,end;
    start = std::chrono::system_clock::now();
    // Comparing
    this->printOverlapSpans();
    // timer
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = end-start;
    edm::LogPrint("") << "Elapsed time (s): " << elapsed_time.count();
}
// Functions for fetching date from handles
void PixelBadModules::getPixelDetsBarrel(){
    for(auto const & geomDetPtr : trackerGeometry->detsPXB() ) {
        if(geomDetPtr->geographicalId().subdetId() == PixelSubdetector::PixelBarrel){
            pixelDetsBarrel.push_back(geomDetPtr->geographicalId().rawId());
        }
    }
    std::sort(pixelDetsBarrel.begin(),pixelDetsBarrel.end());
}
void PixelBadModules::getPixelDetsEndcap(){
    for(auto const & geomDetPtr : trackerGeometry->detsPXF() ) {
        if(geomDetPtr->geographicalId().subdetId() == PixelSubdetector::PixelEndcap){
            pixelDetsEndcap.push_back(geomDetPtr->geographicalId().rawId());
        }
    }
    std::sort(pixelDetsEndcap.begin(),pixelDetsEndcap.end());
}
void PixelBadModules::getBadPixelDets(){
    for(auto const & disabledModule : pixelQuality->getBadComponentList() ){
        if( DetId(disabledModule.DetID).subdetId() == PixelSubdetector::PixelBarrel ){
            badPixelDetsBarrel.push_back( disabledModule.DetID );
        } else if ( DetId(disabledModule.DetID).subdetId() == PixelSubdetector::PixelEndcap ){
            badPixelDetsEndcap.push_back( disabledModule.DetID );
        }
    }
    std::sort(badPixelDetsBarrel.begin(),badPixelDetsBarrel.end());
    std::sort(badPixelDetsEndcap.begin(),badPixelDetsEndcap.end());
    badPixelDetsBarrel.erase(
            std::unique(badPixelDetsBarrel.begin(),badPixelDetsBarrel.end()),badPixelDetsBarrel.end());
    badPixelDetsEndcap.erase(
            std::unique(badPixelDetsEndcap.begin(),badPixelDetsEndcap.end()),badPixelDetsEndcap.end());
}
// Printing functions
void PixelBadModules::detInfo(const det_t & det, Stream & ss){
    using std::tie;
    using std::setw;
    using std::showpos;
    using std::noshowpos;
    using std::fixed;
    using std::right;
    using std::left;
    using std::setprecision;
    using std::setfill;
    std::string deli = "; ";
    ss << "id:[" << det << "]" <<deli;
    ss << "subdetid:[" << DetId(det).subdetId() << "]" << deli;
    if(DetId(det).subdetId()==PixelSubdetector::PixelBarrel){
        unsigned int layer  = trackerTopology->pxbLayer (DetId(det));
        unsigned int ladder = trackerTopology->pxbLadder(DetId(det));
        unsigned int module = trackerTopology->pxbModule(DetId(det));
        ss  << "layer:["                      << layer  << "]" << deli 
            << "ladder:[" << right << setw(2) << ladder << "]" << deli 
            << "module:["                     << module << "]" << deli;
    }else if(DetId(det).subdetId()==PixelSubdetector::PixelEndcap){
        unsigned int disk  = trackerTopology->pxfDisk (DetId(det));
        unsigned int blade = trackerTopology->pxfBlade(DetId(det));
        unsigned int panel = trackerTopology->pxfPanel(DetId(det));
        ss  << left << setw(6) << "disk:"  << "["            << right << disk  << "]" << deli 
            << left << setw(7) << "blade:" << "[" << setw(2) << right << blade << "]" << deli 
            << left << setw(7) << "panel:" << "["            << right << panel << "]" << deli;
    }
    float phiA,phiB,zA,zB,rA,rB;
    auto detSurface = trackerGeometry->idToDet(DetId(det))->surface();
    tie(phiA,phiB) = detSurface.phiSpan();
    tie(zA,zB) = detSurface.zSpan();
    tie(rA,rB) = detSurface.rSpan();
    ss 
        << setprecision(16) 
        << fixed 
        << showpos
        << setfill(' ')
        << "phi:[" << right << setw(12) << phiA << "," << left << setw(12) << phiB << "]" << deli
        << "z:["   << right << setw(7)  << zA   << "," << left << setw(7)  << zB   << "]" << deli << noshowpos
        << "r:["   << right << setw(10) << rA   << "," << left << setw(10) << rB   << "]" << deli;

}
void PixelBadModules::clusterSpanInfo(const ClusterSpan & cspan, Stream & ss){
    using std::showpos;
    using std::noshowpos;
    using std::fixed;
    using std::setprecision;
    using std::setw;
    using std::setfill;
    using std::left;
    using std::right;
    std::string deli = "; ";
    ss  << "subdetid:[" << cspan.subdetId << "]" << deli
        //<< setfill(' ') << setw(36) << " "
        << setprecision(16)
        << showpos
        << "phi:<" << right << setw(12) << cspan.phiSpan.first << "," << left << setw(12) << cspan.phiSpan.second << ">" << deli
        << "z:<" << right << setw(7) << cspan.zSpan.first << "," << left << setw(7) << cspan.zSpan.second << ">" << deli << noshowpos
        << "r:<" << right << setw(10) << cspan.rSpan.first << "," << left << setw(10) << cspan.rSpan.second << ">" << deli
       ; 
}
void PixelBadModules::printPixelDets(){
    edm::LogPrint("") << "Barrel detectors:";
    Stream ss;
    for(auto const & det : pixelDetsBarrel){
        detInfo(det,ss);
        edm::LogPrint("") << ss.str();ss.str(std::string());
    }
    edm::LogPrint("") << "Endcap detectors;";
    for(auto const & det : pixelDetsEndcap){
        detInfo(det,ss);
        edm::LogPrint("") << ss.str();ss.str(std::string());
    }
}
void PixelBadModules::printBadPixelDets(){
    edm::LogPrint("") << "Bad barrel detectors:";
    Stream ss;
    for(auto const & det : badPixelDetsBarrel){
        detInfo(det,ss);
        edm::LogPrint("") << ss.str();ss.str(std::string());
    }
    edm::LogPrint("") << "Endcap detectors;";
    for(auto const & det : badPixelDetsEndcap){
        detInfo(det,ss);
        edm::LogPrint("") << ss.str();ss.str(std::string());
    }
}
void PixelBadModules::printBadClusters(){
    ClusterContainer badClustersBar = badClustersBarrel();
    ClusterContainer badClustersEnd = badClustersEndcap();
    Stream ss;
    for(auto const & cluster : badClustersBar){
        ss << std::setfill(' ') << std::left << std::setw(16) << "Cluster:";
        ClusterSpan cspan;
        getPhiSpanBarrel(cluster,cspan);
        getZSpan(cluster,cspan);
        getRSpan(cluster,cspan);
        clusterSpanInfo(cspan,ss);
        ss<<std::endl;
        for(auto const & det : cluster){
            detInfo(det,ss);ss<<std::endl;
        }
        ss<<std::endl;
    }
    for(auto const & cluster : badClustersEnd){
        ss << std::setfill(' ') << std::left << std::setw(16) << "Cluster:";
        ClusterSpan cspan;
        getPhiSpanEndcap(cluster,cspan);
        getZSpan(cluster,cspan);
        getRSpan(cluster,cspan);
        clusterSpanInfo(cspan,ss);
        ss << std::endl;
        for(auto const & det : cluster){
            detInfo(det,ss);ss<<std::endl;
        }
        ss << std::endl;
    }
    edm::LogPrint("")<<ss.str();
}
void PixelBadModules::printBadClusterSpans(){
    ClusterSpanContainerPair cspans = clusterSpans();
    Stream ss;
    for(auto const & cspan : cspans.first){
        clusterSpanInfo(cspan,ss);ss<<std::endl;
    }
    for(auto const & cspan : cspans.second){
        clusterSpanInfo(cspan,ss);ss<<std::endl;
    }
    edm::LogPrint("") << ss.str();
}
void PixelBadModules::printOverlapSpans(){
    OverlapSpansContainer ospans = this->overlappingSpans();
    Stream ss;
    for(auto const & spans : ospans){
        ss << "Overlapping clusters:\n";
        for(auto const cspan : spans){
            clusterSpanInfo(cspan,ss);
            ss << std::endl;
        }
    }
    edm::LogPrint("") << ss.str();
}
// Functions for finding bad clusters
bool PixelBadModules::phiRangesOverlap(const float x1,const float x2, const float y1,const float y2){

    // assuming phi ranges are [x1,x2] and [y1,y2] and xi,yi in [-pi,pi]
    if(x1<=x2 && y1<=y2){
        return x1<=y2 && y1 <= x2;
    }else if (( x1>x2 && y1 <= y2) || (y1 > y2 && x1 <= x2 )){
        return y1 <= x2 || x1 <= y2;
    }else if (x1 > x2 && y1 > y2){
        return true;
    }else {
        return false;
    }
}
bool PixelBadModules::phiRangesOverlap(const Span_t & phiSpanA, const Span_t & phiSpanB){
    float x1,x2,y1,y2;
    std::tie(x1,x2) = phiSpanA;
    std::tie(y1,y2) = phiSpanB;
    // assuming phi ranges are [x1,x2] and [y1,y2] and xi,yi in [-pi,pi]
    if(x1<=x2 && y1<=y2){
        return x1<=y2 && y1 <= x2;
    }else if (( x1>x2 && y1 <= y2) || (y1 > y2 && x1 <= x2 )){
        return y1 <= x2 || x1 <= y2;
    }else if (x1 > x2 && y1 > y2){
        return true;
    }else {
        return false;
    }
}
bool PixelBadModules::detWorks(det_t det){
    return 
        std::find(badPixelDetsBarrel.begin(),badPixelDetsBarrel.end(),det)
        == badPixelDetsBarrel.end()
        &&
        std::find(badPixelDetsEndcap.begin(),badPixelDetsEndcap.end(),det)
        == badPixelDetsEndcap.end()
        ;
}
PixelBadModules::Cluster PixelBadModules::badAdjecentDetsBarrel(const det_t & det){
    using std::remove_if;
    using std::bind1st;
    using std::mem_fun;

    Cluster adj;
    auto const & tTopo = trackerTopology;
    auto const & detId = DetId(det);
    unsigned int layer  = tTopo->pxbLayer (detId);
    unsigned int ladder = tTopo->pxbLadder(detId);
    unsigned int module = tTopo->pxbModule(detId);
    unsigned int nLads;
    switch (layer){
        case 1:  nLads = nLayer1Ladders;break;
        case 2:  nLads = nLayer2Ladders;break;
        case 3:  nLads = nLayer3Ladders;break;
        case 4:  nLads = nLayer4Ladders;break;
        default: nLads = 0 ;break;
    }
    //add detectors from next and previous ladder
    adj.push_back( tTopo->pxbDetId( layer, ((ladder-1)+1)%nLads+1, module )() );
    adj.push_back( tTopo->pxbDetId( layer, ((ladder-1)-1+nLads)%nLads+1, module )() );
    //add adjecent detectors from same ladder
    switch (module){
        case 1:
            adj.push_back( tTopo->pxbDetId( layer, ladder, module+1 )() );
            break;
        case nModulesPerLadder:
            adj.push_back( tTopo->pxbDetId( layer, ladder, module-1 )() );
            break;
        default :
            adj.push_back( tTopo->pxbDetId( layer, ladder, module+1 )() );
            adj.push_back( tTopo->pxbDetId( layer, ladder, module-1 )() );
            break;
    }
    //remove working detectors from list
    adj.erase(remove_if(adj.begin(),adj.end(),bind1st(
                    mem_fun(&PixelBadModules::detWorks),this)),adj.end());
    return adj;
}
PixelBadModules::Cluster PixelBadModules::badAdjecentDetsEndcap(const det_t & det){
    // this might be faster if adjecent 
    using std::tie;
    using std::ignore;
    Cluster adj;
    Span_t  phiSpan, phiSpanComp;
    float z, zComp;
    unsigned int disk, diskComp;
    auto const & detSurf = trackerGeometry->idToDet(DetId(det))->surface();
    phiSpan = detSurf.phiSpan();
    tie(z,ignore) = detSurf.zSpan();
    disk = trackerTopology->pxfDisk(DetId(det));
    // add detectors from same disk whose phi ranges overlap to the adjecent list
    for(auto const & detComp : badPixelDetsEndcap){
        auto const & detIdComp = DetId(detComp);
        auto const & detSurfComp = trackerGeometry->idToDet(detIdComp)->surface();
        diskComp = trackerTopology->pxfDisk(detIdComp);
        phiSpanComp = detSurfComp.phiSpan();
        tie(zComp,ignore) = detSurfComp.zSpan();
        if(det != detComp && disk == diskComp && z*zComp > 0
                && phiRangesOverlap(phiSpan,phiSpanComp)){
            adj.push_back(detComp);
        }
    }
    return adj;
}
PixelBadModules::Cluster PixelBadModules::reachableCluster(const det_t & initDet, DetectorSet & foundDets){
    Cluster reachableCluster;
    std::queue<det_t> workQueue;
    det_t workDet;
    Cluster badAdjDets;
    foundDets.insert(initDet);
    workQueue.push(initDet);
    reachableCluster.push_back(initDet);
    while(!workQueue.empty()){
        workDet = workQueue.front();workQueue.pop();
        if(DetId(workDet).subdetId() == PixelSubdetector::PixelBarrel){
            badAdjDets = this->badAdjecentDetsBarrel(workDet);
        }else if(DetId(workDet).subdetId() == PixelSubdetector::PixelEndcap){
            badAdjDets = this->badAdjecentDetsEndcap(workDet);
        }else {
            badAdjDets = {};
        }
        for(auto const & badDet : badAdjDets){
            if(foundDets.find(badDet) == foundDets.end()){
                reachableCluster.push_back(badDet);
                foundDets.insert(badDet);
                workQueue.push(badDet);
            }
        }
    }
    return reachableCluster;
}
PixelBadModules::ClusterContainer PixelBadModules::badClustersBarrel(){
    ClusterContainer clusters;
    DetectorSet foundDets;
    for(auto const & badDet : badPixelDetsBarrel){
        if(foundDets.find(badDet) == foundDets.end()){
            clusters.push_back(this->reachableCluster(badDet,foundDets));
        } 
    }
    return clusters;
}
PixelBadModules::ClusterContainer PixelBadModules::badClustersEndcap(){
    ClusterContainer clusters;
    DetectorSet foundDets;
    for(auto const & badDet : badPixelDetsEndcap){
        if(foundDets.find(badDet) == foundDets.end()){
            clusters.push_back(this->reachableCluster(badDet,foundDets));
        } 
    }
    return clusters;
}
// Functions for finding ClusterSpans
bool PixelBadModules::phiMoreClockwise(float phiA, float phiB){
    // return true if a is more clockwise than b
    // assuming both angels are in same half
    float xa,ya,xb,yb;
    xa = cos(phiA);
    ya = sin(phiA);
    xb = cos(phiB);
    yb = sin(phiB);
    if(xa >= 0 && xb >= 0){
        return ya <= yb;
    }else if (ya >= 0 && yb >= 0 ){
        return xa >= xb;
    }else if (xa <= 0 && xb <= 0){
        return ya >= yb;
    }else if (ya <= 0 && yb <= 0){
        return xa <= xb;
    }else {
        return false;
    }
}
bool PixelBadModules::phiMoreCounterclockwise(float phiA, float phiB){
    // return true if a is more counterclockwise than b
    // assuming both ngels are in same half
    float xa,ya,xb,yb;
    xa = cos(phiA);
    ya = sin(phiA);
    xb = cos(phiB);
    yb = sin(phiB);
    if(xa >= 0 && xb >= 0){
        return ya >= yb;
    }else if (ya >= 0 && yb >= 0 ){
        return xa <= xb;
    }else if (xa <= 0 && xb <= 0){
        return ya <= yb;
    }else if (ya <= 0 && yb <= 0){
        return xa >= xb;
    }else {
        return false;
    }
}
void PixelBadModules::getPhiSpanBarrel(const Cluster & cluster, ClusterSpan & cspan){
    // find phiSpan using ordered vector of unique ladders in cluster
    if(cluster.size() == 0){
        cspan = ClusterSpan();
        return;
    } else{
        cspan.layer = trackerTopology->pxbLayer(DetId(cluster[0]));
        cspan.disk = 0;
    }
    using uint = unsigned int;
    using LadderSet = std::set<uint>;
    using LadVec = std::vector<uint>;
    LadderSet lads;
    for(auto const & det : cluster){
        lads.insert(trackerTopology->pxbLadder(DetId(det)));
    }
    LadVec ladv(lads.begin(),lads.end());
    uint nLadders = 0;
    switch(cspan.layer){
        case 1: nLadders = nLayer1Ladders;break;
        case 2: nLadders = nLayer2Ladders;break;
        case 3: nLadders = nLayer3Ladders;break;
        case 4: nLadders = nLayer4Ladders;break;
        default: nLadders = 0;
    }
    // find start ladder of cluster
    uint i = 0;
    uint currentLadder = ladv[0];
    uint previousLadder = ladv[ (ladv.size()+i-1) % ladv.size() ];
    // loop until discontinuity is found from vector
    while ( (nLadders+currentLadder-1)%nLadders  == previousLadder  ){
        ++i;
        currentLadder = ladv[i%ladv.size()];
        previousLadder = ladv[ (ladv.size()+i-1)%ladv.size() ];
        if(i == ladv.size()){
            cspan.phiSpan.first  =  std::numeric_limits<float>::epsilon();
            cspan.phiSpan.second = -std::numeric_limits<float>::epsilon();
            return;
        }
    }
    uint startLadder = currentLadder;
    uint endLadder = previousLadder;
    auto detStart = trackerTopology->pxbDetId(cspan.layer,startLadder,1); 
    auto detEnd = trackerTopology->pxbDetId(cspan.layer,endLadder,1); 
    cspan.phiSpan.first  = trackerGeometry->idToDet(detStart)->surface().phiSpan().first;
    cspan.phiSpan.second = trackerGeometry->idToDet(detEnd)->surface().phiSpan().second;
}
void PixelBadModules::getPhiSpanEndcap(const Cluster & cluster, ClusterSpan & cspan){
    // this is quite naive/bruteforce method
    // 1) it starts by taking one detector from cluster and starts to compare it to others
    // 2) when it finds overlapping detector in clockwise direction it starts comparing 
    //    found detector to others
    // 3) search stops until no overlapping detectors in clockwise detector or all detectors
    //    have been work detector
    Stream ss;
    bool found = false;
    auto const & tGeom = trackerGeometry;
    Cluster::const_iterator startDetIter = cluster.begin();
    Span_t phiSpan,phiSpanComp;
    unsigned int counter = 0;
    while(!found){
        phiSpan = tGeom->idToDet(DetId(*startDetIter))->surface().phiSpan();
        for(Cluster::const_iterator compDetIter=cluster.begin();compDetIter!=cluster.end();++compDetIter){
            phiSpanComp = tGeom->idToDet(DetId(*compDetIter))->surface().phiSpan();
            if(phiRangesOverlap(phiSpan,phiSpanComp)
                    && phiMoreClockwise(phiSpanComp.first,phiSpan.first)
                    && startDetIter != compDetIter)
            {
                ++counter;
                if(counter > cluster.size()){
                    cspan.phiSpan.first  =  std::numeric_limits<float>::epsilon();
                    cspan.phiSpan.second = -std::numeric_limits<float>::epsilon();
                    return;
                }
                startDetIter = compDetIter;break;
            } else if (compDetIter == cluster.end()-1){
                found = true;
            }
        }
    }
    cspan.phiSpan.first = phiSpan.first;
    // second with same method}
    found = false;
    Cluster::const_iterator endDetIter = cluster.begin();
    counter = 0;
    while(!found){
        phiSpan = tGeom->idToDet(DetId(*endDetIter))->surface().phiSpan();
        for(Cluster::const_iterator compDetIter=cluster.begin();compDetIter!=cluster.end();++compDetIter){
            phiSpanComp = tGeom->idToDet(DetId(*compDetIter))->surface().phiSpan();
            if(phiRangesOverlap(phiSpan,phiSpanComp)
                    && phiMoreCounterclockwise(phiSpanComp.second,phiSpan.second)
                    && endDetIter != compDetIter)
            {
                ++counter;
                if(counter > cluster.size()){
                    cspan.phiSpan.first  =  std::numeric_limits<float>::epsilon();
                    cspan.phiSpan.second = -std::numeric_limits<float>::epsilon();
                    return;
                }
                endDetIter = compDetIter;break;
            } else if (compDetIter == cluster.end()-1){
                found = true;
            }
        }
    }
    cspan.phiSpan.second = phiSpan.second;
}
void PixelBadModules::getZSpan(const Cluster & cluster, ClusterSpan & cspan){
    auto cmpFun = [this] (det_t detA, det_t detB){
        return
            trackerGeometry->idToDet(DetId(detA))->surface().zSpan().first
            <
            trackerGeometry->idToDet(DetId(detB))->surface().zSpan().first
            ;
    };
    
    auto minmaxIters = std::minmax_element(cluster.begin(),cluster.end(),cmpFun);
    cspan.zSpan.first = trackerGeometry->idToDet(DetId(*(minmaxIters.first)))->surface().zSpan().first;
    cspan.zSpan.second = trackerGeometry->idToDet(DetId(*(minmaxIters.second)))->surface().zSpan().second;
}
void PixelBadModules::getRSpan(const Cluster & cluster, ClusterSpan & cspan){
    auto cmpFun = [this] (det_t detA, det_t detB){
        return
            trackerGeometry->idToDet(DetId(detA))->surface().rSpan().first
            <
            trackerGeometry->idToDet(DetId(detB))->surface().rSpan().first
            ;
    };
    
    auto minmaxIters = std::minmax_element(cluster.begin(),cluster.end(),cmpFun);
    cspan.rSpan.first = trackerGeometry->idToDet(DetId(*(minmaxIters.first)))->surface().rSpan().first;
    cspan.rSpan.second = trackerGeometry->idToDet(DetId(*(minmaxIters.second)))->surface().rSpan().second;
}
void PixelBadModules::getSpan(const Cluster & cluster, ClusterSpan & cspan){
    auto firstDetIt = cluster.begin();
    if(firstDetIt != cluster.end()){
        cspan.subdetId = DetId(*firstDetIt).subdetId();
        if(cspan.subdetId == 1){
            cspan.layer = trackerTopology->pxbLayer(DetId(*firstDetIt));
            cspan.disk = 0;
            getPhiSpanBarrel(cluster,cspan);    
        }else if(cspan.subdetId == 2){
            cspan.disk = trackerTopology->pxfDisk(DetId(*firstDetIt));
            cspan.layer = 0;
            getPhiSpanEndcap(cluster,cspan);
        }
        getZSpan(cluster,cspan);
        getRSpan(cluster,cspan);
    }
}
PixelBadModules::ClusterSpanContainerPair PixelBadModules::clusterSpans(){
    ClusterSpanContainer cspansBarrel;
    ClusterSpanContainer cspansEndcap;
    ClusterContainer badClustersBar = badClustersBarrel();
    ClusterContainer badClustersEnd = badClustersEndcap();
    for(auto const & cluster : badClustersBar){
        ClusterSpan cspan;
        getSpan(cluster,cspan);
        cspansBarrel.push_back(cspan);
    }
    for(auto const & cluster : badClustersEnd){
        ClusterSpan cspan;
        getSpan(cluster,cspan);
        cspansEndcap.push_back(cspan);
    }
    return ClusterSpanContainerPair(cspansBarrel,cspansEndcap);
}
// Functions for findind overlapping functions
float PixelBadModules::zAxisIntersection(const float zrPointA[2], const float zrPointB[2]){
    return (zrPointB[0]-zrPointA[0])/(zrPointB[1]-zrPointA[1])*(-zrPointA[1])+zrPointA[0];
}
bool PixelBadModules::getZAxisOverlapRangeBarrel(const ClusterSpan & cspanA, const ClusterSpan & cspanB, std::pair<float,float> & range){
    ClusterSpan cspanUpper;
    ClusterSpan cspanLower;
    if(cspanA.rSpan.second < cspanB.rSpan.first){
        cspanLower = cspanA;
        cspanUpper = cspanB;
    }else if(cspanA.rSpan.first > cspanB.rSpan.second){
        cspanUpper = cspanA;
        cspanLower = cspanB;
    }else{
        return false;
    }
    float lower = 0;
    float upper = 0;
    if(cspanUpper.zSpan.second < cspanLower.zSpan.first){
        // lower intersectionpoint, point = {z,r} in cylindrical coordinates
        const float pointUpperClusterL[2] = {cspanUpper.zSpan.second, cspanUpper.rSpan.second};
        const float pointLowerClusterL[2] = {cspanLower.zSpan.first,  cspanLower.rSpan.first};
        lower = zAxisIntersection(pointUpperClusterL,pointLowerClusterL);
        // upper intersectionpoint
        const float pointUpperClusterU[2] = {cspanUpper.zSpan.first, cspanUpper.rSpan.first};
        const float pointLowerClusterU[2] = {cspanLower.zSpan.second,  cspanLower.rSpan.second};
        upper = zAxisIntersection(pointUpperClusterU,pointLowerClusterU);
    }else if (cspanUpper.zSpan.first <= cspanLower.zSpan.second && cspanLower.zSpan.first <= cspanUpper.zSpan.second){
        // lower intersectionpoint, point = {z,r} in cylindrical coordinates
        const float pointUpperClusterL[2] = {cspanUpper.zSpan.second, cspanUpper.rSpan.first};
        const float pointLowerClusterL[2] = {cspanLower.zSpan.first,  cspanLower.rSpan.second};
        lower = zAxisIntersection(pointUpperClusterL,pointLowerClusterL);
        // upper intersectionpoint
        const float pointUpperClusterU[2] = {cspanUpper.zSpan.first, cspanUpper.rSpan.first};
        const float pointLowerClusterU[2] = {cspanLower.zSpan.second,  cspanLower.rSpan.second};
        upper = zAxisIntersection(pointUpperClusterU,pointLowerClusterU);
    }else if (cspanUpper.zSpan.first > cspanLower.zSpan.second){
        // lower intersectionpoint, point = {z,r} in cylindrical coordinates
        const float pointUpperClusterL[2] = {cspanUpper.zSpan.second, cspanUpper.rSpan.first};
        const float pointLowerClusterL[2] = {cspanLower.zSpan.first,  cspanLower.rSpan.second};
        lower = zAxisIntersection(pointUpperClusterL,pointLowerClusterL);
        // upper intersectionpoint
        const float pointUpperClusterU[2] = {cspanUpper.zSpan.first, cspanUpper.rSpan.second};
        const float pointLowerClusterU[2] = {cspanLower.zSpan.second,  cspanLower.rSpan.first};
        upper = zAxisIntersection(pointUpperClusterU,pointLowerClusterU);
    }else{
        //something wrong
        return false;
    }
    range = std::pair<float,float>(lower,upper);
    return true;
}
bool PixelBadModules::getZAxisOverlapRangeEndcap(const ClusterSpan & cspanA, const ClusterSpan & cspanB, std::pair<float,float> & range){
    // While on left hand side of pixel detector
    ClusterSpan cspanNearer;
    ClusterSpan cspanFurther;
    float lower = 0;
    float upper = 0;
    if(cspanA.zSpan.first < 0 && cspanB.zSpan.first < 0){
        if(cspanA.zSpan.second < cspanB.zSpan.first){
            cspanFurther = cspanA;
            cspanNearer = cspanB;
        }else if (cspanB.zSpan.second < cspanA.zSpan.first){
            cspanFurther = cspanB;
            cspanNearer = cspanA;
        }else {
            //edm::LogPrint("") << "No overlap, same disk propably. Spans:";
            //Stream ss;
            //clusterSpanInfo(cspanA,ss);ss<<std::endl;clusterSpanInfo(cspanB,ss);ss<<std::endl;
            //edm::LogPrint("") << ss.str();ss.str(std::string());
            //edm::LogPrint("") << "**";
            return false;
        }
        if(cspanFurther.rSpan.second > cspanNearer.rSpan.first){
            const float pointA[2] = {cspanFurther.zSpan.second, cspanFurther.rSpan.second};
            const float pointB[2] = {cspanNearer.zSpan.first, cspanNearer.rSpan.first};
            lower = zAxisIntersection(pointA,pointB);
            if(cspanFurther.rSpan.first > cspanNearer.rSpan.second){
                const float pointC[2] = {cspanFurther.zSpan.first, cspanFurther.rSpan.first};
                const float pointD[2] = {cspanNearer.zSpan.second, cspanFurther.rSpan.second};
                upper = zAxisIntersection(pointC,pointD);
            }else{
                upper = std::numeric_limits<float>::infinity();
            }
        }else{
            //edm::LogPrint("") << "No overlap, further cluster is lower. Spans:";
            //Stream ss;
            //clusterSpanInfo(cspanA,ss);ss<<std::endl;clusterSpanInfo(cspanB,ss);ss<<std::endl;
            //edm::LogPrint("") << ss.str();ss.str(std::string());
            //edm::LogPrint("") << "**";
            return false;
        }
    }else if(cspanA.zSpan.first > 0 && cspanB.zSpan.first > 0){
        if(cspanA.zSpan.first > cspanB.zSpan.second ){
            cspanFurther = cspanA;
            cspanNearer = cspanB;
        }else if(cspanB.zSpan.first > cspanA.zSpan.second){
            cspanFurther = cspanB;
            cspanNearer = cspanA;
        }else{
            //edm::LogPrint("") << "No overlap, same disk propably. Spans:";
            //Stream ss;
            //clusterSpanInfo(cspanA,ss);ss<<std::endl;clusterSpanInfo(cspanB,ss);ss<<std::endl;
            //edm::LogPrint("") << ss.str();ss.str(std::string());
            //edm::LogPrint("") << "**";
            return false;
        }
        if(cspanFurther.rSpan.second > cspanNearer.rSpan.first){
            const float pointA[2] = {cspanFurther.zSpan.first, cspanFurther.rSpan.second};
            const float pointB[2] = {cspanNearer.zSpan.second, cspanNearer.rSpan.first};
            upper = zAxisIntersection(pointA,pointB);
            if(cspanFurther.rSpan.first > cspanNearer.rSpan.second){
                const float pointC[2] = {cspanFurther.zSpan.second, cspanFurther.rSpan.first};
                const float pointD[2] = {cspanNearer.zSpan.first, cspanFurther.rSpan.second};
                lower = zAxisIntersection(pointC,pointD);
            }else{
                lower = -std::numeric_limits<float>::infinity();
            }
        }else{
            //edm::LogPrint("") << "No overlap, further cluster lower. Spans:";
            //Stream ss;
            //clusterSpanInfo(cspanA,ss);ss<<std::endl;clusterSpanInfo(cspanB,ss);ss<<std::endl;
            //edm::LogPrint("") << ss.str();ss.str(std::string());
            //edm::LogPrint("") << "**";
            return false;
        }
    }else{       
        //edm::LogPrint("") << "No overlap, different sides of z axis. Spans:";
        //Stream ss;
        //clusterSpanInfo(cspanA,ss);ss<<std::endl;clusterSpanInfo(cspanB,ss);ss<<std::endl;
        //edm::LogPrint("") << ss.str();ss.str(std::string());
        //edm::LogPrint("") << "**";
        return false;
    }
    range = std::pair<float,float>(lower,upper);
    return true;
}

bool PixelBadModules::getZAxisOverlapRangeBarrelEndcap(const ClusterSpan & cspanBar, const ClusterSpan & cspanEnd, std::pair<float,float> & range){
    float lower = 0;
    float upper = 0;
    if(cspanEnd.rSpan.second > cspanBar.rSpan.first){
        if(cspanEnd.zSpan.second < cspanBar.zSpan.first){
            // if we are on the left hand side of pixel detector
            const float pointA[2] = {cspanEnd.zSpan.second, cspanEnd.rSpan.second};
            const float pointB[2] = {cspanBar.zSpan.first, cspanBar.rSpan.first};
            lower = zAxisIntersection(pointA,pointB);
            if(cspanEnd.rSpan.first > cspanBar.rSpan.second){
                // if does not overlap, then there is also upper limit
                const float pointC[2] = {cspanEnd.zSpan.first, cspanEnd.rSpan.first};
                const float pointD[2] = {cspanBar.zSpan.second, cspanBar.rSpan.second};
                upper = zAxisIntersection(pointC,pointD);
            }else{
                upper = std::numeric_limits<float>::infinity();
            }
        }else if (cspanEnd.zSpan.first > cspanBar.zSpan.second){
            // if we are on the right hand side of pixel detector
            const float pointA[2] = {cspanEnd.zSpan.first, cspanEnd.rSpan.second};
            const float pointB[2] = {cspanBar.zSpan.second, cspanBar.rSpan.first};
            upper = zAxisIntersection(pointA,pointB);
            if(cspanEnd.rSpan.first > cspanBar.rSpan.second){
                const float pointC[2] = {cspanEnd.zSpan.second,cspanEnd.rSpan.first};
                const float pointD[2] = {cspanBar.zSpan.first, cspanBar.rSpan.second};
                lower = zAxisIntersection(pointC,pointD);
            }else{
                lower = - std::numeric_limits<float>::infinity();
            }
        }else {
            return false;
        }
    }else{
        return false;
    } 
    range =  std::pair<float,float>(lower,upper);
    return true;
}
void PixelBadModules::compareClusterSpansBarrel(){
    Stream ss;
    ClusterSpanContainerPair cspans = clusterSpans();
    for(auto const & cspan : cspans.first){
        ss << std::endl;
        clusterSpanInfo(cspan,ss);
        edm::LogPrint("")<< ss.str();ss.str(std::string());
        for(auto const & cspanComp : cspans.first){
            if(&cspan == &cspanComp){continue;}
            if(phiRangesOverlap(cspan.phiSpan,cspanComp.phiSpan)){
                std::pair<float,float> range;
                if(getZAxisOverlapRangeBarrel(cspan,cspanComp,range)){

                }
            }
        }
    }
    for(auto const & cspan : cspans.second){
        ss << std::endl;
        clusterSpanInfo(cspan,ss);
        edm::LogPrint("")<< ss.str();ss.str(std::string());
        for(auto const & cspanComp : cspans.second){
            if(&cspan == &cspanComp){continue;}
            if(phiRangesOverlap(cspan.phiSpan,cspanComp.phiSpan)){
            }
        }
    }
    for(auto const & cspanBar : cspans.first){
        ss << std::endl;
        clusterSpanInfo(cspanBar,ss);
        edm::LogPrint("")<< ss.str();ss.str(std::string());
        for(auto const & cspanEnd : cspans.second){
            if(&cspanBar == &cspanEnd){continue;}
            if(phiRangesOverlap(cspanBar.phiSpan,cspanEnd.phiSpan));
        }
    }
}
PixelBadModules::OverlapSpansContainer PixelBadModules::overlappingSpans(float zAxisThreshold){    
    OverlapSpansContainer overlapSpansContainer;
    // find clusterSpans ie phi,r,z limits for detector clusters that are not working
    // returns pair where first is barrel spans and second endcap spans
    ClusterSpanContainerPair cspans = clusterSpans();

    // First comparison between barrel clusters
    for(ClusterSpanContainer::const_iterator barSpanIt = cspans.first.begin(); barSpanIt != cspans.first.end();++barSpanIt){
        OverlapSpans overlapSpans;
        for(ClusterSpanContainer::const_iterator compIt = barSpanIt+1;compIt != cspans.first.end();++compIt){
            if(phiRangesOverlap(barSpanIt->phiSpan,compIt->phiSpan)){
                std::pair<float,float> range(0,0);
                if(getZAxisOverlapRangeBarrel(*barSpanIt,*compIt,range)){
                    if(-zAxisThreshold <= range.second && range.first <= zAxisThreshold){
                        if(overlapSpans.empty()){
                            overlapSpans.push_back(*barSpanIt);
                        }
                        overlapSpans.push_back(*compIt);
                    }
                }
                
            }       
        }
        if(!overlapSpans.empty()){
            overlapSpansContainer.push_back(overlapSpans);
        }
    }


    // Then comparison between endcap  clusters
    for(ClusterSpanContainer::const_iterator endSpanIt = cspans.second.begin(); endSpanIt != cspans.second.end();++endSpanIt){
        OverlapSpans overlapSpans;
        for(ClusterSpanContainer::const_iterator compIt = endSpanIt+1;compIt != cspans.second.end();++compIt){
            if(phiRangesOverlap(endSpanIt->phiSpan,compIt->phiSpan)){
                std::pair<float,float> range(0,0);
                if(getZAxisOverlapRangeEndcap(*endSpanIt,*compIt,range)){
                    if(-zAxisThreshold <= range.second && range.first <= zAxisThreshold){
                        if(overlapSpans.empty()){
                            overlapSpans.push_back(*endSpanIt);
                        }
                        overlapSpans.push_back(*compIt);
                    }
                }
                
            }       
        }
        if(!overlapSpans.empty()){
            overlapSpansContainer.push_back(overlapSpans);
        }
    }



    return overlapSpansContainer;
}
DEFINE_FWK_MODULE(PixelBadModules);

