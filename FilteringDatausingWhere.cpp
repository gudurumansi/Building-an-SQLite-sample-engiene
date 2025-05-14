//STEP-8: Filtering Data Using a Where Clause 

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
// Read varint (simplified)
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
void extractFilteredColumns(
 std::ifstream& file, unsigned rootPage, unsigned pageSize,
 const std::vector<int>& columnIndices, int filterCol, const std::string& filterVal,
 bool applyFilter
) {
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
 file.seekg(pageOffset + 8);
 for (unsigned i = 0; i < cellCount; i++) {
 unsigned char offsetBytes[2];
 file.read(reinterpret_cast<char*>(offsetBytes), 2);
 cellOffsets[i] = (offsetBytes[0] << 8) | offsetBytes[1];
 }
 std::cout << "\nFiltered Results:\n---------------------------\n";
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
 values.push_back("");
 }
 }
 if (applyFilter) {
 if (filterCol >= values.size() || values[filterCol] != filterVal) {
 continue; // Skip non-matching row
 }
 }
 for (int colIdx : columnIndices) {
 if (colIdx < values.size())
 std::cout << values[colIdx] << " | ";
 else
 std::cout << "[N/A] | ";
 }
 std::cout << "\n";
 }
}
#include <limits> // for numeric_limits
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
 std::cout << "Page size: " << pageSize << " bytes\n";
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
 }
 }
 if (tableList.empty()) {
 std::cout << "No tables found in database.\n";
 return 1;
 }
 std::cout << "\nAvailable tables:\n";
 for (int i = 0; i < tableList.size(); i++) {
 std::cout << i << ": " << tableList[i].first << "\n";
 }
 int choice;
 std::cout << "Enter table index to extract from: ";
 std::cin >> choice;
 if (std::cin.fail() || choice < 0 || choice >= static_cast<int>(tableList.size())) {
 std::cin.clear();
 std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
 std::cout << "Invalid table choice.\n";
 return 1;
 }
 std::cin.ignore(); // clear newline
 std::string input;
 std::cout << "Enter comma-separated column indices (e.g., 0,2): ";
 std::getline(std::cin, input);
 std::stringstream ss(input);
 std::string token;
 std::vector<int> selectedCols;
 while (std::getline(ss, token, ',')) {
 int colIdx = std::stoi(token);
 if (colIdx < 0 ||colIdx>tableList.size()) {
 std::cout << "Invalid column index " << "\n";
 return 1;
 }
 selectedCols.push_back(colIdx);
 }
 char filterChoice;
 std::cout << "Do you want to filter with a WHERE clause? (y/n): ";
 std::cin >> filterChoice;
 if (filterChoice == 'y' || filterChoice == 'Y') {
 int filterColumn;
 std::string filterValue;
 std::cout << "Enter column index to filter by: ";
 std::cin >> filterColumn;
 if (std::cin.fail() || filterColumn < 0) {
 std::cout << "Invalid column index for filtering.\n";
 return 1;
 }
 std::cin.ignore(); // clear newline
 std::cout << "Enter value to match: ";
 std::getline(std::cin, filterValue);
 extractFilteredColumns(file, tableList[choice].second, pageSize, selectedCols, filterColumn,
filterValue, true);
 } else {
 extractFilteredColumns(file, tableList[choice].second, pageSize, selectedCols, -1, "", false);
 }
 return 0;
}

