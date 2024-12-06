#include "CacheProtocol.cpp"

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
    unordered_set<string> IDSet;
    for (vector<string>& instruction : instructions)
        IDSet.insert(instruction[0]);
    vector<string> IDs(IDSet.begin(), IDSet.end());
    sort(IDs.begin(), IDs.end());
    
    CacheProtocol protoc(IDs);
    for (vector<string>& instruction : instructions) {
        string ID = instruction[0];
        string instruct = instruction[1];
        string tag = instruction[2];
        if (instruct == "write") {
            protoc.processWrite(ID, tag);
        } else {
            protoc.processRead(ID, tag);
        }
    }

    cout << protoc.hits << endl;
    cout << protoc.misses << endl;
    cout << protoc.writeBacks << endl;
    cout << protoc.broadcasts << endl;
    cout << protoc.transfers;
    // cout << protoc.transfers << endl; //HOLY FCK THIS LINE SCREWED ME UP FOR SO LONG
}
