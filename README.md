# usaco-bundler

Bundles `#include` directives into a single output file, which is useful for contests like USACO where only a single file can be uploaded. This allows you to create useful utility functions in advance and simply `#include` them during the contest. This works recursively, such that the `#include`s in the files you include will also be bundled. Note that STL libraries will not be included in the output file, and that multiple instances of the same file will not be bundled twice.

## Usage

Use your C++ compiler (clang, GCC, etc.) to compile `main.cpp`.

```
g++ main.cpp -o bundle
./bundle ../path/to/your-entry-file.cpp ./out-file.cpp
```

If you don't specify an output file, the contents will be sent to the standard output. This can be piped to the clipboard, your own script, etc.

On MacOS (bash):
```bash
./bundle ../cow-herding/main.cpp | pbcopy
```
On Windows (CMD/PowerShell):
```
./bundle ../cow-herding/main.cpp | clip.exe
```

## Example

`main.cpp`:
```cpp
#include <iostream>
#include <epic.hpp>
#include "cool.cpp"

int main() {
    std::cout << cool() << '\n';
}
```

`epic.hpp`:
```cpp
#include <string>
std::string EPIC_STRING = "EPIC";
```

`cool.cpp`:
```cpp
#include <string>
#include "epic.hpp"
std::string cool() {
    return EPIC_STRING;
}
```

Output:
```cpp
#include <iostream>
#include <string>

// File: epic.cpp
std::string EPIC_STRING = "EPIC";

// File: cool.cpp
std::string cool() {
    return EPIC_STRING;
}

int main() {
    std::cout << cool() << '\n';
}
```

As you can see, header files and normal `.cpp` files work, both double-quote and angle-bracket `#include`s work, and duplicate `#include`s are ignored. All STL imports are hoisted to the top of the file.


## Wait, isn't using other people's code cheating?
This script basically copy-pastes external `#include`s into a single file with the rest of your code, something you could do yourself but also something that you may not want to waste time doing. It's not cheating to use it. However, **it IS cheating to `#include` other people's code, with or without this tool.**

## License
MIT