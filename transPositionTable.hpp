#pragma once

#include <cstdint>
#include <exception>
#include <vector>
#include <limits>


enum EntryType : uint8_t {
    EXACT,      
    LOWERBOUND, 
    UPPERBOUND  
};


struct TTEntry {
    uint64_t key;       
    int32_t value;         
    int16_t depth;     
    EntryType type;      
    uint64_t best_move;    
    uint8_t generation;    // How old is this entry?
};


class TranspositionTable {
    static constexpr size_t TABLE_SIZE = 1 << 20; // 1M entries
    static constexpr uint64_t KEY_MASK = 0xFFFFFFFFFFFF0000;
    std::vector<TTEntry> table;
    uint8_t current_generation = 0;

public:
    TranspositionTable() : table(TABLE_SIZE) {}
    
    void clear() {
        std::fill(table.begin(), table.end(), TTEntry{});
    }
    
    void store(uint64_t hash, int depth, int value, EntryType type, uint64_t best_move) {
        TTEntry& entry = table[hash % TABLE_SIZE];
        if (entry.depth <= depth || entry.generation != current_generation) {
            entry = {
                hash & KEY_MASK,
                static_cast<int32_t>(value),
                static_cast<int16_t>(depth),
                type,
                best_move,
                current_generation
            };
        }
    }
    
    bool probe(uint64_t hash, int depth, int& alpha, int& beta, int& value, uint64_t& best_move) const {
        const TTEntry& entry = table[hash % TABLE_SIZE];
        
        if ((entry.key ^ hash) & KEY_MASK) return false;
        if (entry.depth == 0 && entry.type == EXACT && entry.value == 0) return false;
        
        best_move = entry.best_move;
        
        if (entry.depth >= depth) {
            switch (entry.type) {
                case EXACT:
                    value = entry.value;
                    return true;
                case LOWERBOUND:
                    alpha = std::max(alpha, entry.value);
                    break;
                case UPPERBOUND:
                    beta = std::min(beta, entry.value);
                    break;
            }
            if (alpha >= beta) {
                value = entry.value;
                return true;
            }
        }
        return false;
    }
    
    void new_search() {
        current_generation = (current_generation + 1) % 256; // 0-255
    }
};



