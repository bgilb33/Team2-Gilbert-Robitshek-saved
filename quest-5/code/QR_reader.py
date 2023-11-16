# QR imports
import cv2
from pyzbar.pyzbar import decode

# UDP imports
import socket

SERVER_ADDRESS = '127.0.0.1'  # Replace with the actual server address
SERVER_PORT = 12345  # Replace with the actual server port

def send_udp_data(data):
    # Create a UDP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Server address and port
    server_address = (SERVER_ADDRESS, SERVER_PORT)

    try:
        # Send data to the server
        client_socket.sendto(data.encode('utf-8'), server_address)
        print(f"Data sent to {server_address}: {data}")

    finally:
        # Close the socket
        client_socket.close()

# Function to read QR code from an image
def read_qr_code(image_path):
    # Read the image
    image = cv2.imread(image_path)

    # Decode QR codes
    decoded_objects = decode(image)

    # Print the decoded information
    return decoded_objects[0].data.decode('utf-8')

# QRMeter2Fob1.png
# QRMeter1Fob1.png

def main():
    # Continuous loop to read QR codes
    while True:
        # Get user input for the image path
        image_path = input("Enter the path of the image (or type 'exit' to quit): ")

        if image_path.lower() == 'exit':
            break

        data = read_qr_code(image_path)
        send_udp_data(data)
        
        print(data)

if __name__ == "__main__":
    main()
