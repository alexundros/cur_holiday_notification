import os
import sys
import shutil
import sysconfig
from pathlib import Path

from cx_Freeze import Executable, setup as cx_setup
from cx_Freeze.command.build_exe import build_exe as _build_exe


# noinspection PyPep8Naming
class build_exe(_build_exe):
    def run(self):
        super().run()
        build_dir = Path(self.build_exe)
        dll_dir = build_dir / "dll"
        dll_dir.mkdir(exist_ok=True)
        for pattern in ("*.dll", "*.pyd"):
            for fi in build_dir.glob(pattern):
                if fi.name.startswith("python"):
                    continue
                shutil.move(str(fi), dll_dir / fi.name)


platform = sysconfig.get_platform()
build_out = os.path.join("build", platform)

with Path("VERSION").open() as file:
    version = file.read().strip()

appname = "py_project"
cx_setup(
    name=appname,
    version=version,
    description="Календарь торгов по валютам",
    executables=[Executable("src/main.py", icon="assets/python", target_name=appname)],
    cmdclass={"build_exe": build_exe},
    options={
        "build_exe": {
            "include_msvcr": True,
            "excludes": [],
            "includes": [],
            "zip_include_packages": ["*"],
            "zip_exclude_packages": [],
            "build_exe": build_out,
            "include_files": [],
        }
    },
)

pv = f"{sys.version_info.major}.{sys.version_info.minor}"
zip_out = os.path.join("build", f"{appname}-py{pv}-{platform}-{version}")
print(f"\nCreate application zip archive: {zip_out}.zip")
shutil.make_archive(zip_out, "zip", build_out)
