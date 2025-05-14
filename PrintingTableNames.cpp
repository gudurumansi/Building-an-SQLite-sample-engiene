//STEP-3: Printing Table Names

#include <iostream>
#include <fstream>
#include <vector>

// Function to read varint (just enough for our use)
unsigned readVarint(std::ifstream &file) {
    unsigned result = 0;
    for (int i = 0; i < 9; i++) {
        unsigned char byte;
        file.read(reinterpret_cast<char*>(&byte), 1);
        result = (result << 7) | (byte & 0x7F);
        if ((byte & 0x80) == 0) break;
    }
    return result;
}

int main() {
    std::ifstream file("CampusInfo.db", std::ios::binary);
    if (!file) {
        std::cout << "Failed to open file.\n";
        return 1;
    }

    // Read page size (bytes 16–17)
    file.seekg(16);
    unsigned char pageSizeBytes[2];
    file.read(reinterpret_cast<char*>(pageSizeBytes), 2);
    unsigned pageSize = (pageSizeBytes[0] << 8) | pageSizeBytes[1];
    std::cout << "Page size: " << pageSize << " bytes\n";

    // Read number of cells (bytes 103–104)
    file.seekg(103);
    unsigned char cellCountBytes[2];
    file.read(reinterpret_cast<char*>(cellCountBytes), 2);
    unsigned numCells = (cellCountBytes[0] << 8) | cellCountBytes[1];

    // Read cell pointer array (starts at offset 108)
    file.seekg(108);
    std::vector<unsigned> cellOffsets(numCells);
    for (unsigned i = 0; i < numCells; i++) {
        unsigned char offsetBytes[2];
        file.read(reinterpret_cast<char*>(offsetBytes), 2);
        cellOffsets[i] = (offsetBytes[0] << 8) | offsetBytes[1];
    }

    std::cout << "Number of tables and their names:\n";

    for (unsigned offset : cellOffsets) {
        file.seekg(offset);

        // Skip payload size and rowid
        readVarint(file);
        readVarint(file);
        // Header info
        std::streampos headerStart = file.tellg();
        unsigned headerSize = readVarint(file);
       // Read serial types of first 5 columns in sqlite_master
        std::vector<unsigned> serialTypes;
        for (int i = 0; i < 5; i++) {
            serialTypes.push_back(readVarint(file));
        }
        // Go to start of body
        file.seekg(headerStart + static_cast<std::streamoff>(headerSize));

        // Read actual values based on serial types
        std::vector<std::string> values;
        for (unsigned st : serialTypes) {
            unsigned len = (st >= 13) ? (st - 13) / 2 : 0;
            std::string value(len, '\0');
            file.read(&value[0], len);
            values.push_back(value);
        }
        if (values.size() >= 2 && values[0] == "table") {
            std::cout << "- " << values[1] << "\n";  // values[1] = table name
        }
    }
    file.close();
    return 0;}

