#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <omp.h>
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

std::unordered_map<uint32_t, uint32_t> parse_item_origins(std::string filename) {
    // Placeholder for result
    std::unordered_map<uint32_t, uint32_t> item2origin;

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
        // Parse item id and timestamp
        uint32_t item = std::stoi(line.substr(0, line.find(',')));
        uint32_t origin = std::stoi(line.substr(line.find(',') + 1,line.size()));

        // Save origin for item
        item2origin[item] = origin;
    }
    // Close buffer
    fb.close();

    return item2origin;
}

std::vector<uint32_t> filter_recommendable_items(const std::unordered_map<uint32_t, uint32_t>& item2origin, const uint32_t& rec_begin, const uint32_t& rec_end) {
    // Placeholder for results
    std::vector<uint32_t> recommendable;

    // For every item
    for(auto i2o: item2origin) {
        if(rec_begin < i2o.second && i2o.second <= rec_end) recommendable.push_back(i2o.first);
    }

    return recommendable;
}

std::unordered_map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> split_events(std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> events, uint32_t num_chunks) {
    // Placeholder for result
    std::unordered_map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>> chunk2events;

    // For every event, add it to the right chunk
    for(auto event: events) chunk2events[std::get<1>(event) % num_chunks].emplace_back(std::get<0>(event), std::get<1>(event));

    return chunk2events;
}

int main(int argc, char* argv[]) {
    // Placeholder for commandline arguments
    std::string name;
    std::string pageviews;
    std::string item_origins;
    uint32_t num_threads;
    uint32_t batch_size;
    uint32_t recommendability_seconds;

    // Parse commandline arguments
    if(argc != 7) {
        std::cout << "Usage: " << argv[0] << " <name> <pageviews> <item_origins> <batch_size> <num_threads> <recommendability_seconds>" << std::endl;
        return 0;
    } else {
        name = std::string(argv[1]);
        pageviews = std::string(argv[2]);
        item_origins = std::string(argv[3]);
        batch_size = std::stoi(argv[4]);
        num_threads = std::stoi(argv[5]);
        recommendability_seconds = std::stoi(argv[6]);
    }

    // Set number of threads over which to parallelise
    omp_set_num_threads(num_threads);

    // Parse and store events
    auto events = parse_events(pageviews);
   
    // Parse and store item origins
    auto item2origin = parse_item_origins(item_origins);

    // Stringstream for output
    std::stringstream output_file;
    output_file << name << "_" << batch_size << "_" << num_threads << "_" << recommendability_seconds << ".csv";
    std::ofstream output;
    output.open(output_file.str());
    output << "num_events,iterative,incremental,num_recommendable\n";
    output << "0,0,0,0" << std::endl;
   
    // Placeholder for runtime of last incremental computation
    uint32_t last_incremental_run = 0;

    // Rerun the entire algorithm at every batch_size events
    for(uint32_t num_events = batch_size; num_events <= events.size(); num_events += batch_size) {
        std::cout << num_events << " events..." << std::endl;

        // Copy data, keep first 'num_events' events
        auto first_events = events;
        first_events.resize(num_events);

        // Extract latest timestamp
        uint32_t rec_end = std::get<2>(first_events.at(first_events.size()-1));

        // Compute the recommendability range
        uint32_t rec_begin = rec_end - recommendability_seconds;

        // Filter out set of recommendable items
        auto recommendable = filter_recommendable_items(item2origin, rec_begin, rec_end);

        // Generate data chunks for every thread
        auto chunk2events = split_events(first_events, num_threads);

        // Placeholder for results from every thread
        std::vector<std::shared_ptr<NNService>> results;

        // Process every chunk in parallel
        auto start = std::chrono::system_clock::now();
        #pragma omp parallel for
        for(uint32_t chunk_id = 0; chunk_id < num_threads; chunk_id++) {
            //NNService nn;
            std::shared_ptr<NNService> nn = std::make_shared<NNService>();
            nn->set_recommendable(recommendable);
            
            // Dynamically index events
            nn->dynamic_index(chunk2events[chunk_id]);
            results.push_back(nn);
        }
        // Merge results for all chunks
        for(int iteration = num_threads / 2; iteration >= 1 ; iteration /= 2) {
            #pragma omp parallel for
            for(int i = 0; i < iteration; ++i) {
                (*results.at(i)).merge(*results.at(i+iteration));
            }
        }
        auto end = std::chrono::system_clock::now();
        auto elapsed_seconds = end-start;
        std::cout<< "\tDynamic index finished in " << static_cast<uint32_t>(elapsed_seconds.count()/1000000000.0) << " seconds!" << std::endl;
        output << num_events << "," << static_cast<uint32_t>(elapsed_seconds.count()/1000000000.0) << "," << last_incremental_run << "," << recommendable.size() << std::endl; 

    }
    output.close();
}

