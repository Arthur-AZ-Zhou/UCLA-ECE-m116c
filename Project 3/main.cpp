#include "Protocol.cpp"

using namespace std;

vector<string> splitLine(string& line) {
    vector<string> result;
    string sanitizedLine = line;
    stringstream ss(sanitizedLine);
    string token;
    while (ss >> token)
        result.push_back(token);
    result[0].pop_back();
    return result;
}

int main(int argc, char* argv[]) {
    string input_file = argv[1];
    ifstream file(input_file);

    vector<vector<string>> instructions; // can be thought of as a vector<Instructions>
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
    
    Protocol protoc(coreIDs);
    for (vector<string>& instruction : instructions) {
        string coreID = instruction[0];
        string instruct = instruction[1];
        string tag = instruction[2];
        if (instruct == "read")
            protoc.processRead(coreID, tag);
        else
            protoc.processWrite(coreID, tag);
    }
    protoc.printStatistics();
}
