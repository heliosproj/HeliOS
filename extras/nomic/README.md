# Nomic C Semantic Source Code Analyzer

A powerful semantic analysis tool for C source code that enables custom rule-based code quality checks.

## License

This software is licensed under the GNU General Public License v3.0 or later.
See <https://www.gnu.org/licenses/> for details.

## Prerequisites

Before building Nomic, ensure you have the following installed:

- **CMake** 3.20 or higher
- **C++ Compiler** with C++20 support (GCC 11+, Clang 14+, or MSVC 2019+)
- **LLVM/Clang** 14.0 or higher with development headers
- **Git** (for downloading dependencies)
- **Internet connection** (required for first build to download dependencies)

### Installing LLVM/Clang on Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install llvm-14-dev libclang-14-dev clang-14
```

### Installing LLVM/Clang on macOS

```bash
brew install llvm@14
export LLVM_DIR=/usr/local/opt/llvm@14
```

### Installing LLVM/Clang on Fedora/RHEL

```bash
sudo dnf install llvm-devel clang-devel
```

## Building Nomic

### Quick Build

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Build Steps Explained

1. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

2. **Configure with CMake:**
   ```bash
   cmake ..
   ```

   This step will automatically download and configure:
   - spdlog (logging library)
   - yaml-cpp (YAML parsing)
   - nlohmann/json (JSON output)
   - fmt (string formatting)

3. **Build:**
   ```bash
   make -j$(nproc)
   ```

   On macOS or systems without `nproc`:
   ```bash
   make -j4
   ```

4. **Verify the build:**
   ```bash
   ./nomic --version
   ```

## Installation (Optional)

After building, you can install Nomic system-wide:

```bash
sudo make install
```

This installs:
- `nomic` binary to `/usr/local/bin/`
- Header files to `/usr/local/include/nomic/`

## Usage

### Basic Analysis

```bash
nomic analyze myfile.c --rules rules.yaml
```

### Command Line Options

```bash
nomic --help
```

## Creating Rules

Nomic uses YAML-based rule files. Example:

```yaml
metadata:
  version: "1.0"
  description: "My custom rules"

rules:
  - id: COMPLEXITY-001
    description: "Functions should have low complexity"
    severity: warning
    scope: Function
    assert: "fn.cyclomatic_complexity <= 10"
    message: "Function '{{fn.name}}' is too complex"
```

## Troubleshooting

### CMake can't find LLVM

If CMake cannot find LLVM, specify the path explicitly:

```bash
cmake -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm ..
```

### Build fails with C++20 errors

Ensure your compiler supports C++20:

```bash
g++ --version  # Should be 11.0 or higher
clang++ --version  # Should be 14.0 or higher
```

### Missing dependencies during build

The first build downloads dependencies automatically. Ensure you have:
- Active internet connection
- Git installed
- Sufficient disk space (~500 MB)

### LLVM/Clang version mismatch

Nomic requires LLVM/Clang 14.0 or higher. Check your version:

```bash
llvm-config --version
```

If multiple versions are installed, specify the correct one:

```bash
cmake -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm \
      -DClang_DIR=/usr/lib/llvm-14/lib/cmake/clang ..
```

## Clean Build

To start fresh:

```bash
rm -rf build
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Project Structure

```
nomic/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── include/                # Public headers
│   └── nomic/
│       ├── core/           # Core analysis engine
│       ├── dsl/            # Domain-Specific Language
│       ├── cli/            # Command-line interface
│       ├── output/         # Output formatters
│       └── patterns/       # Pattern matching
└── src/                    # Source implementation files
```

## External Dependencies (Auto-Downloaded)

Nomic automatically downloads these dependencies during build:

- **spdlog** v1.12.0 - Fast C++ logging library
- **yaml-cpp** v0.7.0 - YAML parser and emitter
- **nlohmann/json** v3.11.2 - JSON for Modern C++
- **fmt** v10.1.1 - Modern formatting library

These are fetched via CMake's FetchContent during the configuration step.

## Building in Release Mode

For optimized production builds:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Development Build with Debug Symbols

For development and debugging:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

---

**Nomic C Semantic Source Code Analyzer**
Copyright (C) 2025 Manny Peterson
Licensed under GNU General Public License v3.0 or later
