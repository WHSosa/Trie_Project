# Trie Autocomplete Project (C++17 + PostgreSQL + libpqxx)

A command-line Trie autocomplete system written in C++17, featuring persistent storage through PostgreSQL using the libpqxx driver.  
The application supports word insertion, autocomplete suggestions, frequency tracking, and viewing all words stored in the database.

---

## 1. Features

- **Trie-based data structure** for fast prefix matching  
- **PostgreSQL storage** to persist words and usage frequency  
- **libpqxx integration** for safe, modern C++ database access  
- **Interactive command menu** for inserting, searching, autocompleting, and listing words  
- **Clean architecture**, easy to extend and debug  

---

## 2. Requirements

Install required packages on Ubuntu:

```bash
sudo apt install g++ cmake postgresql libpqxx-dev
```

## 3. Database Setup
Switch to the PostgreSQL system user:

```bash
sudo -u postgres psql
```

```sql
    CREATE USER trie_user WITH PASSWORD '2024161143';
    CREATE DATABASE trie_db OWNER trie_user;
    GRANT ALL PRIVILEGES ON DATABASE trie_db TO trie_user;
    \q
```

## 4. Initialize the Database Schema

```bash
psql -U trie_user -d trie_db -f sql/init_trie_db.sql
2024161143
```

## 5. Build Instructions

```bash
mkdir build
cd build
cmake ..
make

./Trie_Project
```

You will see the menu:

Trie Autocomplete System
------------------------
1. Insert word
2. Search word
3. Autocomplete
4. View all words (from database)
0. Exit

## 7. Project Structure

Trie_Project/
├── cpp/
│   └── Trie_Project.cpp
├── sql/
│   └── init_trie_db.sql
├── CMakeLists.txt
├── README.md
└── .gitignore

## 8. Notes

Words inserted are stored both in-memory (Trie) and in PostgreSQL.
Autocomplete suggestions are sorted by frequency, making frequently used words appear first.
The system can be extended with:

JSON export/import
Trie compression
REST API (FastAPI or C++ Crow)
Qt GUI frontend