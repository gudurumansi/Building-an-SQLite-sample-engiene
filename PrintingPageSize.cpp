//STEP-1:Printing Page Size 
#include <iostream>
#include <fstream>

int getPageSize(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file!\n";
        return -1;
    }

    file.seekg(16); // Move to byte 16 (where page size is stored)
    unsigned char buffer[2]; 
    file.read(reinterpret_cast<char *>(buffer), 2); // Read 2 bytes

    // Convert Big-Endian to integer
    int pageSize = (buffer[0] << 8) | buffer[1];

    file.close();
    return pageSize;
}

int main() {
    std::string dbFile = "CampusInfo.db"; // Path to your SQLite database file
    int pageSize = getPageSize(dbFile);
    if (pageSize > 0)
        std::cout << "Page Size: " << pageSize << " bytes\n";
    return 0;
}


