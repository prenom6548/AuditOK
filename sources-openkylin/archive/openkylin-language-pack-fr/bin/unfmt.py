import sys
from pathlib import Path

from polib import mofile


def main(directory):
    source_directory = Path(directory)
    if source_directory.is_dir():
        for source in source_directory.glob("*.mo"):
            target = source.with_suffix(".po")
            m = mofile(str(source))
            m.save_as_pofile(str(target))


if __name__ == '__main__':
    main(sys.argv[1])
