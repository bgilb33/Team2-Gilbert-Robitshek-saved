import time
import subprocess
import os
from picamera2 import Picamera2

#def capture_and_save_array(camera):
#    metadata = camera.capture_array()
#    picam2.capture_file("QR.jpg")

def activate_virtual_environment(venv_path):
    activate_cmd = f"bash -c 'source {venv_path} && python QR_reader2.py'"
    subprocess.run(activate_cmd, shell=True)

def main():
    try:
        while True:  # Infinite loop
            picam2 = Picamera2()

            preview_config = picam2.create_preview_configuration(main={"size": (800, 600)})
            picam2.configure(preview_config)

            picam2.start()
            time.sleep(.25)
            
            picam2.capture_file("QR.jpg")
            time.sleep(.25)
            

            venv_path = os.path.join("env", "bin", "activate")
            activate_virtual_environment(venv_path)

            picam2.close()

            # Add a delay between iterations to avoid continuous execution
            time.sleep(10)  # Adjust the delay duration as needed

    except KeyboardInterrupt:
        # Break the loop if KeyboardInterrupt (Ctrl+C) is detected
        pass

if __name__ == "__main__":
    main()

