"""Remove volatile timestamps so identical boards produce identical ZIPs."""
import os
import re
import sys


def main(directory):
    creation_date = re.compile(rb"(%TF\.CreationDate,)[^*]+")
    for name in sorted(os.listdir(directory)):
        path = os.path.join(directory, name)
        if not os.path.isfile(path):
            continue
        with open(path, "rb") as handle:
            content = handle.read()
        normalized = creation_date.sub(rb"\g<1>1980-01-01T00:00:00+00:00", content)
        if normalized != content:
            with open(path, "wb") as handle:
                handle.write(normalized)
        os.utime(path, (315532800, 315532800))


if __name__ == "__main__":
    main(sys.argv[1])
