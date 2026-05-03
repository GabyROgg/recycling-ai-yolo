from pathlib import Path
import shutil
import yaml

SOURCE_DIR = Path(".")
OUTPUT_DIR = Path("dataset_5classes")

NEW_CLASSES = [
    "plastic",
    "metal",
    "glass",
    "paper_cardboard",
    "other"
]

# clase vechi Roboflow -> clase noi
CLASS_MAP = {
    0: "metal",            # Aluminum can
    1: "paper_cardboard",  # Cardboard
    2: "other",            # Container for household chemicals
    3: "glass",            # Glass bottle
    4: "other",            # Organic
    5: "paper_cardboard",  # Paper
    6: "plastic",          # Plastic bag
    7: "plastic",          # Plastic bottle
    8: "plastic",          # Plastic cup
    9: "metal",            # Tin
}

new_id = {name: i for i, name in enumerate(NEW_CLASSES)}

splits = ["train", "valid", "test"]

if OUTPUT_DIR.exists():
    shutil.rmtree(OUTPUT_DIR)

for split in splits:
    img_src = SOURCE_DIR / split / "images"
    lbl_src = SOURCE_DIR / split / "labels"

    if not img_src.exists() or not lbl_src.exists():
        print(f"Sar peste {split}, nu există complet.")
        continue

    img_out = OUTPUT_DIR / split / "images"
    lbl_out = OUTPUT_DIR / split / "labels"
    img_out.mkdir(parents=True, exist_ok=True)
    lbl_out.mkdir(parents=True, exist_ok=True)

    for img_path in img_src.glob("*.*"):
        label_path = lbl_src / f"{img_path.stem}.txt"
        if not label_path.exists():
            continue

        new_lines = []

        for line in label_path.read_text(errors="ignore").splitlines():
            parts = line.strip().split()

            if len(parts) != 5:
                continue

            old_cls = int(float(parts[0]))

            if old_cls not in CLASS_MAP:
                continue

            new_class_name = CLASS_MAP[old_cls]
            parts[0] = str(new_id[new_class_name])
            new_lines.append(" ".join(parts))

        if new_lines:
            shutil.copy2(img_path, img_out / img_path.name)
            (lbl_out / label_path.name).write_text("\n".join(new_lines))

data = {
    "train": "dataset_5classes/train/images",
    "val": "dataset_5classes/valid/images",
    "test": "dataset_5classes/test/images",
    "nc": len(NEW_CLASSES),
    "names": NEW_CLASSES
}

with open(OUTPUT_DIR / "data.yaml", "w", encoding="utf-8") as f:
    yaml.dump(data, f, sort_keys=False)

print("Gata. Dataset simplificat creat în:")
print("dataset_5classes/")
print("Folosește:")
print("dataset_5classes/data.yaml")