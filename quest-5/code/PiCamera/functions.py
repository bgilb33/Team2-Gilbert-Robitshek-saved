# File: functions.py
# File: functions.py

import argparse
import json
import numpy as np

def print_metadata(metadata):
    print("Metadata:")
    print(metadata)

def main():
    parser = argparse.ArgumentParser(description="Print metadata represented as a NumPy array.")
    parser.add_argument("metadata", type=str, help="Metadata values as a JSON-formatted string")

    args = parser.parse_args()
    metadata_json = args.metadata

    # Deserialize the JSON string to a NumPy array
    metadata_array = np.array(json.loads(metadata_json))

    # Call the function to print metadata
    print_metadata(metadata_array)

if __name__ == "__main__":
    main()

