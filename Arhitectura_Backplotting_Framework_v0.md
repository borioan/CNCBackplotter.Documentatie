# Arhitectura completă -- Aplicație profesională de Backplotting CNC

## 1. Strat Input / Import

Responsabil pentru: - Încărcare fișiere (G-code, Siemens, Heidenhain
etc.) - Detectare dialect - Normalizare unități și encoding - Eliminare
comentarii - Expandare macro-uri simple

------------------------------------------------------------------------

## 2. Lexer + Parser

Transformă textul brut în structură internă.

### Lexer

-   Tokenizare (G, M, X, Y, Z, F, S etc.)

### Parser

-   Analiză sintactică
-   Validare structură blocuri
-   Generare structură intermediară (AST sau Block List)

Rezultat: program CNC reprezentat logic, nu text.

------------------------------------------------------------------------

## 3. CNC Kernel (Interpreter Cinematic)

Componenta centrală.

Responsabil pentru: - Gestionare stări modale (G90/G91 etc.) - Sisteme
de coordonate (G54--G59) - Compensări (G41/G42) - Tool length offset -
Interpolări (G0, G1, G2, G3) - Cicli fixați - Subprograme - Variabile

Output: listă de **Mișcări Canonice** (format intern standardizat)

Exemplu mișcare canonică:

    LINEAR_MOVE(x, y, z, feed)
    ARC_MOVE_CW(start, end, center, feed)

------------------------------------------------------------------------

## 4. Model Cinematic Mașină

Definește mașina virtuală.

Include: - Axe liniare și rotative - Limite curse - Lanț cinematic
(kinematic chain) - Transformări matriceale 4x4

Responsabil pentru: - Calcul poziție reală sculă - Transformare
coordonate piesă → mașină - Gestionare 4/5 axe

------------------------------------------------------------------------

## 5. Generator Traiectorie

Transformă mișcările canonice în segmente grafice.

Include: - Discretizare linii și arce - Interpolare numerică - Control
precizie geometrică - Segmentare adaptivă

Output: puncte 3D pentru randare

------------------------------------------------------------------------

## 6. Motor Grafic 3D

Separat complet de logica CNC.

Tehnologii posibile: - OpenGL - DirectX - Vulkan

Responsabil pentru: - Randare traiectorie - Model 3D piesă - Model 3D
sculă - Highlight linie cod activă

------------------------------------------------------------------------

## 7. Modul Simulare Material (Opțional Avansat)

### Variante:

1.  Voxel-based
2.  Boolean / B-Rep

Permite: - Material removal - Detectare coliziuni - Comparare cu model
CAD

------------------------------------------------------------------------

## 8. Engine Detectare Coliziuni

Detectează: - Sculă vs piesă - Sculă vs prinderi - Axe vs structură
mașină - Depășire limită axe

Tehnici: - Bounding Boxes - BVH (Bounding Volume Hierarchy) - Teste
geometrice optimizate

------------------------------------------------------------------------

## 9. Controller UI

Funcționalități: - Timeline execuție - Step-by-step - Play / Pause /
Rewind - Pan / Zoom / Rotate - Sincronizare cod ↔ mișcare

------------------------------------------------------------------------

# Flux Arhitectural General

Input\
→ Parser\
→ CNC Kernel\
→ Mișcări Canonice\
→ Model Cinematic\
→ Generator Traiectorie\
→ Motor Grafic

------------------------------------------------------------------------

# Separări Critice în Arhitectură

🔵 Logică CNC (interpretare cod)\
🟢 Cinematică (transformări spațiale)\
🔴 Randare (vizualizare)

------------------------------------------------------------------------

# Principii de Design

-   Separare completă dialect ↔ kernel
-   Sistem bazat pe stări modale
-   Arhitectură modulară
-   Posibilitate multi-thread (interpretare vs randare)
-   Recalcul incremental
-   Motor grafic independent de UI
