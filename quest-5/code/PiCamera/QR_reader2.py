from PIL import Image
from pyzbar.pyzbar import decode
import time

# Load the image
image_path = 'QR.jpg'  # Replace with the path to your image file
loaded_image = Image.open(image_path)

# Decode the QR code
data = decode(loaded_image)

if not data:
    print("No QR code found in the image.")
else:
    print(data[0].data.decode('utf-8'))
    
time.sleep(0.25)
# send_udp_data(data[0].data.decode('utf-8'))

