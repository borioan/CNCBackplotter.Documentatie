# Documentare Aplicație Back-Plotting CNC

## 1. Situația Existentă

### 1.1 Server
Aplicația de server este construită pe .NET SignalR și rulează pe un PC în rețeaua LAN locală. Aceasta preia fișierele generate de aplicația de CAM de pe discul local și le trimite către clientul SignalR, care este implementat într-o aplicație WPF ce rulează pe un alt calculator din rețeaua locală.

### 1.2 Client
În aplicația client WPF, care implementează și funcționalitatea de back-plotting dorită, există o librărie C++/CLI ce implementează un wrapper managed C# OpenCascade. 
- În prezent, clientul poate recepționa un blob binar din server care conține segmentele geometrice pure/nativ ale operațiilor CAM. 
- Aceste segmente pot fi afișate și animate dinamic, simulând deplasarea sculei în funcție de feedrate-ul primit.
- Geometria este afișată printr-un `HWNDHost` în fereastra UI WPF.
- Interacțiunea cu UI-ul se face prin elemente precum slider sau click în fereastra de vizualizare.

### 1.3 Necesitatea Refolosirii Motorului Geometric C++
Motorul geometric implementat în librăria C++/CLI trebuie să fie extins pentru a suporta și procesarea datelor CNC/G-code, nu doar geometrii pure.

---

## 2. Cerințe Aplicație Back-Plotting

### 2.1 Obiectivul Aplicației
Scopul este dezvoltarea unei aplicații de back-plotting CNC similară cu cele de la Cimco sau HSM Edit, având în vedere profesionalismul și complexitatea acestor aplicații. Aplicația trebuie să fie mai performantă decât Cimco, poate chiar comparabilă cu Vericut, dar acest lucru trebuie realizat în etape. Pregătirea arhitecturii trebuie să permită extinderea funcționalităților pe termen lung.

### 2.2 Provocări
1. **Dimensiunea fișierelor:** Fișierele de text procesate pot varia semnificativ ca mărime (de la câțiva KB până la sute de MB). De obicei, fișierele sunt între 0.5 MB și 10 MB, deci procesarea textului CNC/G-code trebuie să fie eficientă.
2. **Locația procesării:** Trebuie să se analizeze dacă procesarea fișierelor CNC/G-code se va face în librăria C++/CLI sau în aplicația WPF.
3. **Sincronizarea UI și toolpath:** O funcționalitate esențială este sincronizarea corectă între linia de G-code selectată în UI și segmentul geometric corespunzător din toolpath, tratând fiecare linie de G-code ca un interval de execuție.

---

## 3. Cerințe Funcționale Aplicație Back-Plotting CNC

### 3.1 Cerințe de MVP (Minimum Viable Product)

- **Standarde G-code suportate:**
  - ISO G-code Fanuc
  - Siemens
  - Heidenhain
  - Isel (.ncp)
  - Utilaje de tip lemn (ex. Biesse)

- **Axe interpolate:** Suport pentru 3 axe simultane pentru frezare CNC, cu posibilitatea de extindere la 4/5 axe.
  
- **Complexitatea comenzilor CNC:** Aplicația trebuie să gestioneze:
  - Deplasări liniare (inclusiv RAPID)
  - Deplasări circulare (arce/cercuri)

- **Comenzi G-code de implementat:**
  - **G90/G91:** Moduri de coordonare incrementală.
  - **G0/G1/G2/G3:** Comenzi pentru mișcare liniară și circulară.
  - **Plane multiple:** G17/G18/G19 pentru alegerea planurilor de lucru.
  - **Feedrate corect aplicat:** Gestionarea corectă a vitezei de alimentare.
  - **Unități:** Suport pentru unități (G20 pentru inch, G21 pentru mm).
  - **Logica schimbării sculei:** Implementarea unui sistem logic pentru schimbarea sculei.

### 3.2 Sincronizare UI și Geometrie

- Atunci când o linie de G-code este selectată în UI-ul de text CNC, trebuie să existe o corespondentă corectă între linia respectivă și segmentul geometric din toolpath.
- O linie de G-code nu reprezintă un punct, ci o **acțiune desfășurată în timp și spațiu**, care trebuie să fie sincronizată corect.
- UI-ul trebuie să trateze fiecare linie de G-code ca un **interval de execuție**, sincronizându-se cu deplasările din toolpath.

---

## 4. Concluzii și Direcții Viitoare

- **Arhitectura aplicației:** Se va construi o fundație solidă pentru extinderea ulterioară a aplicației.
- **Optimizări:** Trebuie implementate optimizări pentru gestionarea fișierelor mari și pentru sincronizarea corectă între UI și mișcările sculei.
- **Extensibilitate:** Aplicația trebuie să permită extinderea ulterioară a suportului pentru noi standarde de G-code și noi tipuri de utilaje CNC.

---

## 5. Cerințe MVP: Implementarea Stărilor Modale

Aplicația trebuie să includă un **Modal State Manager** care să gestioneze și să monitorizeze toate stările modale relevante pentru G-code, asigurând corectitudinea execuției și sincronizarea între interfața utilizator și mișcările sculei. Stările modale ce trebuie gestionate includ:

- **G90** – Mod absolut
- **G91** – Mod incremental
- **G17/G18/G19** – Planuri de lucru (XY, ZX, YZ)
- **G0/G1** – Mișcări rapide și liniare
- **G20/G21** – Unități (inch/mm)
- **G94/G95** – Feedrate (per minut/per rotație)
- **G92** – Setarea offset-urilor
- **M-coduri** – Comenzi mașină (ex. M3 pentru turație)

Managerul de stări trebuie să asigure:
- **Gestionarea corectă a tranzițiilor** între moduri.
- **Sincronizarea corectă a UI-ului** cu starea activă a aplicației, garantând interpretarea corectă a coordonatelor și mișcărilor.

