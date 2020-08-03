#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <functional>
#include <set>
#ifdef _WIN32
#include <direct.h>
#define cd _chdir
#define getcwd _getcwd
#define sep '\\'
#else
#include <unistd.h>
#define cd chdir
#define getcwd getcwd
#define sep '/'
#endif

typedef std::pair<std::string, std::set<std::string>> ret;

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

ret process(const char* fileName, std::set<int> &processed) {
    std::ifstream fin(fileName);
    std::string fileNameStr (fileName);
    if (!fin.good()) {
        std::string ext = fileNameStr.substr(fileNameStr.find_last_of('.') + 1);
        // Ignore .c and .h - assume those are external
        if (ext == "cpp" || ext == "hpp") {
            std::cerr << "WARN: Included file " << fileName << " not found. Skipping.\n";
        }
        // If there was no extension or the extension wasn't one of those, no need to warn.
        // IDE would have caught it, or it's STL.
        ret r;
        r.second.insert(fileName);
        return r;
    }
    cdfp(fileNameStr);
    char* cwd = getcwd(nullptr, 255);
    std::stringstream s;
    s << fin.rdbuf();
    std::string res = s.str();
    std::stringstream input(res);
    if (!processed.insert(std::hash<std::string>{}(res)).second) {
        // This file has already been processed
        return ret();
    }
    std::string line;
    ret r;
    while (std::getline(input, line)) {
        if (line.substr(0, 8) == "#include") {
            std::string rawFP = line.substr(9);
            const char* check = "<>";
            if (rawFP.find(check[0]) == -1)
                check = "\"\"";
            std::string fp = rawFP.substr(rawFP.find(check[0]) + 1, rawFP.find_last_of(check[1]) - 1);
            ret result = process(fp.c_str(), processed);
            if (cd(cwd)) {
                throw std::runtime_error(std::string("Could not chdir to ") + cwd);
            }
            for (std::string s : result.second) {
                r.second.insert(s);
            }
             if (result.first.size() != 0) {
                r.first += "// File: " + fp + '\n' + result.first + '\n';
            }
        } else if (line == "#pragma once") {
            // ignore - files are bundled only once automatically
        } else {
            r.first += line + '\n';
        }
    }
    return r;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::invalid_argument("missing filename");
    }
    std::set<int> processed;
    char* initCwd = getcwd(nullptr, 255);
    ret out = process(argv[1], processed);
    std::string o;
    for (std::string dep : out.second) {
        o += "#include <" + dep + ">\n";
    }
    o += '\n' + out.first + "\n// This code was bundled by usaco-bundler\n";
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