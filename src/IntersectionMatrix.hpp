#ifndef DYNAMICINDEX_INTERSECTIONMATRIX_HPP
#define DYNAMICINDEX_INTERSECTIONMATRIX_HPP

#include <unordered_map>
//#include <boost/serialization/access.hpp>
//#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/unordered_map.hpp>

class IntersectionMatrix: public std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> > {
public:
    // Constructors
    IntersectionMatrix() : std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> >() {};
    IntersectionMatrix(const std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> > &__u) : unordered_map(__u) {};

    // Functionality
    uint32_t get(uint32_t, uint32_t);
    void set(uint32_t, uint32_t, uint32_t);
    void increment(uint32_t, uint32_t);

    // Serialization
    //friend class boost::serialization::access;
    //template<class Archive>
    //void serialize(Archive & ar, const unsigned int version) {
    //    ar & boost::serialization::base_object<std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> > >(*this);
    //}

    // Parallelisation: merge results from different threads
    void merge(const IntersectionMatrix& other);
    
    // Analysis
    uint64_t get_sparsity() const;
};

#endif //DYNAMICINDEX_INTERSECTIONMATRIX_HPP
