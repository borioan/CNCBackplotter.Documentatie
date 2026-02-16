# Analiza copiei locale `CNCBackplotter.Documentatie`

## Inventar

- Total fișiere versionate identificate: **9**.
- Structură:
  - `README.md`
  - `CNC_code_examples/` cu 8 fișiere de programe CNC în formate diferite.

## Observații generale

1. Repository-ul este în principal un set de **exemple de programe CNC** pentru controllere/producători diferiți (Fanuc, Heidenhain, Mach3, Isel, Homag/WoodWOP, Biesse, Busselatto).
2. Majoritatea fișierelor reprezintă **același traseu de prelucrare** (ID program 1234), transpus în sintaxe diferite.
3. Dimensiunile piesei apar recurent ca aproximativ **122 x 96 x 36** (L x W x T / X x Y x Z), indicând consistență între formate.
4. Nu există cod sursă de aplicație în acest repo; este orientat pe documentație + mostre input CNC.

## Fișiere analizate

### `README.md`
- Descriere scurtă a scopului repo-ului: documentație pentru aplicație de backplotting.
- Menționează explicit folderul cu formate CNC diferite.

### `CNC_code_examples/Fanuc.nc`
- Program FANUC clasic (`%`, `O1234`, blocuri `Nxx`).
- Include secvențe standard: selecție sculă (`T1 M06`), spindle (`S5000 M03`), coolant (`M08`), reveniri și `M30`.
- Mișcări mixte liniar/circular (`G01`, `G02`, `G03`) în plane XY și YZ.

### `CNC_code_examples/Mach3.tap`
- Variantă apropiată de G-code generic pentru Mach3.
- Include inițializare modală (`G90 G94 ...`), ciclu de prelucrare similar cu Fanuc.
- Se observă diferențe de sintaxă la arce (I/J/K explicite).

### `CNC_code_examples/Heidenhain.h`
- Program conversațional Heidenhain (`BEGIN PGM`, `TOOL CALL`, `L`, `CC`, `CP`).
- Include toleranță (`CYCL DEF 32`) și finalizare `END PGM`.

### `CNC_code_examples/Isel.ncp`
- Sintaxă Isel orientată pe comenzi text (`FASTABS`, `MOVEABS`, `CCWABS`, `CWABS`, `SPINDLE`).
- Coordonatele par scalate în miimi (ex. `X-53000`).

### `CNC_code_examples/Homag_woodwop.mpr`
- Format WoodWOP/HOMAG structurat pe secțiuni cu chei (`[H`, `[001`, etc.).
- Include metadata de piesă/material și macro-uri/operații.
- Fișier relativ mare (545 linii), potrivit pentru testarea parserelor bogate în atribute.

### `CNC_code_examples/Biesse.cix`
- Format CIX cu blocuri `BEGIN ... END` și `BEGIN MACRO` repetate.
- Conține `MAINDATA` (dimensiuni/material) și numeroase macro-uri parametrizate.
- Cel mai mare fișier din repo (1116 linii), util pentru testarea performanței parserului.

### `CNC_code_examples/Busselatto_Genesis.cnc`
- G-code compact specific Genesis, cu linii dense (`G1G90G54...`).
- Conține secvență de contur/pasaje pe coordonate apropiate de celelalte formate.

### `CNC_code_examples/Busselatto_Evolution.cnc`
- Format Busselatto Evolution cu blocuri `SIDE#` și instrucțiuni `W#...` parametrizate.
- Include definiri multiple ale laturilor și traseului, stil non-G-code clasic.

## Recomandări pentru aplicația de backplotting

1. Definiți un model intermediar comun (toolpath primitives: line/arc/rapid/tool/spindle/coolant).
2. Implementați parser modular per format (un adaptor per extensie).
3. Normalizați unitățile (mm vs inch/conversii implicite).
4. Construiți un set de teste de regresie folosind aceste fișiere ca corpus minim.
5. Prioritizați fișierele mari (`.cix`, `.mpr`) pentru testare de robustețe și performanță.

## Re-gândire arhitecturală (aliniată cu `Docs/Framework.md`)

Arhitectura recomandată pentru implementare este acum formalizată în `Docs/Arhitectura_Codebase.md` și se bazează pe:

1. Front-end separat per dialect CNC.
2. Convergență obligatorie în `CanonicalMotionStream` (nivel cinematic atomic).
3. Expander dedicat doar pentru formatele declarative (`OperationGraph -> motion`).
4. Motor de backplot complet agnostic de dialect.

Această structură elimină riscul de parser monolitic și permite adăugarea dialectelor noi prin module noi, fără modificări invazive în engine.
