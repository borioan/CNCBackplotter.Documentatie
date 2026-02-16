

> Cum trebuie să gândesc aplicația astfel încât să suporte 8 formate CNC complet diferite, fără să o distrug pe termen lung?

Perfect.

---

# 🧠 PASUL 1 — Schimbarea mentalității

Nu construiești un „parser CNC”.

Construiești un **motor de procesare CNC extensibil**.

Diferența e uriașă.

---

# 🧠 PASUL 2 — Realitatea tehnică

Ai 3 tipuri de limbaje în fișierele tale:

### 1️⃣ Procedural G-code

(Fanuc, Mach3, Genesis)

Descriu mișcări pas cu pas.

### 2️⃣ Procedural alternativ

(Heidenhain)

Tot mișcări, dar altă sintaxă.

### 3️⃣ Declarativ pe operații

(Homag, Biesse, Evolution)

Descriu operații și parametri.

---

# 🧠 PASUL 3 — Ce înseamnă asta pentru arhitectură

Trebuie să separi sistemul în 3 concepte fundamentale:

## A. Motorul de procesare

Acesta este independent de limbaj.

## B. Modulele de limbaj

Fiecare limbaj este un plugin logic separat.

## C. Modelul intern comun

Unde toate limbajele converg.

---

# 🧠 PASUL 4 — Gândește în termeni de „module”

În loc să întrebi:

> Cum parsez 8 tipuri?

Întreabă:

> Cum adaug un limbaj nou fără să modific restul sistemului?

Dacă răspunsul este:
„Modific 5 fișiere existente”

→ Arhitectura e greșită.

Dacă răspunsul este:
„Adaug un modul nou”

→ Arhitectura e corectă.

---

# 🧠 PASUL 5 — Separare pe responsabilități

Aplicația ta trebuie să aibă 4 zone conceptuale:

---

## 1️⃣ Detectare

Rol:\
Identifică tipul fișierului.

Nu știe nimic despre parsare.

---

## 2️⃣ Interpretare sintactică (per limbaj)

Fiecare limbaj are propriul său univers.

ISO nu trebuie să știe că există WoodWop.
WoodWop nu trebuie să știe că există Fanuc.

Sunt lumi complet separate.

---

## 3️⃣ Model semantic

Fiecare limbaj produce un model propriu.

Nu le amesteci.

---

## 4️⃣ Convergență

Abia aici toate limbajele sunt reduse la ceva comun.

---

# 🧠 PASUL 6 — Principiul de aur

Nu unifica la nivel de sintaxă.
Unifică doar la nivel de comportament fizic.

---

# 🧠 PASUL 7 — Greșelile clasice

❌ Un singur enum global de tokeni\
❌ Un singur parser cu 100 de if-uri\
❌ Dialecte amestecate în aceeași clasă\
❌ Logică „dacă este Homag fă asta” peste tot

Asta duce la colaps după 6 luni.

---

# 🧠 PASUL 8 — Ce construiești de fapt

Nu construiești un parser.

Construiești un sistem cu:

- Kernel neutru
- Module de limbaj independente
- Pipeline stabil

---

# 🧠 PASUL 9 — Gândește ca un compilator

Compilatorul C++ nu încearcă să parseze Python.

Are front-end diferit pentru fiecare limbaj.

Dar back-end-ul este comun.

Tu construiești exact asta.

---

# 🧠 PASUL 10 — Cum gândești corect

Îți pui 3 întrebări:

1. Ce este specific limbajului?
2. Ce este comun tuturor mașinilor?
3. Unde trebuie făcută separarea?

Răspuns:

Specific → modul\
Comun → motor intern

---

# 🎯 Concluzia arhitecturală

Aplicația trebuie să fie:

Modulară\
Extensibilă\
Separată pe responsabilități\
Fără amestec de logică între limbaje

---

# 🔥 Imagine simplă

Gândește sistemul ca:

```
[Detector]
     ↓
[Language Module]
     ↓
[Core Engine]
     ↓
[Backplot]
```

Detectorul alege modulul.
Modulul procesează limbajul.
Core engine nu știe ce limbaj a fost.

---

Dacă vrei, următorul pas util ar fi să discutăm:

> Ce este cu adevărat „core engine”-ul tău și ce nu trebuie să știe niciodată despre limbaje.

Acolo este cheia maturității arhitecturale.

# 🔥 Corectarea modelului conceptual

Trebuie să separi clar:

## 1️⃣ Nivelul Cinematic (Low-Level Motion)

Aici trăiesc doar:

- RapidMove
- LinearMove
- ArcMove
- HelixMove
- ToolChange
- SpindleOn/Off
- CoolantOn/Off

Acesta este **nivelul mașinii CNC**.

Este atomic.
Nu se mai descompune.

---

## 2️⃣ Nivelul Operațional (High-Level Machining Intent)

Aici trăiesc:

- DrillOp
- PocketOp
- ContourOp
- SlotOp
- EngraveOp
- etc.

Acestea NU sunt mișcări.

Ele sunt:

> intenții de prelucrare

Și trebuie compilate în mișcări.

---

# 🧠 Diferența esențială

Procedural G-code (Fanuc, Isel) operează direct la:

→ Nivel Cinematic

Declarative (Biesse, WoodWop) operează la:

→ Nivel Operațional

Numeric Stream (Evolution) este:

→ Nivel Cinematic expandat

---

# 🏗️ Arhitectura corectă (pe 3 nivele reale)

Nu două.
Nu unul.

Trei.

---

# 🟢 LEVEL A – Representation Native (pe dialect)

Fiecare familie produce propriul model:

- Procedural → Instruction AST
- Declarative → Operation Graph
- Numeric → Motion Stream

Nu le amesteci.

---

# 🔵 LEVEL B – Semantic Layer (intermediar inteligent)

Aici faci separarea pe două direcții:

## B1 – Machining Intent Layer (opțional)

Doar pentru limbaje declarative.

Transformi:

```
PocketOp
DrillOp
ContourOp
```

în

→ secvențe de mișcări

Aceasta este o fază de „expansion”.

---

## B2 – Canonical Motion Layer (obligatoriu)

Toate formatele ajung aici.

Conține doar:

- RapidMove
- LinearMove
- ArcMove
- ToolChange
- etc.

Nimic compus.
Nimic semantic.

Doar mișcare pură.

---

# 🔥 Acesta este adevăratul layer comun.

Backplotul trebuie să consume DOAR:

```
CanonicalMotionStream
```

Nu operații.
Nu macro.
Nu intenții.

---

# 🧠 De ce este asta corect?

Pentru că:

Backplot = simulare cinematică.

Nu te interesează dacă mișcarea vine din:

- G1 X...
- W#2201
- LINE\_EP
- POCKET macro

La final, este doar o traiectorie 3D în timp.

---

# 🎯 Structura arhitecturală corectă

```
             DIALECT FRONT-END
                    │
                    ▼
         Native Representation
                    │
                    ▼
      (Optional) Operation Expander
     (doar pt declarative formats)
                    │
                    ▼
         Canonical Motion Model
                    │
                    ▼
             Backplot Engine
```

---

# ⚠️ Observație foarte importantă

Nu trebuie să păstrezi DrillOp în layer-ul comun.

Ele trebuie:

- fie expandate
- fie ignorate
- fie păstrate separat pentru analiză CAM

Dar nu amestecate cu mișcările.

---

# 🧩 Cum tratezi diferența dintre paradigme

## Procedural

Direct → Canonical Motion

## Numeric Stream

Direct → Canonical Motion

## Declarative

Operation Graph\
→ Operation Expander\
→ Canonical Motion

---

# 💎 Concluzie

Mișcarea atomică este fundamentul universal.

Operația este o abstracție deasupra.

Nu trebuie amestecate.

---
