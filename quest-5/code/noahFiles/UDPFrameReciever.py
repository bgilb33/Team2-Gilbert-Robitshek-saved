import socket
import cv2
import numpy as np

UDP_IP = "0.0.0.0"
UDP_PORT = 10001

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

cv2.namedWindow('Received Frame', cv2.WINDOW_NORMAL)

while True:
    data, addr = sock.recvfrom(65535)  # Increase buffer size
    frame = np.frombuffer(data, dtype=np.uint8)
    
    # Assuming the video stream is in H.264 format
    # You may need to adjust the decoding based on your actual format
    
    # This example uses the OpenCV library for decoding
    decoded_frame = cv2.imdecode(frame, cv2.IMREAD_COLOR)
    # Check if decoding was successful and frame size is valid
    if decoded_frame is not None and decoded_frame.shape[0] > 0 and decoded_frame.shape[1] > 0:
        cv2.imshow('Received Frame', decoded_frame)
        
    # Break the loop if 'q' key is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
sock.close()
