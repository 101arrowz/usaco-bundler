#include <iostream>
#include <fstream>
#include <string>
#include <set>
#ifdef _WIN32
#include <direct.h>
#define cd _chdir
#define getcwd _getcwd
#define sep '\\'
#define pathbufsize 261
#define fullpath(fp, buf) _fullpath(buf, fp, pathbufsize)
#else
#include <unistd.h>
#include <stdlib.h>
#define cd chdir
#define getcwd getcwd
#define sep '/'
#define pathbufsize 4097
#define fullpath(fp, buf) realpath(fp, buf)
#endif

std::set<std::string> processed;
std::set<std::string> toInclude;

void cdfp(std::string fileName) {
    int lastInd = fileName.find_last_of(sep);
    if (lastInd == -1) {
        return;
    }
    std::string dir = fileName.substr(0, lastInd);
    if (cd(dir.c_str())) {
        throw std::runtime_error("Could not chdir to " + dir);
    }    
}

std::string process(const char* fileName) {
    char filePath[pathbufsize];
    !fullpath(fileName, filePath);
    if (!processed.insert(filePath).second) {
        // This file has already been processed
        return "";
    }
    std::ifstream fin(filePath);
    if (!fin.good()) {
        std::string fileNameStr (fileName);
        std::string ext = fileNameStr.substr(fileNameStr.find_last_of('.') + 1);
        // Ignore .c and .h - assume those are external
        if (ext == "cpp" || ext == "hpp") {
            std::cerr << "WARN: Included file " << filePath << " not found. Skipping.\n";
        }
        // If there was no extension or the extension wasn't one of those, no need to warn.
        // IDE would have caught it, or it's STL.
        toInclude.insert(fileName);
        return "";
    }
    cdfp(filePath);
    char* cwd = getcwd(nullptr, pathbufsize);
    std::string line;
    std::string out;
    while (std::getline(fin, line)) {
        if (line.substr(0, 8) == "#include") {
            std::string rawFP = line.substr(9);
            const char* check = "<>";
            if (rawFP.find(check[0]) == -1)
                check = "\"\"";
            std::string fp = rawFP.substr(rawFP.find(check[0]) + 1, rawFP.find_last_of(check[1]) - 1);
            std::string result = process(fp.c_str());
            if (cd(cwd)) {
                throw std::runtime_error(std::string("Could not chdir to ") + cwd);
            }
            if (result.size() != 0) {
                out += "// File: " + fp + '\n' + result + '\n';
            }
        } else if (line == "#pragma once") {
            // ignore - files are bundled only once automatically
        } else {
            out += line + '\n';
        }
    }
    return out;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::invalid_argument("missing filename");
    }
    char* initCwd = getcwd(nullptr, pathbufsize);
    std::string processed = process(argv[1]);
    std::string o;
    for (std::string dep : toInclude) {
        o += "#include <" + dep + ">\n";
    }
    o += '\n' + processed + "\n// This code was bundled by usaco-bundler\n";
    if (cd(initCwd)) {
        throw std::runtime_error(std::string("Could not chdir to ") + initCwd);
    }
    if (argc < 3) {
        std::cout << o;
    } else {
        std::ofstream fout(argv[2]);
        fout << o;
    }
}