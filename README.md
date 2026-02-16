# Documentație – Aplicație Backplotting CNC

Acest repository conține documentația și setul minim de fișiere CNC necesare pentru proiectarea arhitecturii aplicației de backplotting.

## Conținut

- `Docs/Framework.md` – observații conceptuale despre separarea pe module, niveluri și convergența pe mișcare.
- `Docs/Arhitectura_Codebase.md` – propunerea de arhitectură tehnică completă (pipeline, contracte, structură de cod, migrare).
- `CNC_code_examples/` – corpus de fișiere reale în dialecte CNC diferite pentru testare și regresie.

## Scop

Scopul principal este definirea unei arhitecturi extensibile care poate integra dialecte CNC diferite fără cuplare excesivă între parser-ele de limbaj și motorul de backplot.
