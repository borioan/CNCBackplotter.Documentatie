# CNC ISO G-code Dialect Detector (pentru integrare în librărie statică C++)

Acest modul este pregătit pentru integrare directă într-o librărie statică C++ din Visual Studio 2022 (Windows 11), fără CMake și fără executabil separat.

## Fișiere de integrat în proiectul `.vcxproj` (Static Library)

- `cpp_dialect_detector/include/DialectDetector.h`
- `cpp_dialect_detector/src/DialectDetector.cpp`

## Integrare rapidă în Visual Studio 2022

1. Creezi/folosești proiectul tău de tip **Static Library (.lib)**.
2. Adaugi fișierul header `DialectDetector.h` la **Header Files**.
3. Adaugi fișierul `DialectDetector.cpp` la **Source Files**.
4. În `Project Properties -> C/C++ -> Additional Include Directories`, adaugi:
   - `$(SolutionDir)cpp_dialect_detector\include`
5. Compilezi în configurația dorită (`Debug`/`Release`, `x64`).

## API disponibil

- `cnc::DialectDetector::detectText(const std::string&)`
- `cnc::DialectDetector::detectFile(const std::string&)`
- `cnc::DialectDetector::setThresholds(double minScore, double ambiguityDelta)`
- `cnc::toString(cnc::Dialect)`
- `cnc::toString(cnc::Family)`

## Observații

- Nu există dependențe externe (doar STL).
- Structura este rule-based + scoring, ușor de extins prin `RuleRepository::defaultRules()`.
- Nu există fișiere de build system în modul (fără `CMakeLists.txt`).
