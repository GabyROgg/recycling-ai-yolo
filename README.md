# ♻️ Recycling AI - YOLOv8 Waste Detection

## 📌 Descriere
Acest proiect implementează un sistem de inteligență artificială bazat pe YOLOv8 pentru detectarea și clasificarea automată a deșeurilor reciclabile.

Modelul poate identifica în timp real obiecte din imagini sau video și le clasifică în categorii relevante pentru reciclare:
- plastic
- metal
- sticlă
- hârtie/carton
- alte deșeuri

Scopul proiectului este automatizarea procesului de sortare și reducerea intervenției umane în gestionarea deșeurilor.

---

## 🚀 Tehnologii utilizate
- Python
- YOLOv8 (Ultralytics)
- PyTorch (CUDA)
- OpenCV

---

## ⚙️ Instalare

```bash
python -m venv .venv
.\.venv\Scripts\activate
python -m pip install --upgrade pip
python -m pip install ultralytics torch torchvision opencv-python pyyaml
