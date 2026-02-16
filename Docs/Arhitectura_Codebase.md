# Arhitectura țintă pentru CNCBackplotter

Acest document transformă observațiile conceptuale din `Docs/Framework.md` într-o arhitectură de implementare, astfel încât aplicația să poată suporta formate CNC heterogene fără degradare pe termen lung.

## 1. Principii de proiectare

1. **Front-end per dialect, kernel comun**: fiecare familie de formate are parser și semantică separate.
2. **Convergență doar la nivel cinematic**: singurul model universal este fluxul de mișcare atomică.
3. **Fără cuplare transversală**: modulele de limbaj nu au dependențe între ele.
4. **Extensibilitate prin adăugare, nu modificare**: un dialect nou se adaugă ca modul nou, fără editări invazive în core.
5. **Separare între intenție și execuție**: operațiile CAM (drill/pocket/contour) nu ajung direct în motorul de backplot.

## 2. Modelul pe niveluri

### Level A — Native Representation (per dialect)

Fiecare dialect produce un model propriu:
- `InstructionAst` pentru procedural (Fanuc, Mach3, Heidenhain, Isel, Genesis);
- `OperationGraph` pentru declarative (WoodWOP/Homag, Biesse);
- `NumericMotionStream` pentru numeric stream (Evolution).

> Regula: nu există enum/token global comun la acest nivel.

### Level B1 — Operation Expansion (opțional)

Activ doar pentru formatele declarative:
- transformă `OperationGraph` în mișcări atomice;
- rezolvă parametri, sisteme locale de coordonate, intrări/ieșiri din material;
- produce o secvență de mișcare fără concepte macro la ieșire.

### Level B2 — Canonical Motion Layer (obligatoriu)

Toate dialectele converg în `CanonicalMotionStream` cu primitive atomice:
- `RapidMove`
- `LinearMove`
- `ArcMove`
- `HelixMove`
- `ToolChange`
- `SpindleOn` / `SpindleOff`
- `CoolantOn` / `CoolantOff`
- `ProgramStop` / `ProgramEnd`

> Backplot-ul consumă exclusiv acest nivel.

### Level C — Backplot Engine

Motor neutru față de limbaj:
- simulare cinematică 3D;
- calcul bounding box și timpi estimați;
- validări de siguranță (de ex. plunge ilegal, arc degenerat, feed invalid);
- export vizualizare/traseu.

## 3. Pipeline complet

```text
Input File
   │
   ▼
Dialect Detector
   │
   ▼
Dialect Front-end (parse + semantic local)
   │
   ├── procedural/numeric  ────────────────┐
   │                                        ▼
   └── declarative ──► Operation Expander ► Canonical Motion Stream
                                              │
                                              ▼
                                        Backplot Engine
```

## 4. Structură recomandată de cod

```text
src/
  app/
    pipeline/
      run_pipeline.*
    services/
      file_loader.*
      diagnostics.*

  core/
    motion/
      canonical_motion.*
      motion_validator.*
    engine/
      backplot_engine.*
      interpolation.*
    units/
      units_normalizer.*
    diagnostics/
      diagnostic_model.*

  detection/
    dialect_detector.*
    signatures/
      fanuc_signature.*
      heidenhain_signature.*
      woodwop_signature.*
      biesse_signature.*
      evolution_signature.*

  dialects/
    fanuc/
      parser.*
      semantic_mapper.*
    mach3/
      parser.*
      semantic_mapper.*
    heidenhain/
      parser.*
      semantic_mapper.*
    isel/
      parser.*
      semantic_mapper.*
    genesis/
      parser.*
      semantic_mapper.*
    woodwop/
      parser.*
      operation_graph.*
      expander.*
    biesse/
      parser.*
      operation_graph.*
      expander.*
    evolution/
      parser.*
      numeric_mapper.*

  contracts/
    dialect_plugin.*
    parse_result.*
    semantic_result.*
```

## 5. Contracte obligatorii

### `DialectPlugin`

Fiecare modul implementează:
- `canHandle(fileMeta, sample): ConfidenceScore`
- `parse(source): NativeRepresentation`
- `toCanonical(native): CanonicalMotionStream | OperationGraph`

### `OperationExpander`

Doar pentru declarative:
- `expand(operationGraph): CanonicalMotionStream`

### `BackplotEngine`

- `simulate(canonical): SimulationResult`
- fără API dependent de dialect.

## 6. Reguli anti-colaps

1. **Interzis** `if dialect == ...` în `core/engine`.
2. **Interzis** import din `dialects/*` în `core/*`.
3. **Interzis** păstrarea operațiilor CAM în `CanonicalMotionStream`.
4. **Obligatoriu** test de contract per plugin nou.
5. **Obligatoriu** fixture de regresie pe fișier real din `CNC_code_examples/`.

## 7. Strategie de extindere (dialect nou)

Când se adaugă un format nou:
1. se creează director `src/dialects/<nou>/`;
2. se implementează `DialectPlugin`;
3. se adaugă semnătură în detector;
4. se adaugă teste de contract + teste pe fixture real;
5. nu se editează codul din backplot engine.

Dacă sunt necesare modificări în engine pentru un dialect nou, arhitectura este încălcată.

## 8. Plan de migrare în 4 etape

### Etapa 1 — Stabilizare contracte
- definire `CanonicalMotionStream` și `DialectPlugin`;
- introducere detector cu scor de încredere.

### Etapa 2 — Migrare procedural/numeric
- Fanuc + Mach3 + Heidenhain + Isel + Genesis + Evolution către `toCanonical` direct.

### Etapa 3 — Migrare declarative
- WoodWOP și Biesse: `OperationGraph` + `OperationExpander`.

### Etapa 4 — Hardening
- suite regresie pe toate exemplele din repo;
- validări cinematice stricte;
- benchmark pe fișiere mari (`.cix`, `.mpr`).

## 9. Criterii de acceptare arhitecturală

Arhitectura este considerată sănătoasă dacă:
- un format nou se adaugă în principal prin fișiere noi;
- engine-ul rulează identic indiferent de dialect;
- nu există dependențe circulare între `core`, `detection`, `dialects`;
- ieșirea standard a tuturor parserelor este `CanonicalMotionStream`.

## 10. Decizie finală

Modelul comun al aplicației este **mișcarea atomică**.

Tot ce este sintaxă, macro, operație CAM sau dialect rămâne în front-end-uri dedicate. Kernel-ul de backplot rămâne neutru, stabil și extensibil.
