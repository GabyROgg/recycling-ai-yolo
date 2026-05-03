from ultralytics import YOLO

MODEL_PATH = "runs/detect/recycling_balanced/weights/best.pt"

model = YOLO(MODEL_PATH)

model.export(
    format="openvino",
    imgsz=640
)

print("Export OpenVINO terminat.")