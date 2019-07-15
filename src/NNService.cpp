#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <functional>
#include "NNService.hpp"

void NNService::dynamic_index(const std::vector<std::pair<uint32_t, uint32_t> >& events) {
    // For every event
    for(auto iterator: events) {
        // Extract item i
        uint32_t i = iterator.first;
        // Increment count of i
        _item2count[i]++;
        // Extract user u
        uint32_t u = iterator.second;
        // Recommendability status of i
        bool i_r = _item2recomm[i];
        // All recommendable items seen by the same user get a similarity boost
        for (auto j: _user2rec[u]) {
            // Update intersection between items
            _m.increment(i, j);
        }
        // All non-recommendable items as well, if i is recommendable
        if(i_r) {
            for (auto j: _user2nonrec[u]) {
                // Update intersection between items
                _m.increment(i, j);
            }
            // Add i to recommendable items seen by user u
            _user2rec[u].emplace_back(i);
        } else {
            _user2nonrec[u].emplace_back(i);
        }
    }
}

std::unordered_map<uint32_t,std::vector<std::pair<double,uint32_t>>> NNService::compute_knn(const uint32_t& k) {
    // Initialize results
    std::unordered_map<uint32_t,std::vector<std::pair<double,uint32_t>>> item2knn;
    
    // For every non-zero entry in the matrix
    for(auto i_j_v: _m) {
        for(auto j_v: i_j_v.second) {
            if(_item2recomm[i_j_v.first]) item2knn[j_v.first].emplace_back(j_v.second/sqrt(_item2count[i_j_v.first]),i_j_v.first);
            if(_item2recomm[j_v.first]) item2knn[i_j_v.first].emplace_back(j_v.second/sqrt(_item2count[j_v.first]),j_v.first);
        }
    }
    // Cut-off at k
    for(auto& i_knn: item2knn) {
        if(i_knn.second.size() > k) {
            auto& knn = i_knn.second;
            std::nth_element(knn.begin(), knn.begin() + k, knn.end(), std::greater<std::pair<double,uint32_t>>());
        }
    }
    return item2knn;
}

void NNService::set_recommendable(const std::vector<uint32_t>& items) {
    // Clear previous mapping
    _item2recomm.clear();
    // Set all new items to true
    for(auto i: items) this->_item2recomm[i] = true;
}

void NNService::set_timestamp(const uint64_t& t1) {
    _t0 = t1;
}

uint64_t NNService::get_timestamp() {
    return _t0;
}

void NNService::persist(const std::string& location) {
    std::ofstream os;
    os.open(location + "_norms.csv");
    os << "item,norm" << std::endl;
    for(auto item_norm: _item2count) os << item_norm.first << "," << item_norm.second << std::endl;
    os.close();
    os.open(location + "_intersections.csv");
    os << "i,j,intersection" << std::endl;
    for(auto item_item2intersect: _m){
        for(auto item_intersect: item_item2intersect.second) {
            os << item_item2intersect.first << "," << item_intersect.first << ","<< item_intersect.second << std::endl;
        }
    }
    os.close();
}

void NNService::drop_user(const uint32_t& u) {
    _user2rec.erase(u);
    _user2nonrec.erase(u);
}

void NNService::becameNonRec(const uint32_t& u, const uint32_t& i) {
    auto hist_it = _user2rec[u].end();
    for(auto j_it = _user2rec[u].begin(); j_it != _user2rec[u].end(); j_it++) {
        if((*j_it) == i) {
            hist_it = j_it;
            break;
        }
    }
    _user2rec[u].erase(hist_it);
    _user2nonrec[u].emplace_back(i);
}
    
void NNService::cleanIntersections() {
    for(auto i_j_v: _m) for(auto j_v: i_j_v.second) if(!(_item2recomm[i_j_v.first]) && !(_item2recomm[j_v.first])) _m[i_j_v.first].erase(j_v.first);
}

void NNService::becameRec(const uint32_t& u, const uint32_t& i) {
    auto hist_it = _user2nonrec[u].end();
    for(auto j_it = _user2nonrec[u].begin(); j_it != _user2nonrec[u].end(); j_it++) {
        if((*j_it) == i) hist_it = j_it;
        else _m.increment(i,(*j_it));
    }
    _user2nonrec[u].erase(hist_it);
    _user2rec[u].emplace_back(i);
}

void NNService::merge(const NNService& other, bool disjoint_users) {
    // Merge Intersection matrices
    this->_m.merge(other._m);

    // Merge item counts
    for(auto i_c: other._item2count) this->_item2count[i_c.first] += i_c.second;

    // If user sets are disjoint
    if(disjoint_users) {
        // Merge seen recommendable items
        this->_user2rec.insert(other._user2rec.begin(), other._user2rec.end());

        // Merge seen non-recommendable items
        this->_user2nonrec.insert(other._user2nonrec.begin(), other._user2nonrec.end());
    } else { // User sets are not disjoint
        // For all users that appear in 'other'
        // Compute intersection between recommendable items they have seen in 'this' and all items they have seen in 'other'
        for(auto user_rec: other._user2rec) {
            for(auto i: user_rec.second) {
                for(auto j: this->_user2rec[user_rec.first]) this->_m.increment(i, j);
                for(auto j: this->_user2nonrec[user_rec.first]) this->_m.increment(i, j);
            }
        }
        // Compute intersection between non-recommendable items they have seen in 'this' and recommendable items they have seen in 'other'
        for(auto user_rec: other._user2nonrec) {
            for(auto i: user_rec.second) {
                for(auto j: this->_user2rec[user_rec.first]) this->_m.increment(i, j);
                //for(auto j: this->_user2nonrec[user_rec.first]) this->_m.increment(i, j);
            }
        }
        // For all users that appear in 'other', merge inverted indices with existing ones for recommendable items
        for(auto user_rec: other._user2rec) this->_user2rec[user_rec.first].insert(this->_user2rec[user_rec.first].end(),user_rec.second.begin(),user_rec.second.end());
        // For all users that appear in 'other', merge inverted indices with existing ones for non-recommendable items
        for(auto user_rec: other._user2nonrec) this->_user2nonrec[user_rec.first].insert(this->_user2nonrec[user_rec.first].end(),user_rec.second.begin(),user_rec.second.end());
    }
}
