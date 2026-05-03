import cv2
import torch
from ultralytics import YOLO

MODEL_PATH = r"runs/detect/recycling_balanced/weights/best.pt"

# Webcam laptop:
SOURCE = 0

# Telefon IP Webcam:
# SOURCE = "http://192.168.1.128:8080/video"

CONF = 0.45
IMGSZ = 640

device = "cuda" if torch.cuda.is_available() else "cpu"

model = YOLO(MODEL_PATH)

cap = cv2.VideoCapture(SOURCE)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

if not cap.isOpened():
    raise RuntimeError("Nu pot deschide camera/stream-ul.")

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    results = model.predict(
        frame,
        conf=CONF,
        imgsz=IMGSZ,
        device=device,
        verbose=False
    )

    annotated = results[0].plot()
    cv2.imshow("Recycling AI", annotated)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()