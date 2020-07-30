#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <functional>
#include <set>
#ifdef _WIN32
#include <direct.h>
#define cd _chdir
#define sep '\\'
#else
#include <unistd.h>
#define cd chdir
#define sep '/'
#endif

void cdfp(std::string fileName) {
    cd(fileName.substr(0, fileName.find_last_of(sep)).c_str());
}

std::string process(const char* fileName, std::set<int> &processed) {
    std::ifstream fin(fileName);
    std::string fileNameStr (fileName);
    cdfp(fileNameStr);
    if (!fin.good()) {
        std::string ext = fileNameStr.substr(fileNameStr.find_last_of('.') + 1);
        // Ignore .c and .h - assume those are external
        if (ext == "cpp" || ext == "hpp") {
            std::cerr << "WARN: Included file " << fileName << " not found. Skipping.\n";
        }
        // If there was no extension or the extension wasn't one of those, no need to warn.
        // IDE would have caught it, or it's STL.
        return processed.insert(std::hash<std::string>{}(fileNameStr)).second
            ? "// ignore"
            : "";
    }
    std::stringstream s;
    s << fin.rdbuf();
    std::string res = s.str();
    std::stringstream input(res);
    if (!processed.insert(std::hash<std::string>{}(res)).second) {
        // This file has already been processed
        return "";
    }
    std::string line;
    std::string out;
    while (std::getline(input, line)) {
        if (line.substr(0, 8) == "#include") {
            std::string rawFP = line.substr(9);
            const char* check = "<>";
            if (rawFP.find(check[0]) == -1)
                check = "\"\"";
            std::string fp = rawFP.substr(rawFP.find(check[0]) + 1, rawFP.find_last_of(check[1]) - 1);
            std::string result = process(fp.c_str(), processed);
            if (result == "// ignore") {
                out += line + '\n';
            } else if (result.size() != 0) {
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
    std::set<int> processed;
    std::string out = process(argv[1], processed);
    cdfp(std::string(argv[1]));
    std::ostream* strm = nullptr;
    if (argc < 3) {
        strm = &std::cout;
    } else {
        std::ofstream fout(argv[2]);
        strm = &fout;
    }
    *strm << out << "\n// This code was bundled by usaco-bundler, a tool created by 101arrowz.\n";
}