import os

def count_ino_files():
    ino_count = 0
    # Walk through all directories and files in the repository
    for root, dirs, files in os.walk('.'):
        # Skip hidden folders like .git or .github
        if any(part.startswith('.') for part in root.split(os.sep)):
            continue
        for file in files:
            if file.endswith('.ino'):
                ino_count += 1

    print("=" * 40)
    print(f"📊 Total Arduino (.ino) files found: {ino_count}")
    print("=" * 40)

if __name__ == "__main__":
    count_ino_files()
