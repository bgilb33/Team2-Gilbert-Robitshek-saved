import socket

def send_udp_message(message):
    ip = "192.168.1.23"
    port = 12345

    try:
        # Create a UDP socket
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
            # Send the message to the specified IP address and port
            udp_socket.sendto(message.encode(), (ip, port))
        print(f"Message sent successfully to {ip}:{port}")
    except Exception as e:
        print(f"Error: {e}")

# Example usage:
if __name__ == "__main__":

    message_to_send = "1,1"

    # Call the function to send the UDP message
    send_udp_message(message_to_send)

