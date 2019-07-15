#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <omp.h>
#include <set>
#include <sstream>
#include <stdio.h>
#include <unordered_map>

#include "NNService.hpp"

std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> parse_events(std::string filename) {
    // Placeholder for result
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> events;

    // File buffer
    std::filebuf fb;
    fb.open(filename, std::ios::in);

    // Input stream
    std::istream is(&fb);

    // Read header
    std::string line;
    std::getline(is, line);

    // Read rest of file
    while(std::getline(is, line)){
        // Parse user and item id
        uint32_t user = std::stoi(line.substr(0, line.find(',')));
        uint32_t item = std::stoi(line.substr(line.find(',') + 1,line.size()));
        uint32_t tstamp = std::stoi(line.substr(line.rfind(',') + 1,line.size()));

        // Add pair to events
        events.emplace_back(item,user,tstamp);
    }
    // Close buffer
    fb.close();

    return events;
}

std::unordered_map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> split_events(std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> events, uint32_t num_chunks) {
    // Placeholder for result
    std::unordered_map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> chunk2events;

    // For every event, add it to the right chunk
    for(auto event: events) chunk2events[std::get<1>(event) % num_chunks].emplace_back(std::get<0>(event), std::get<1>(event));

    return chunk2events;
}

std::unordered_map<uint32_t, std::unordered_map<uint32_t,double>> baseline_similarity_join(const std::vector<std::pair<uint32_t, uint32_t>>& events) {
    // Build inverted indices from items to users
    std::unordered_map<uint32_t, std::set<uint32_t>> i2u;
    for(auto i_u: events) i2u[i_u.first].insert(i_u.second);

    // Placeholder for result
    std::unordered_map<uint32_t, std::unordered_map<uint32_t,double>> i2j2s;

    // For every item-pair
    std::unordered_map<uint32_t, std::set<uint32_t>>::iterator i_iter;
    for(i_iter = i2u.begin(); i_iter != i2u.end(); ++i_iter){
        // Copy iterator - use symmetry of similarity in a (semi-)smart way
        std::unordered_map<uint32_t, std::set<uint32_t>>::iterator j_iter;
        for(auto j_iter = i2u.begin(); i_iter != j_iter; ++j_iter) {
            // Compute cosine similarity between i and j
            double cos;
            uint32_t intersection = 0;
            // Compute size of intersection of U_i and U_j
            auto i_u_iter = i_iter->second.begin();
            auto j_v_iter = j_iter->second.begin();
            while(i_u_iter != i_iter->second.end() && j_v_iter != j_iter->second.end()){
                // Extract u and v
                auto u = *i_u_iter;
                auto v = *j_v_iter;
                // Either u == v
                if(u == v) {
                    // Increment Intersection
                    ++intersection;
                    // Increment both iterators
                    ++i_u_iter;
                    ++j_v_iter;
                } else if (u < v) {
                    // Increment iterator over u
                    ++i_u_iter;
                } else if (v < u) {
                    ++j_v_iter;
                }
            }
            if(intersection > 0) {
                cos = intersection / (sqrt(i_iter->second.size()) * sqrt(j_iter->second.size()));
                i2j2s[i_iter->first][j_iter->first] = cos;
            }
        }
    }
    return i2j2s;
}

std::unordered_map<uint32_t, std::unordered_map<uint32_t,double>> sparse_baseline_similarity_join(const std::vector<std::pair<uint32_t, uint32_t>>& events) {
    // Build inverted indices from items to users and from users to items
    std::unordered_map<uint32_t, std::set<uint32_t>> i2u;
    std::unordered_map<uint32_t, std::set<uint32_t>> u2i;
    for(auto i_u: events){
        i2u[i_u.first].insert(i_u.second);
        u2i[i_u.second].insert(i_u.first);
    }

    // Placeholder for result
    std::unordered_map<uint32_t, std::unordered_map<uint32_t,double>> i2j2s;

    // For every item
    std::unordered_map<uint32_t, std::set<uint32_t>>::iterator i_iter;
    for(i_iter = i2u.begin(); i_iter != i2u.end(); ++i_iter){
        uint32_t i = i_iter->first;
        // For every user that has seen that item
        for(auto u: i_iter->second){
            // For every item that user has seen
            for(auto j: u2i[u]) {
                // Exploit symmetry
                if(i <= j) { 
                    ++i2j2s[i][j];
                }
            }
        }
    }

    // Compute actual similarity
    for(auto i_j_s: i2j2s) {
        uint32_t i = i_j_s.first;
        for(auto j_s: i_j_s.second) {
            uint32_t j = j_s.first;
            i2j2s[i][j] /= (sqrt(i2u[i].size()) * sqrt(i2u[j].size()));
        }
    }
    return i2j2s;
}


int main(int argc, char* argv[]) {
    // Placeholder for commandline arguments
    std::string name;
    std::string pageviews;
    uint32_t batch_size;
    uint32_t num_threads;

    // Parse commandline arguments
    if(argc != 5) {
        std::cout << "Usage: " << argv[0] << " <name> <pageviews> <batch_size> <num_threads>" << std::endl;
        return 0;
    } else {
        name = std::string(argv[1]);
        pageviews = std::string(argv[2]);
        batch_size = std::stoi(argv[3]);
        num_threads = std::stoi(argv[4]);
    }

    // Set number of threads over which to parallelise
    omp_set_num_threads(num_threads);

    // Parse and store events
    auto events = parse_events(pageviews);
   
    // Stringstream for output
    std::stringstream output_file;
    output_file << name << "_" << batch_size << "_baseline.csv";
    std::ofstream output;
    output.open(output_file.str());
    output << "num_events,sparse_baseline,dynamic_index" << std::endl;
    output << "0,0,0" << std::endl;
   
    // Rerun the entire algorithm at every batch_size events
    for(uint32_t num_events = batch_size; num_events <= events.size(); num_events += batch_size) {
        std::cout << num_events << " events..." << std::endl;
        output << num_events << ",";

        // Copy data, keep first 'num_events' events
        auto first_events = events;
        first_events.resize(num_events);

        // Generate data chunks for every thread
        auto chunk2events = split_events(first_events, num_threads);
        
        // Run baseline algorithm
        {
            auto start = std::chrono::system_clock::now();
            std::unordered_map<uint32_t, std::unordered_map<uint32_t,double>> i2j2s = sparse_baseline_similarity_join(chunk2events[0]);
            auto end = std::chrono::system_clock::now();
            auto elapsed_seconds = end-start;
            std::cout<< "\tSparse baseline algorithm finished in " << static_cast<uint32_t>(elapsed_seconds.count()/1000000000.0) << " seconds!" << std::endl;
            output << static_cast<uint32_t>(elapsed_seconds.count()/1000000000.0) << ",";
        }

        // Run dynamic algorithm
        {
            NNService nn;
            auto start = std::chrono::system_clock::now();
            nn.dynamic_index(chunk2events[0]);
            auto end = std::chrono::system_clock::now();
            auto elapsed_seconds = end-start;
            std::cout<< "\tDynamic index algorithm finished in " << static_cast<uint32_t>(elapsed_seconds.count()/1000000000.0) << " seconds!" << std::endl;
            output << static_cast<uint32_t>(elapsed_seconds.count()/1000000000.0) << std::endl;
        }
    }
    output.close();
}
