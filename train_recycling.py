from ultralytics import YOLO
import torch
from multiprocessing import freeze_support
from pathlib import Path


def main():
    DATA_YAML = "data.yaml"
    BASE_MODEL = "yolov8s.pt"

    RUN_DIR = Path("runs/detect/recycling_rtx3060_yolov8s")
    LAST_PT = RUN_DIR / "weights" / "last.pt"

    if not torch.cuda.is_available():
        raise RuntimeError("CUDA nu este disponibil.")

    print("GPU:", torch.cuda.get_device_name(0))

    # Dacă există last.pt, continuă același training
    if LAST_PT.exists():
        print(f"Reiau training-ul din: {LAST_PT}")
        model = YOLO(str(LAST_PT))
        model.train(resume=True)
    else:
        print("Încep training nou.")
        model = YOLO(BASE_MODEL)

        model.train(
            data=DATA_YAML,
            epochs=50,
            imgsz=640,
            batch=16,
            device=0,
            workers=4,

            project="runs/detect",
            name="recycling_rtx3060_yolov8m",
            exist_ok=True,

            pretrained=True,
            patience=15,

            optimizer="AdamW",
            lr0=0.001,
            lrf=0.01,
            weight_decay=0.0005,
            cos_lr=True,

            mosaic=1.0,
            close_mosaic=10,
            fliplr=0.5,
            translate=0.1,
            scale=0.5,

            val=True,
            plots=True,
            save=True,
            save_period=1,  # salvează checkpoint la fiecare epoch
            cache=False,
            amp=True
        )


if __name__ == "__main__":
    freeze_support()
    main()