# Strategie practică de detectare a dialectelor ISO G-code (pe copia locală)

## 1) Obiectiv
Determinarea automată a dialectului pentru un fișier CNC, cu încredere (score), pe baza:
- extensiei,
- semnăturilor lexicale/sintactice,
- codurilor modale și convențiilor de arc/plan,
- structurii de program.

Rezultatul recomandat:
- `dialect_detected` (ex: `fanuc`, `mach3`, `heidenhain`, `isel`, `woodwop`, `biesse_cix`, `busselatto_genesis`, `busselatto_evolution`),
- `family` (ex: `iso_gcode_core`, `vendor_macro`, `conversational`, `structured_parametric`),
- `confidence` (0..1),
- `evidence[]` (tokeni și reguli care au contribuit la scor).

## 2) Observații din corpusul local (feature-uri de antrenare manuală)
În repo există exemple clare pentru mai multe dialecte:
- **Fanuc**: `%`, `O1234`, blocuri `Nxx`, `M06`, `G43 H..`, `M30`. 
- **Mach3**: foarte apropiat de Fanuc, dar include explicit `G91.1` și arce cu `I/J` + adesea `K0` în planul G19.
- **Heidenhain**: conversațional (`BEGIN PGM`, `TOOL CALL`, `L`, `CC`, `CP`, `CYCL DEF`).
- **Isel**: comenzi text (`FASTABS`, `MOVEABS`, `CCWABS`, `PLANE`, `SPINDLE`).
- **Homag/WoodWOP**: structură secțională (`[H`, `[001`, `<...`, `$E...`).
- **Biesse CIX**: blocuri `BEGIN ... END`, `BEGIN MACRO`, `PARAM,NAME=...,VALUE=...`.
- **Busselatto Genesis**: G-code compact cu tokeni concatenați (`G1G90G54...`) și coduri specifice (`G150`, `G63`).
- **Busselatto Evolution**: structură `SIDE#`, `W#...{ ... }W`, puternic parametrică.

## 3) Arhitectură recomandată (2 faze)

### Faza A — Pre-clasificare rapidă (ieftină)
1. **Heuristică după extensie** (`.nc`, `.tap`, `.h`, `.ncp`, `.mpr`, `.cix`, `.cnc`).
2. **Scanare primelor 200-500 linii** pentru tokeni „ancoră” (regexuri unice):
   - Heidenhain: `^\s*\d+\s+BEGIN\s+PGM\b`, `\bTOOL\s+CALL\b`, `\bCYCL\s+DEF\b`
   - Isel: `\bFASTABS\b|\bMOVEABS\b|\bCCWABS\b|\bCWABS\b|\bPROGEND\b`
   - WoodWOP: `^\[H\s*$|^\[\d{3}\s*$|^\$E\d+\s*$`
   - CIX: `^BEGIN\s+MAINDATA\b|^BEGIN\s+MACRO\b|^END\s+MACRO\b`
   - Busselatto Evolution: `^SIDE#\d+\{|^W#\d+\{`
   - ISO/Fanuc/Mach3/Genesis: prezență dominantă `\bG\d+\b`, `\bM\d+\b`, axe `X/Y/Z`.
3. Dacă o regulă „hard signature” lovește, returnezi dialectul direct cu confidence mare.

### Faza B — Scoring fin pentru dialectele apropiate
Folosește un scor ponderat pe feature-uri pentru cazurile ambigue (`.nc`, `.tap`, unele `.cnc`):

`score(dialect) = Σ w_i * feature_i`

Exemple de feature-uri utile:
- **Fanuc**: `%` la început/sfârșit, `O\d+`, `N\d+`, `T\d+ M06`, `G43 ... H\d+`, `M30`.
- **Mach3**: `G91.1`, arce frecvent cu perechi complete `I/J/K`, stil hobby-controller.
- **Genesis**: tokeni concatenați (`G1G90G54`), `G150`, `G63`, densitate mare de coduri pe aceeași linie.

Regulă practică:
- dacă `max_score < prag_minim`: `unknown_iso_variant`;
- dacă `max_score - second_score < delta`: `ambiguous` + top-2 dialecte.

## 4) Normalizare înainte de detectare
Aplică un tokenizer tolerant:
1. Uppercase.
2. Eliminare comentarii multi-stil (`( ... )`, `;...`).
3. Separare tokeni concatenați (`G1G90G54X...` -> `G1 G90 G54 X...`) cu regex incremental.
4. Parsing numeric tolerant (`-53.`, `+61`, `36.0000`).
5. Captură plan modal (`G17/G18/G19`) și convenții arce (`I/J/K`, `R`, `CC/CP`).

## 5) Model de reguli (exemplu minimal)
```yaml
dialects:
  heidenhain:
    hard_signatures:
      - "BEGIN PGM"
      - "TOOL CALL"
      - "CYCL DEF"
  isel:
    hard_signatures:
      - "FASTABS"
      - "MOVEABS"
      - "PROGEND"
  woodwop:
    hard_signatures:
      - "[H"
      - "$E"
  biesse_cix:
    hard_signatures:
      - "BEGIN MAINDATA"
      - "BEGIN MACRO"
  fanuc:
    weighted:
      O_program: 2.0
      block_numbers_N: 1.0
      tool_change_M06: 1.5
      length_comp_H: 1.5
  mach3:
    weighted:
      G91_1: 2.5
      ijk_full_arcs: 1.0
  busselatto_genesis:
    weighted:
      gcode_concatenation: 2.0
      G150: 2.0
      G63: 1.0
```

## 6) Validare pe corpusul local
Recomandare:
1. Creezi un fișier `labels.json` cu eticheta adevărată pentru fiecare exemplu.
2. Rulezi detectorul și colectezi:
   - accuracy top-1,
   - confusion matrix (important pentru Fanuc vs Mach3 vs Genesis),
   - cazuri `ambiguous`.
3. Ajustezi ponderile până obții stabilitate.

Țintă realistă pe acest corpus mixt: 
- >95% pe dialecte structurale (Heidenhain/Isel/WoodWOP/CIX/Evolution),
- 85-95% pe diferențierea fină Fanuc/Mach3/Genesis (depinde de variabilitatea reală a fișierelor noi).

## 7) Recomandări de implementare
- Implementare inițială rule-based (rapidă, explicabilă).
- Păstrează `evidence[]` în output pentru debugging și UI („de ce s-a detectat dialectul X”).
- Versionează regulile (`dialect_rules.v1.yaml`) și adaugă teste de regresie la fiecare exemplu nou.
- Introdu fallback parser:
  - dacă dialect necunoscut -> parsezi subset ISO comun (`G0/G1/G2/G3`, `M3/M5/M30`, `X/Y/Z/F/S`) și marchezi restul ca vendor-specific.

## 8) Mapping rapid pentru fișierele existente
- `Fanuc.nc` -> `fanuc`
- `Mach3.tap` -> `mach3`
- `Busselatto_Genesis.cnc` -> `busselatto_genesis`
- `Heidenhain.h` -> `heidenhain`
- `Isel.ncp` -> `isel`
- `Homag_woodwop.mpr` -> `woodwop`
- `Biesse.cix` -> `biesse_cix`
- `Busselatto_Evolution.cnc` -> `busselatto_evolution`

Această strategie oferă un început robust pentru detecție și poate evolua gradual către un clasificator ML doar dacă apar multe dialecte noi și ambiguități frecvente.
