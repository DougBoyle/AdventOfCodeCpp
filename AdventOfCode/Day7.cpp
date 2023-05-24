#include <iostream>
#include <fstream>
#include <string> // getline
#include <set>
#include <limits>

using namespace std;

class File {
private:
    const int size;
    const string filename;
public:
    File(int size, string filename) : size{ size }, filename{ filename } {}

    struct FileComparator {
        bool operator()(const File& f1, const File& f2) const {
            return f1.filename < f2.filename;
        }
    };

    int getSize() const {
        return size;
    }
};

class Directory {
private:
    Directory* const parent;
    const string dirname;
    mutable int cachedSize = -1;

    struct DirectoryComparator {
        bool operator()(Directory* d1, Directory* d2) const {
            return d1->dirname < d2->dirname;
        }
    };

    // Can potentially contain both a directory and file of the same name
    set<Directory*, DirectoryComparator> subdirs;
    set<File, File::FileComparator> files;

    void calcSize() const {
        cachedSize = 0;
        for (const Directory* d : subdirs) {
            cachedSize += d->getSize();
        }

        for (const File f : files) {
            cachedSize += f.getSize();
        }
    }

    Directory* findDir(string path) {
        auto it = find_if(subdirs.begin(), subdirs.end(), [path](const Directory* d) {
            return d->dirname == path;
        });

        if (it != subdirs.end()) {
            return *it;
        }
        else {
            return nullptr;
        }
    }

public:
    Directory(Directory* parent, string dirname) : parent{ parent }, dirname { dirname } {}

    ~Directory() {
        for (Directory* d : subdirs) {
            delete d;
        }
    }

    // Copy constructors take const arg, move constructors take non-const arg (else may as well just copy!)

    // Copy constructors implicitly deleted by definition of move operations: https://en.cppreference.com/w/cpp/language/copy_constructor
    // Prevent copying or assignment i.e. file systems are singletons, with a sole copy represented by the root 
  //  Directory(const Directory& other) = delete;
  //  Directory& operator=(const Directory& other) = delete;
    
    // Fine to move around (not too expensive, only the root is stack allocated), just don't create a copy.
    // Would probably be better to just make all of these deleted and return a pointer to the root rather than the actual thing!
    Directory(Directory&& other) = default;
    Directory& operator=(Directory&& other) = default;


    Directory* addDir(const string name) {
        if (Directory* d = findDir(name)) return d;

        Directory* dir = new Directory(this, name);

        subdirs.insert(dir);

        return dir;
    }

    void addFile(File f) {
        files.insert(f);
    }

    // Assumes we cd through one directory at a time.
    // (else would need to split on '/', and handle root directory specially)
    Directory* cd(const string &path) {
        if (path == "..") {
            return parent;
        }
        else {
            return addDir(path);
        }
    }

    int getSize() const {
        if (cachedSize < 0) calcSize();

        return cachedSize;
    }

    template<typename func_t>
    void forEachDir(func_t f) const {
        // pre-order traversal
        f(this);
        for (Directory* d : subdirs) {
            d->forEachDir(f);
        }
    }
};

namespace day7 {

    Directory readRoot() {
        ifstream input("Day7.txt");

        Directory root((Directory*)nullptr, "/");

        Directory* current = &root;


        string s;
        input >> s; // "$", always start while loop on the next command
        while (input >> s) {
            if (s == "cd") {
                input >> s;
                current = current->cd(s);
                input >> s; // "$" for next command
            }
            else if (s == "ls") {
                string dirOrSize, filename;
                while (input >> dirOrSize) {
                    if (dirOrSize == "$") break; // end of output, next command
                    input >> filename;
                    if (dirOrSize == "dir") {
                        current->addDir(filename);
                    }
                    else {
                        current->addFile(File(stoi(dirOrSize), filename));
                    }
                }

            }
            else {
                throw invalid_argument("unknown command " + s);
            }
        }

        return root;
    }

    void part1() {
        Directory root = readRoot();

        int totalSize = 0;
        root.forEachDir([&totalSize](const Directory* d) {
            int dirSize = d->getSize();
            if (dirSize <= 100000) totalSize += dirSize;
        });

        cout << totalSize << endl; // 1444896
    }

    void part2() {
        Directory root = readRoot();

        int currentSize = root.getSize();

        int maxSize = 70000000 - 30000000;

        int toRemove = currentSize - maxSize;

        int smallestSuitable = std::numeric_limits<int>::max();

        root.forEachDir([&smallestSuitable, toRemove](const Directory* d) {
            int dirSize = d->getSize();
        if (dirSize >= toRemove && dirSize < smallestSuitable) smallestSuitable = dirSize;
        });

        cout << smallestSuitable << endl; // 404395
    }

    int main() {
        part2();

        return 0;
    }

}