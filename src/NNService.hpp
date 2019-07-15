#ifndef NNSERVICE_H
#define NNSERVICE_H

//#include <boost/serialization/vector.hpp>
#include <cmath>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "IntersectionMatrix.hpp"

class NNService{
public:
    // Constructors and destructors
    NNService() {};
    NNService(const NNService&);
    ~NNService() {};

    // Main functionality
    void dynamic_index(const std::vector<std::pair<uint32_t, uint32_t> >&);
    std::unordered_map<uint32_t,std::vector<std::pair<double,uint32_t> > > compute_knn(const uint32_t&);

    // Setters and getters
    void set_recommendable(const std::vector<uint32_t>&);
    void set_timestamp(const uint64_t&);
    uint64_t get_timestamp();
    
    void drop_user(const uint32_t& u);
    void cleanIntersections();
    void becameNonRec(const uint32_t& u, const uint32_t& i);
    void becameRec(const uint32_t& u, const uint32_t& i);
    
    // Serialization
    //friend class boost::serialization::access;
    //template<class Archive>
    //void serialize(Archive & ar, const unsigned int version) {
    //    ar & _t0;
    //    ar & _m;
    //    ar & _item2count;
    //    ar & _user2rec;
    //    ar & _user2nonrec;
    //    ar & _item2recomm;
    //}

    void persist(const std::string&);

    // Parallelisation: merge results from different threads
    void merge(const NNService& other, bool disjoint_users = true);

//private:
    // Timestamp of previous iteration
    uint64_t _t0;

    // Placeholder for intersection size between items
    IntersectionMatrix _m;

    // Placeholder for item counts
    std::unordered_map<uint32_t, uint32_t> _item2count;

    // Placeholder for inverted index from users to items
    std::unordered_map<uint32_t, std::vector<uint32_t> > _user2rec;
    std::unordered_map<uint32_t, std::vector<uint32_t> > _user2nonrec;

    // Placeholder for item recommendability
    std::unordered_map<uint32_t, bool> _item2recomm;
};

#endif /* NNSERVICE_H */
