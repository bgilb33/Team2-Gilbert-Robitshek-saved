from PIL import Image
from pyzbar.pyzbar import decode


data = decode(Image.open('hi.jpg'))
if(data == []):
    print("not an image")
else:
    print(data)
