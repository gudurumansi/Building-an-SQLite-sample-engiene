//STEP 4: Printing No of Rows in a Table 
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

// Function to read a 2-byte value
unsigned read2Bytes(std::ifstream &file, std::streampos offset) {
    file.seekg(offset);
    unsigned char bytes[2];
    file.read(reinterpret_cast<char*>(bytes), 2);
    return (bytes[0] << 8) | bytes[1];
}

// Function to count rows
int countRows(std::ifstream& file, unsigned rootPage, unsigned pageSize) {
    int rowCount = 0;
    unsigned pageOffset = (rootPage - 1) * pageSize; // root page starts from 1, not 0

    file.seekg(pageOffset);
    unsigned char pageType;
    file.read(reinterpret_cast<char*>(&pageType), 1);

    if (pageType == 0x0D) { // Table Leaf page
        unsigned cellCount = read2Bytes(file, pageOffset + 3); // Offset 3: number of cells
        rowCount = cellCount;
    }
    return rowCount;
}

int main() {
    std::ifstream file("CampusInfo.db", std::ios::binary);
    if (!file) {
        std::cout << "Failed to open file.\n";
        return 1;
    }

    // Read page size
    file.seekg(16);
    unsigned char pageSizeBytes[2];
    file.read(reinterpret_cast<char*>(pageSizeBytes), 2);
    unsigned pageSize = (pageSizeBytes[0] << 8) | pageSizeBytes[1];
    std::cout << "Page size: " << pageSize << " bytes\n";

    // Read number of cells
    file.seekg(103);
    unsigned char cellCountBytes[2];
    file.read(reinterpret_cast<char*>(cellCountBytes), 2);
    unsigned numCells = (cellCountBytes[0] << 8) | cellCountBytes[1];

    // Read cell offsets
    file.seekg(108);
    std::vector<unsigned> cellOffsets(numCells);
    for (unsigned i = 0; i < numCells; i++) {
        unsigned char offsetBytes[2];
        file.read(reinterpret_cast<char*>(offsetBytes), 2);
        cellOffsets[i] = (offsetBytes[0] << 8) | offsetBytes[1];
    }

    unsigned rootPage = 0;

    for (unsigned offset : cellOffsets) {
        file.seekg(offset);

        // Skip payload size and rowid
        readVarint(file);
        readVarint(file);

        // Header
        std::streampos headerStart = file.tellg();
        unsigned headerSize = readVarint(file);

        // Read 5 serial types
        std::vector<unsigned> serialTypes;
        for (int i = 0; i < 5; i++) {
            serialTypes.push_back(readVarint(file));
        }

        // Go to body
        file.seekg(headerStart + static_cast<std::streamoff>(headerSize));

        // Read actual values
        std::vector<std::string> values;
        for (int i = 0; i < 5; i++) {
            if (serialTypes[i] == 1) { // Integer 1 byte
                char val;
                file.read(&val, 1);
                values.push_back(std::to_string(val));
            }
            else if (serialTypes[i] == 2) { // Integer 2 bytes
                unsigned char val[2];
                file.read(reinterpret_cast<char*>(val), 2);
                int intVal = (val[0] << 8) | val[1];
                values.push_back(std::to_string(intVal));
            }
            else if (serialTypes[i] == 4) { // Integer 4 bytes
                unsigned char val[4];
                file.read(reinterpret_cast<char*>(val), 4);
                int intVal = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                values.push_back(std::to_string(intVal));
            }
            else if (serialTypes[i] >= 13) { // Text
                unsigned len = (serialTypes[i] - 13) / 2;
                std::string text(len, '\0');
                file.read(&text[0], len);
                values.push_back(text);
            }
            else {
                values.push_back(""); // Not supported type
            }
        }

        if (values.size() >= 2 && values[0] == "table") {
            std::cout << "- Table name: " << values[1] << "\n";
            rootPage = std::stoi(values[3]); // values[3] now correctly has the root page as number
            std::cout << "Root Page: " << rootPage << "\n";
            int rows = countRows(file, rootPage, pageSize);
            std::cout << "Total Rows: " << rows << "\n";
        }
    }

    if (rootPage == 0) {
        std::cout << "No table found.\n";
        file.close();
        return 1;
    }

    file.close();
    return 0; }

