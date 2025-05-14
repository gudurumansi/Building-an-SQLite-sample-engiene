//STEP-2: Printing Number of Tables
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

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

    // Read number of cells (bytes 100+3 = 103, 2 bytes)
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

    unsigned tableCount = 0;
  // Process each cell
    for (unsigned i = 0; i < numCells; i++) {
        // Go to that cell
        file.seekg(cellOffsets[i]);
        // Skip payload size and rowid
        readVarint(file);
        readVarint(file);
        // Record header
        std::streampos headerStart = file.tellg();
        unsigned headerSize = readVarint(file);

        // Serial type for first field ("type" string)
        unsigned serialType = readVarint(file);
        unsigned typeLength = (serialType - 13) / 2;

        // Jump to start of record body
        file.seekg(headerStart + static_cast<std::streamoff>(headerSize));

        // Read "type" string
        std::string type(typeLength, '\0');
        file.read(&type[0], typeLength);

        if (type == "table") {
            tableCount++;
        }
    }
    std::cout << "Number of tables: " << tableCount << "\n";
    file.close();
    return 0;
}
