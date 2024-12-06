#include "CacheProtocol.cpp"

using namespace std;

vector<string> splitInstructionToVector(string instruction) { //chatgpt helped generate this
    vector<string> answer;
    stringstream ss(instruction); // Directly use the input instruction
    string token;

    // Tokenize the string
    while (ss >> token) {
        answer.push_back(token); // Remove the last character of the first token, if it's non-empty
    }

    // for (int i = 0; i < answer.size(); i++) {
    //     cout << answer[i] << " ";
    // }
    // cout << endl;

    return answer;
}

vector<vector<string>> readInstructionsFromFile (string filename) {
    ifstream fileReader(filename);
    vector<vector<string>> cacheInstructions;
    string line;
    
    while (getline(fileReader, line)) {
        cacheInstructions.push_back(splitInstructionToVector(line));
    }
    
    fileReader.close();
    return cacheInstructions;
}

vector<string> getUniqueSortedIDs(vector<vector<string>> cacheInstructions) {
    unordered_set<string> uniqueCacheIDs;
    
    for (int i = 0; i < cacheInstructions.size(); i++) {
        uniqueCacheIDs.insert(cacheInstructions[i][0]);
    }

    // for (auto& instruction : cacheInstructions) {
    //     cout << "instruction: " << instruction << endl;
    // }
    
    vector<string> sortedIDs(uniqueCacheIDs.begin(), uniqueCacheIDs.end());
    sort(sortedIDs.begin(), sortedIDs.end()); //FRIEND ADVISED ME TO SORT SINCE IT WAS FAILING CAMPUSWIRE
    
    return sortedIDs;
}

int main(int argc, char* argv[]) {
    vector<vector<string>> cacheInstructions = readInstructionsFromFile(argv[1]);
    vector<string> uniqueIDs = getUniqueSortedIDs(cacheInstructions);
    CacheProtocol protoc(uniqueIDs); //create cache protocol based on uniqueIDs
    
    for (int i = 0; i < cacheInstructions.size(); i++) {        
        if (cacheInstructions[i][1] == "write") {
            protoc.writeInstruction(cacheInstructions[i][0], cacheInstructions[i][2]);
        } else {
            protoc.readInstruction(cacheInstructions[i][0], cacheInstructions[i][2]);
        }
    }

    // for (auto& instruction : cacheInstructions) {        
    //     if (instruction == "write") {
    //         cout << "TRIGGER WRITE" << endl;
    //         protoc.writeInstruction(instruction[0], instruction[2]);
    //     } else {
    //         protoc.readInstruction(instruction[0], instruction[2]);
    //     }
    // }
    
    cout << protoc.hits << endl;
    cout << protoc.misses << endl;
    cout << protoc.writeBacks << endl;
    cout << protoc.broadcasts << endl;
    cout << protoc.transfers;
    // cout << protoc.transfers << endl; //HOLY FCK THIS LINE SCREWED ME UP FOR SO LONG
}