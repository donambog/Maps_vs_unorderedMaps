#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>

// Function to measure execution time
template<typename Func>
double measureTime(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Generate a random word
std::string generateRandomWord(std::mt19937& gen, int length) {
    std::string characters = "abcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<> distrib(0, characters.size() - 1);
    
    std::string word;
    for (int i = 0; i < length; ++i) {
        word += characters[distrib(gen)];
    }
    return word;
}

// Scenario 1: Spell Checker
// This scenario demonstrates a use case where individual key lookups are the primary operation
void testSpellCheckerScenario(int numWords, int numSearches) {
    std::cout << "\n=== SCENARIO 1: SPELL CHECKER ===" << std::endl;
    std::cout << "Number of words in dictionary: " << numWords << std::endl;
    std::cout << "Number of searches: " << numSearches << std::endl;
    
    // Data generation
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::vector<std::string> dictionaryWords;
    std::vector<std::string> wordsToSearch;
    
    // Create dictionary
    for (int i = 0; i < numWords; ++i) {
        dictionaryWords.push_back(generateRandomWord(gen, 8));
    }
    
    // Create words to search (50% in dictionary, 50% outside dictionary)
    for (int i = 0; i < numSearches/2; ++i) {
        wordsToSearch.push_back(dictionaryWords[i % dictionaryWords.size()]);
    }
    for (int i = 0; i < numSearches/2; ++i) {
        wordsToSearch.push_back(generateRandomWord(gen, 8));
    }
    
    // Test with ordered map
    std::map<std::string, bool> dictionaryMap;
    double loadTimeMap = measureTime([&]() {
        for (const auto& word : dictionaryWords) {
            dictionaryMap[word] = true;
        }
    });
    
    double searchTimeMap = measureTime([&]() {
        int numFound = 0;
        for (const auto& word : wordsToSearch) {
            if (dictionaryMap.find(word) != dictionaryMap.end()) {
                numFound++;
            }
        }
    });
    
    // Test with unordered_map
    std::unordered_map<std::string, bool> dictionaryUnorderedMap;
    double loadTimeUnorderedMap = measureTime([&]() {
        for (const auto& word : dictionaryWords) {
            dictionaryUnorderedMap[word] = true;
        }
    });
    
    double searchTimeUnorderedMap = measureTime([&]() {
        int numFound = 0;
        for (const auto& word : wordsToSearch) {
            if (dictionaryUnorderedMap.find(word) != dictionaryUnorderedMap.end()) {
                numFound++;
            }
        }
    });
    
    // Display results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\nResults (time in ms):" << std::endl;
    std::cout << "                    | map      | unordered_map | Improvement factor" << std::endl;
    std::cout << "--------------------+----------+---------------+------------------------" << std::endl;
    std::cout << "Loading time        | " << std::setw(8) << loadTimeMap 
              << " | " << std::setw(13) << loadTimeUnorderedMap 
              << " | " << std::setw(24) << loadTimeMap / loadTimeUnorderedMap << "x" << std::endl;
    std::cout << "Search time         | " << std::setw(8) << searchTimeMap 
              << " | " << std::setw(13) << searchTimeUnorderedMap 
              << " | " << std::setw(24) << searchTimeMap / searchTimeUnorderedMap << "x" << std::endl;
}

// Scenario 2: Time-based Reservation System
// This scenario demonstrates a use case where range queries and ordered data are important
void testTimeReservationScenario(int numReservations, int numRanges) {
    std::cout << "\n=== SCENARIO 2: TIME-BASED RESERVATION SYSTEM ===" << std::endl;
    std::cout << "Number of reservations: " << numReservations << std::endl;
    std::cout << "Number of time ranges to search: " << numRanges << std::endl;
    
    // Data generation
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_int_distribution<> timeDistrib(0, 86400 * 365); // Time over a year in seconds
    
    std::vector<time_t> timestamps;
    for (int i = 0; i < numReservations; ++i) {
        timestamps.push_back(timeDistrib(gen));
    }
    
    // Generate time ranges to search
    std::vector<std::pair<time_t, time_t>> ranges;
    for (int i = 0; i < numRanges; ++i) {
        time_t start = timeDistrib(gen);
        time_t end = start + timeDistrib(gen) % 86400; // Range of max 24h
        ranges.push_back({start, end});
    }
    
    // Test with ordered map
    std::map<time_t, int> reservationsMap;
    double loadTimeMap = measureTime([&]() {
        for (int i = 0; i < numReservations; ++i) {
            reservationsMap[timestamps[i]] = i;
        }
    });
    
    // Measuring range search in ordered map - O(log n) complexity
    double rangeSearchTimeMap = measureTime([&]() {
        int totalReservationsFound = 0;
        for (const auto& [start, end] : ranges) {
            auto it_start = reservationsMap.lower_bound(start);
            auto it_end = reservationsMap.upper_bound(end);
            
            for (auto it = it_start; it != it_end; ++it) {
                totalReservationsFound++;
            }
        }
    });
    
    // Measuring time to traverse ordered map (already sorted)
    double traversalTimeMap = measureTime([&]() {
        std::vector<std::pair<time_t, int>> sortedReservations;
        for (const auto& [time, id] : reservationsMap) {
            sortedReservations.push_back({time, id});
        }
    });
    
    // Test with unordered_map
    std::unordered_map<time_t, int> reservationsUnorderedMap;
    double loadTimeUnorderedMap = measureTime([&]() {
        for (int i = 0; i < numReservations; ++i) {
            reservationsUnorderedMap[timestamps[i]] = i;
        }
    });
    
    // Measuring range search in unordered_map - O(n) complexity
    double rangeSearchTimeUnorderedMap = measureTime([&]() {
        int totalReservationsFound = 0;
        for (const auto& [start, end] : ranges) {
            for (const auto& [time, id] : reservationsUnorderedMap) {
                if (time >= start && time <= end) {
                    totalReservationsFound++;
                }
            }
        }
    });
    
    // Measuring time to sort data from unordered_map (requires explicit sorting)
    double traversalTimeUnorderedMap = measureTime([&]() {
        std::vector<std::pair<time_t, int>> sortedReservations;
        for (const auto& [time, id] : reservationsUnorderedMap) {
            sortedReservations.push_back({time, id});
        }
        std::sort(sortedReservations.begin(), sortedReservations.end());
    });
    
    // Display results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\nResults (time in ms):" << std::endl;
    std::cout << "                    | map      | unordered_map | Difference factor" << std::endl;
    std::cout << "--------------------+----------+---------------+----------------------" << std::endl;
    std::cout << "Loading time        | " << std::setw(8) << loadTimeMap 
              << " | " << std::setw(13) << loadTimeUnorderedMap 
              << " | " << std::setw(22) << loadTimeMap / loadTimeUnorderedMap << "x" << std::endl;
    std::cout << "Range search        | " << std::setw(8) << rangeSearchTimeMap 
              << " | " << std::setw(13) << rangeSearchTimeUnorderedMap 
              << " | " << std::setw(22) << rangeSearchTimeUnorderedMap / rangeSearchTimeMap << "x" << std::endl;
    std::cout << "Sorting data        | " << std::setw(8) << traversalTimeMap 
              << " | " << std::setw(13) << traversalTimeUnorderedMap 
              << " | " << std::setw(22) << traversalTimeUnorderedMap / traversalTimeMap << "x" << std::endl;
}

int main() {
    std::cout << "PERFORMANCE COMPARISON: MAP vs. UNORDERED_MAP" << std::endl;
    
    // Scenario 1: Spell Checker (favors unordered_map)
    testSpellCheckerScenario(100000, 50000);
    
    // Scenario 2: Time Reservation System (favors map)
    testTimeReservationScenario(100000, 1000);
    
    return 0;
}
