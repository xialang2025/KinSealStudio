"""Capture this prototype's Qt preview offscreen without touching the user desktop."""  #20261107-6 add

import os
from pathlib import Path
import subprocess
import sys


def main():  #20261107-6 add
    """Render the project's own preview through Qt's offscreen platform plugin."""
    project_root = Path(__file__).resolve().parents[1]
    executable = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else (
        project_root / "build" / "KinSealStudioLite.exe"
    )
    output_path = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else (
        project_root / "artifacts" / "KinSealStudioLite-preview.png"
    )
    environment = os.environ.copy()

    if not executable.is_file():
        raise SystemExit("Executable not found: %s" % executable)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    environment["QT_QPA_PLATFORM"] = "offscreen"
    windows_directory = Path(os.environ.get("WINDIR", "C:/Windows"))
    font_directory = windows_directory / "Fonts"
    if font_directory.is_dir():
        environment["QT_QPA_FONTDIR"] = str(font_directory)
    result = subprocess.run(
        [str(executable), "--capture-preview", str(output_path)],
        cwd=str(project_root),
        env=environment,
        check=False,
    )
    if result.returncode != 0:
        raise SystemExit("Offscreen capture failed with exit code %d" % result.returncode)
    if not output_path.is_file():
        raise SystemExit("The application did not create the preview image.")
    print(output_path)


if __name__ == "__main__":
    main()
