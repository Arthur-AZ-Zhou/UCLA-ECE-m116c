#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include "CacheManager.h"

using namespace std;

vector<string> splitLine(const string& line) {
    vector<string> result;
    string sanitizedLine = line;
    stringstream ss(sanitizedLine);
    string token;
    while (ss >> token)
        result.push_back(token);
    result[0].pop_back();
    return result;
}

void printInstruction(const vector<string>& instruction) {
    string coreID = instruction[0];
    string instruct = instruction[1];
    string tag = instruction[2];
    cout << coreID << " " << instruct << " " << tag << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Error: No input file provided. Usage: ./coherentsim <inputfile.txt>" << endl;
        return 1;
    }
    if (argc > 2) {
        cerr << "Error: Too many arguments provided. Usage: ./coherentsim <inputfile.txt>" << endl;
        return 1;
    }

    string input_file = argv[1];
    ifstream file(input_file);
    if (!file) {
        cerr << "Error: Could not open file " << input_file << endl;
        return 1;
    }

    vector<vector<string> > instructions; // can be thought of as a vector<Instructions>
    string line;
    while (getline(file, line)) {
        auto instruction_tokens = splitLine(line);
        instructions.push_back(instruction_tokens);
    }
    file.close();

    // Extract unique Core IDs
    unordered_set<string> coreIDSet;
    for (vector<string>& instruction : instructions)
        coreIDSet.insert(instruction[0]);
    vector<string> coreIDs(coreIDSet.begin(), coreIDSet.end());
    sort(coreIDs.begin(), coreIDs.end());
    
    CacheManager cm(coreIDs);
    for (vector<string>& instruction : instructions) {
        string coreID = instruction[0];
        string instruct = instruction[1];
        string tag = instruction[2];
        if (instruct == "read")
            cm.processRead(coreID, tag);
        else
            cm.processWrite(coreID, tag);
    }
    cm.printStatistics();
    return 0;
}
