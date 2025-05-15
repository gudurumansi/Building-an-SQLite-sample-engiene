
# 🗂️ Build Your Own SQLite (C++ Implementation)

This project is a simplified version of **SQLite** built from scratch using **C++**. It explores how databases work internally by simulating core SQLite functionality like file storage, B+ trees for indexing, and parsing SQL-like commands.

📁 All C++ source codes and the sample database files are organized by different development stages in this repository.

---

## 🚀 Project Objectives

- Understand the **internal architecture** of SQLite.
- Implement key components like:
  - File/page management
  - SQL command parsing
  - B+ Tree indexing
  - Record insertion, retrieval, and update
- Create a lightweight database engine capable of executing simple SQL commands.

---

## 🧠 Key Concepts Implemented

- ✅ Custom file storage structure  
- ✅ Page & block management  
- ✅ B+ Tree indexing from scratch  
- ✅ Manual binary parsing of SQLite header  
- ✅ Execution of basic SQL commands (`CREATE`, `INSERT`, `SELECT`, `UPDATE`)  
- ✅ Use of `sqlite_master` parsing for metadata  

---

## 📂 Repository Structure



📦 your-own-sqlite/
├── Stage1\_ReadBinaryHeader.cpp     # Parses the SQLite file header
├── Stage2\_ExtractTables.cpp        # Reads sqlite\_master table to get table metadata
├── Stage3\_BPlusTree.cpp            # B+ Tree implementation
├── Stage4\_SQLParser.cpp            # SQL command handling
├── Stage5\_QueryExecutor.cpp        # Query execution logic
├── database/
│   └── sample.db                   # Sample SQLite database used for testing
├── README.md                       # Project documentation
└── ...


## 🛠️ How to Run

1. **Clone the repository:**


git clone https://github.com/your-username/your-own-sqlite.git
cd your-own-sqlite

2. **Compile the required stage:**


g++ Stage1_ReadBinaryHeader.cpp -o stage1
./stage1

> You can compile and run each stage individually depending on what you want to test.


## 📊 Sample Output

The project prints the number of tables, page layout, and B+ tree structure during each stage, giving a transparent view of how internal operations take place.


## 📌 Future Enhancements

* Implement support for more SQL commands (`DELETE`, `JOIN`, etc.)
* Optimize B+ tree balancing
* Add user interface for query input
* Extend support for multiple table operations

## 👩‍💻 Author

**Mansi**
## 📃 License

This project is licensed under the MIT License. Feel free to use and modify it for educational or personal use.

