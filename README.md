#  Version Control System (C++)

A mini Git-like version control system built from scratch in **C++** to demonstrate the practical use of **Data Structures and Algorithms (DSA)** in managing project files, commits, and restore operations.

---

##  Features

- **Repository Management** – Initialize, manage, and update a simple repository.  
- **Commit System** – Tracks and stores file changes with timestamps and messages.  
- **Restore Points** – Allows reverting to previous versions using stored commits.  
- **Log Viewer** – Displays the commit history and related metadata.  
- **Modular Codebase** – Separated into multiple components for maintainability.  
- **Custom CLI Interface** – Easy-to-use text-based interface for interaction.  

---

## DSA Concepts Used

This project emphasizes the **application of DSA concepts** rather than library-based solutions.

| Concept | Implementation |
|----------|----------------|
| **Linked Lists** | To store commit history and maintain sequential references to previous commits. |
| **Stacks** | Used for implementing the restore mechanism — undoing commits in reverse order. |
| **File Handling & Buffers** | Reading/writing repository states efficiently. |
| **Maps / Hash Tables** | For storing commit metadata (e.g., commit IDs → file states). |
| **Recursion** | In traversing and displaying commit trees. |
| **Dynamic Memory Allocation** | For managing data structures during commit and restore operations. |
| **Algorithms** | Searching commits, comparing file states, and optimizing restore operations. |

---

##  Project Structure
```
├── include/
│ ├── CommitManager.h
│ ├── Repository.h
│ ├── LogViewer.h
│ ├── Restore.h
│
├── src/
│ ├── main.cpp
│ ├── CommitManager.cpp
│ ├── Repository.cpp
│ ├── LogViewer.cpp
│ ├── Restore.cpp
│
├── build/
│ ├── (compiled object files + executable)
│
├── Makefile
└── README.md

```

---

## ⚙️ Build & Run

### **To build the project**
```bash
mingw32-make
```
### **To run**
```bash
./build/vcs
````
### **To clean build files**
```bash
mingw32-make clean
````
## **Learning Outcomes**
Understanding how version control systems work internally.

Applying DSA concepts in real-world software scenarios.

Learning modular programming and build automation using Makefiles.

Gaining insight into file I/O, system design, and memory management in C++.

