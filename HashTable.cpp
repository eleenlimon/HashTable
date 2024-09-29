//============================================================================
// Name        : HashTable.cpp
// Author      : Eleen Limon
// Version     : 1.0
// Copyright   : Copyright � 2023 SNHU COCE
// Description : Lab 4-2 Hash Table
//============================================================================

#include <algorithm>
#include <climits>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include "CSVparser.hpp"

using namespace std;

//============================================================================
// Global definitions visible to all methods and classes
//============================================================================

const unsigned int DEFAULT_SIZE = 179; // Default hash table size

// Forward declaration of utility function
double strToDouble(string str, char ch);

// Define a structure to hold bid information
struct Bid {
    string bidId;    // unique identifier for the bid
    string title;    // title of the bid
    string fund;     // fund associated with the bid
    double amount;   // bid amount

    // Default constructor to initialize bid amount to 0.0
    Bid() {
        amount = 0.0;
    }
};

//============================================================================
// Hash Table class definition
//============================================================================

class HashTable {
private:
    // Internal structure for linked list within hash table buckets
    struct Node {
        Bid bid;             // Bid object stored in this node
        unsigned int key;    // Hash key for the bid
        Node* next;          // Pointer to the next node in the chain

        // Default constructor
        Node() {
            key = UINT_MAX;  // Maximum unsigned int value, represents an empty key
            next = nullptr;  // Initialize next pointer to null
        }

        // Constructor with a bid object
        Node(Bid aBid) : Node() {
            bid = aBid;
        }

        // Constructor with a bid object and key
        Node(Bid aBid, unsigned int aKey) : Node(aBid) {
            key = aKey;
        }
    };

    vector<Node> nodes;       // Vector of nodes representing hash table buckets
    unsigned int tableSize = DEFAULT_SIZE;  // Size of the hash table

    // Private hash function to generate a key
    unsigned int hash(int key);

public:
    // Constructors and destructor
    HashTable();                        // Default constructor
    HashTable(unsigned int size);       // Constructor with specified table size
    virtual ~HashTable();               // Destructor

    // Public methods
    void Insert(Bid bid);               // Insert a bid into the hash table
    void PrintAll();                    // Print all bids stored in the hash table
    void Remove(string bidId);          // Remove a bid by bidId
    Bid Search(string bidId);           // Search for a bid by bidId
    size_t Size();                      // Return the size of the hash table
};

//============================================================================
// Hash Table method implementations
//============================================================================

// Default constructor, initializes the hash table with the default size
HashTable::HashTable() {
    nodes.resize(tableSize); // Resize the nodes vector to the default size
}

// Constructor with specified table size
HashTable::HashTable(unsigned int size) {
    this->tableSize = size;
    nodes.resize(tableSize);  // Resize the nodes vector to the given size
}

// Destructor
HashTable::~HashTable() {
    nodes.clear();  // Clear the hash table
}

// Hash function that returns a key based on the table size
unsigned int HashTable::hash(int key) {
    return key % tableSize;  // Simple modulo-based hash function
}

// Insert a bid into the hash table
void HashTable::Insert(Bid bid) {
    // Generate hash key from bidId
    unsigned int key = hash(atoi(bid.bidId.c_str()));
    Node* prev = nullptr;
    Node* node = &nodes.at(key);

    // If the bucket is empty, insert the bid here
    if (node->key == UINT_MAX) {
        node->key = key;
        node->bid = bid;
        node->next = nullptr;
    } else {
        // If there's a collision, traverse the chain to find the right spot
        while (node != nullptr && node->bid.bidId != bid.bidId) {
            prev = node;
            node = node->next;
        }

        // If bidId is not already present, add a new node
        if (node == nullptr) {
            node = new Node(bid, key);
            prev->next = node;
        }
    }
}

// Print all bids stored in the hash table
void HashTable::PrintAll() {
    for (unsigned int i = 0; i < nodes.size(); ++i) {
        Node* node = &nodes.at(i);

        // If the bucket is not empty, print the bid information
        if (node->key != UINT_MAX) {
            cout << "Key " << i << ": " << node->bid.bidId << " | " << node->bid.title << " | "
                 << node->bid.amount << " | " << node->bid.fund << endl;

            // Traverse and print any chained nodes
            node = node->next;
            while (node != nullptr) {
                cout << "   " << node->bid.bidId << " | " << node->bid.title << " | "
                     << node->bid.amount << " | " << node->bid.fund << endl;
                node = node->next;
            }
        }
    }
}

// Remove a bid from the hash table by bidId
void HashTable::Remove(string bidId) {
    unsigned int key = hash(atoi(bidId.c_str()));
    Node* node = &nodes.at(key);
    Node* prev = nullptr;

    // Traverse the chain to find the bid to remove
    while (node != nullptr && node->bid.bidId != bidId) {
        prev = node;
        node = node->next;
    }

    // If bid is found, remove it from the chain
    if (node != nullptr) {
        if (prev == nullptr) {
            nodes.at(key).key = UINT_MAX;
            nodes.at(key).next = nullptr;
        } else {
            prev->next = node->next;
            delete node;
        }
    }
}

// Search for a bid by bidId and return it
Bid HashTable::Search(string bidId) {
    Bid bid;
    unsigned int key = hash(atoi(bidId.c_str()));
    Node* node = &nodes.at(key);

    // Traverse the chain to find the bid
    while (node != nullptr) {
        if (node->bid.bidId == bidId) {
            return node->bid;  // Return the found bid
        }
        node = node->next;
    }

    return bid;  // Return an empty bid if not found
}

//============================================================================
// Utility functions
//============================================================================

// Function to display a bid's details
void displayBid(Bid bid) {
    cout << bid.bidId << ": " << bid.title << " | " << bid.amount << " | " << bid.fund << endl;
}

// Function to load bids from a CSV file into the hash table
void loadBids(string csvPath, HashTable* hashTable, int &bidCount) {
    cout << "Loading CSV file " << csvPath << endl << endl;

    csv::Parser file = csv::Parser(csvPath);

    // Initialize bidCount
    bidCount = 0;

    try {
        // Iterate through CSV rows and load each bid into the hash table
        for (unsigned int i = 0; i < file.rowCount(); i++) {
            Bid bid;
            bid.bidId = file[i][1];           // Read bidId
            bid.title = file[i][0];           // Read title
            bid.fund = file[i][8];            // Read fund
            bid.amount = strToDouble(file[i][4], '$'); // Convert amount to double
            hashTable->Insert(bid);           // Insert bid into hash table
            bidCount++;                       // Increment bid count
        }
    } catch (csv::Error& e) {
        cerr << e.what() << endl;  // Catch and print any CSV parsing errors
    }
}

// Function to convert string to double by removing a specific character
double strToDouble(string str, char ch) {
    str.erase(remove(str.begin(), str.end(), ch), str.end());
    return atof(str.c_str());
}

//============================================================================
// Main function
//============================================================================

int main(int argc, char* argv[]) {
    string csvPath, bidKey;
    switch (argc) {
    case 2:
        csvPath = argv[1];  // Command-line argument for CSV path
        bidKey = "98223";   // Default bidKey
        break;
    case 3:
        csvPath = argv[1];  // Command-line argument for CSV path
        bidKey = argv[2];   // Command-line argument for bidKey
        break;
    default:
        csvPath = "eBid_Monthly_Sales.csv";  // Default file path
        bidKey = "98223";                    // Default bidKey
    }

    clock_t ticks;          // For measuring time
    HashTable* bidTable = new HashTable();  // Create a new hash table
    Bid bid;                // To store the bid being searched for

    int choice = 0;
    while (choice != 9) {
        // Menu
        cout << "Menu:" << endl;
        cout << "  1. Load Bids" << endl;
        cout << "  2. Display All Bids" << endl;
        cout << "  3. Find Bid" << endl;
        cout << "  4. Remove Bid" << endl;
        cout << "  9. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            ticks = clock();  // Start clock
            int bidCount = 0; // Variable to hold bid count
            loadBids(csvPath, bidTable, bidCount);
            ticks = clock() - ticks;  // Stop clock
            cout << bidCount << " bids read" << endl;
            cout << "time: " << ticks << " clock ticks" << endl;
            cout << "time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
            break;
        }

        case 2:
            bidTable->PrintAll();  // Print all bids in the table
            break;

        case 3:
            ticks = clock();  // Start clock
            bid = bidTable->Search(bidKey);  // Search for a bid by bidKey
            ticks = clock() - ticks;  // Stop clock
            if (!bid.bidId.empty()) {
                displayBid(bid);  // Display bid details if found
            } else {
                cout << "Bid Id " << bidKey << " not found." << endl;
            }
            cout << "time: " << ticks << " clock ticks" << endl;
            cout << "time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
            break;

        case 4:
            cout << "Enter Bid Id to remove: ";
            cin >> bidKey;
            bidTable->Remove(bidKey);  // Remove the bid
            cout << "Bid Id " << bidKey << " removed." << endl;
            break;

        case 9:
            cout << "Good bye." << endl;
            break;

        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    delete bidTable;  // Clean up and delete the hash table
    return 0;
}
