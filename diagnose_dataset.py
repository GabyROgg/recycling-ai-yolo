from pathlib import Path
import yaml
from PIL import Image

DATA_YAML = "data.yaml"

with open(DATA_YAML, "r", encoding="utf-8", errors="ignore") as f:
    data = yaml.safe_load(f)

nc = int(data["nc"])
print("Număr clase din data.yaml:", nc)

splits = {
    "train": ("train/images", "train/labels"),
    "valid": ("valid/images", "valid/labels"),
}

bad_labels = []
missing_labels = []
bad_images = []
class_ids = set()

for split, (img_dir, lbl_dir) in splits.items():
    img_dir = Path(img_dir)
    lbl_dir = Path(lbl_dir)

    images = list(img_dir.glob("*.*"))[:100]
    print(f"\n{split}: {len(images)} imagini")

    for img_path in images:
        try:
            with Image.open(img_path) as im:
                im.verify()
        except Exception as e:
            bad_images.append((str(img_path), str(e)))

        label_path = lbl_dir / f"{img_path.stem}.txt"

        if not label_path.exists():
            missing_labels.append(str(img_path))
            continue

        lines = label_path.read_text(encoding="utf-8", errors="ignore").splitlines()

        for line_no, line in enumerate(lines, start=1):
            parts = line.strip().split()

            if len(parts) != 5:
                bad_labels.append((str(label_path), line_no, "format gresit", line))
                continue

            try:
                cls = int(float(parts[0]))
                coords = [float(x) for x in parts[1:]]
            except:
                bad_labels.append((str(label_path), line_no, "valori ne-numerice", line))
                continue

            class_ids.add(cls)

            if cls < 0 or cls >= nc:
                bad_labels.append((str(label_path), line_no, f"class id {cls} invalid pentru nc={nc}", line))

            if any(x < 0 or x > 1 for x in coords):
                bad_labels.append((str(label_path), line_no, "coordonate in afara 0-1", line))

print("\n===== REZULTAT =====")
print("Clase gasite in label-uri:", sorted(class_ids))
print("Imagini corupte:", len(bad_images))
print("Imagini fara label:", len(missing_labels))
print("Label-uri problematice:", len(bad_labels))

print("\nPrimele imagini corupte:")
for x in bad_images[:20]:
    print(x)

print("\nPrimele imagini fara label:")
for x in missing_labels[:20]:
    print(x)

print("\nPrimele label-uri problematice:")
for x in bad_labels[:50]:
    print(x)