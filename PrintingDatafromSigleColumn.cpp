//STEP  5: Reading data from a Single Column

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Read varint (simplified for SQLite varints)
unsigned readVarint(std::ifstream& file) {
    unsigned result = 0;
    for (int i = 0; i < 9; i++) {
        unsigned char byte;
        file.read(reinterpret_cast<char*>(&byte), 1);
        result = (result << 7) | (byte & 0x7F);
        if ((byte & 0x80) == 0) break;
    }
    return result;
}
// Read 2-byte big-endian
unsigned read2Bytes(std::ifstream& file, std::streampos offset) {
    file.seekg(offset);
    unsigned char bytes[2];
    file.read(reinterpret_cast<char*>(bytes), 2);
    return (bytes[0] << 8) | bytes[1];
}
// Extract and print one column from a table
void extractColumnData(std::ifstream& file, unsigned rootPage, unsigned pageSize, int columnIndex) {
    unsigned pageOffset = (rootPage - 1) * pageSize;
    file.seekg(pageOffset);

    unsigned char pageType;
    file.read(reinterpret_cast<char*>(&pageType), 1);

    if (pageType != 0x0D) {
        std::cout << "Only table leaf pages (0x0D) are supported for now.\n";
        return;
    }

    unsigned cellCount = read2Bytes(file, pageOffset + 3);
    std::vector<unsigned> cellOffsets(cellCount);

    file.seekg(pageOffset + 8);  // Cell pointers start at byte 8
    for (unsigned i = 0; i < cellCount; i++) {
        unsigned char offsetBytes[2];
        file.read(reinterpret_cast<char*>(offsetBytes), 2);
        cellOffsets[i] = (offsetBytes[0] << 8) | offsetBytes[1];
    }

    std::cout << "\nValues from column " << columnIndex << ":\n";
    for (unsigned offset : cellOffsets) {
        file.seekg(pageOffset + offset);

        readVarint(file); // payload size
        readVarint(file); // rowid

        std::streampos headerStart = file.tellg();
        unsigned headerSize = readVarint(file);

        std::vector<unsigned> serialTypes;
        while ((file.tellg() - headerStart) < headerSize) {
            serialTypes.push_back(readVarint(file));
        }

        file.seekg(headerStart + static_cast<std::streamoff>(headerSize));
        std::vector<std::string> values;

        for (unsigned serial : serialTypes) {
            if (serial == 1) {
                char val;
                file.read(&val, 1);
                values.push_back(std::to_string(val));
            } else if (serial == 2) {
                unsigned char val[2];
                file.read(reinterpret_cast<char*>(val), 2);
                int intVal = (val[0] << 8) | val[1];
                values.push_back(std::to_string(intVal));
            } else if (serial == 4) {
                unsigned char val[4];
                file.read(reinterpret_cast<char*>(val), 4);
                int intVal = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                values.push_back(std::to_string(intVal));
            } else if (serial >= 13) {
                unsigned len = (serial - 13) / 2;
                std::string text(len, '\0');
                file.read(&text[0], len);
                values.push_back(text);
            } else {
                values.push_back(""); // unsupported
            }
        }

        if (columnIndex < values.size())
            std::cout << values[columnIndex] << "\n";
        else
            std::cout << "[N/A]\n";
    }
}

int main() {
    std::ifstream file("CampusInfo.db", std::ios::binary);
    if (!file) {
        std::cout << "Failed to open file.\n";
        return 1;
    }

    file.seekg(16);
    unsigned char pageSizeBytes[2];
    file.read(reinterpret_cast<char*>(pageSizeBytes), 2);
    unsigned pageSize = (pageSizeBytes[0] << 8) | pageSizeBytes[1];

    file.seekg(103);
    unsigned char cellCountBytes[2];
    file.read(reinterpret_cast<char*>(cellCountBytes), 2);
    unsigned numCells = (cellCountBytes[0] << 8) | cellCountBytes[1];

    file.seekg(108);
    std::vector<unsigned> cellOffsets(numCells);
    for (unsigned i = 0; i < numCells; i++) {
        unsigned char offsetBytes[2];
        file.read(reinterpret_cast<char*>(offsetBytes), 2);
        cellOffsets[i] = (offsetBytes[0] << 8) | offsetBytes[1];
    }

    std::vector<std::pair<std::string, unsigned>> tableList;

    for (unsigned offset : cellOffsets) {
        file.seekg(offset);
        readVarint(file); // payload
        readVarint(file); // rowid

        std::streampos headerStart = file.tellg();
        unsigned headerSize = readVarint(file);
        std::vector<unsigned> serialTypes;
        for (int i = 0; i < 5; i++)
            serialTypes.push_back(readVarint(file));
file.seekg(headerStart + static_cast<std::streamoff>(headerSize));
        std::vector<std::string> values;
        for (int i = 0; i < 5; i++) {
            if (serialTypes[i] == 1) {
                char val;
                file.read(&val, 1);
                values.push_back(std::to_string(val));
            } else if (serialTypes[i] == 2) {
                unsigned char val[2];
                file.read(reinterpret_cast<char*>(val), 2);
                int intVal = (val[0] << 8) | val[1];
                values.push_back(std::to_string(intVal));
            } else if (serialTypes[i] == 4) {
                unsigned char val[4];
                file.read(reinterpret_cast<char*>(val), 4);
                int intVal = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                values.push_back(std::to_string(intVal));
            } else if (serialTypes[i] >= 13) {
                unsigned len = (serialTypes[i] - 13) / 2;
                std::string text(len, '\0');
                file.read(&text[0], len);
                values.push_back(text);
            } else {
                values.push_back("");
            }
        }
        if (values.size() >= 2 && values[0] == "table") {
            std::string tableName = values[1];
            unsigned rootPage = std::stoi(values[3]);
            tableList.push_back({tableName, rootPage});
        } }

    if (tableList.empty()) {
        std::cout << "No tables found in database.\n";
        return 1;
    }
    std::cout << "\nAvailable tables:\n";
    for (int i = 0; i < tableList.size(); i++) {
        std::cout << i << ": " << tableList[i].first << "\n";
    }
    int choice;
    std::cout << "Enter table index to extract column from: ";
    std::cin >> choice;

    if (choice < 0 || choice >= tableList.size()) {
        std::cout << "Invalid choice.\n";
        return 1;
    }

    int colIndex;
    std::cout << "Enter column index to retrieve (starting from 0): ";
    std::cin >> colIndex;

    extractColumnData(file, tableList[choice].second, pageSize, colIndex);
    file.close();
    return 0;  }

