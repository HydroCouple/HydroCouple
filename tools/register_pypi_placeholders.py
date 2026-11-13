#!/usr/bin/env python3
"""Claim PyPI names for HydroCouple projects by uploading a placeholder release.

PyPI creates a project on its first upload, so claiming a name before the real
release workflow runs means uploading something. This script builds a tiny,
pure-Python placeholder for each project and uploads it with twine:

* version ``0.0.0.dev0`` -- a pre-release below every real version, so once a
  real release exists ``pip install <name>`` never selects the placeholder
  (PyPI never lets a version number be reused, so the real ones stay free);
* importing it raises ImportError pointing at the real project;
* projects that already exist on the index are skipped.

The API token is read from ``PYPI_API_TOKEN`` or prompted for without echo. It is
handed to twine through its environment (TWINE_USERNAME/TWINE_PASSWORD), never on
a command line, and never written or printed. Creating a project needs an
account-scoped token; revoke it once the names exist and configure trusted
publishing instead (the steps are printed at the end).

Usage::

    python -m pip install build twine
    python tools/register_pypi_placeholders.py --dry-run   # build + twine check only
    python tools/register_pypi_placeholders.py --test      # upload to TestPyPI
    python tools/register_pypi_placeholders.py             # upload to PyPI

Generated packages are kept under ``build/pypi-placeholders/`` for inspection.
"""

from __future__ import annotations

import argparse
import getpass
import os
import shutil
import subprocess
import sys
import textwrap
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
VERSION = "0.0.0.dev0"

PROJECTS = {
    "hydrocouple": {
        "summary": "Python interface definitions for the HydroCouple component-based modeling framework",
        "repository": "https://github.com/HydroCouple/HydroCouple",
        "license": "MIT",
    },
    "meshkit": {
        "summary": "Independent mixed meshing with terrain certificates and streaming I/O",
        "repository": "https://github.com/HydroCouple/Meshkit",
        "license": "Apache-2.0",
    },
}

INDEXES = {
    "pypi": ("https://upload.pypi.org/legacy/", "https://pypi.org"),
    "testpypi": ("https://test.pypi.org/legacy/", "https://test.pypi.org"),
}


def exists_on(index_url: str, name: str) -> bool:
    """Whether `name` is already a project on the index (its JSON API answers 200)."""
    try:
        with urllib.request.urlopen(f"{index_url}/pypi/{name}/json", timeout=20) as reply:
            return reply.status == 200
    except urllib.error.HTTPError as error:
        if error.code == 404:
            return False
        raise


def write_placeholder(directory: Path, name: str, info: dict) -> None:
    """A minimal project whose only module refuses to be used."""
    if directory.exists():
        shutil.rmtree(directory)
    package = directory / name
    package.mkdir(parents=True)
    (directory / "pyproject.toml").write_text(textwrap.dedent(f"""\
        [build-system]
        requires = ["setuptools>=68"]
        build-backend = "setuptools.build_meta"

        [project]
        name = "{name}"
        version = "{VERSION}"
        description = "Name placeholder for {name}: {info['summary']}"
        readme = "README.md"
        requires-python = ">=3.10"
        license = {{text = "{info['license']}"}}
        authors = [{{name = "Caleb Buahin"}}]
        classifiers = ["Development Status :: 1 - Planning"]

        [project.urls]
        Source = "{info['repository']}"

        [tool.setuptools]
        packages = ["{name}"]
        """), encoding="utf-8")
    (directory / "README.md").write_text(textwrap.dedent(f"""\
        # {name} (placeholder)

        This release only reserves the name. {info['summary']}.

        Real releases are built and published from
        <{info['repository']}>; install one with `pip install {name}` once it
        exists (this placeholder, {VERSION}, is a pre-release below every real
        version and is never selected over one).
        """), encoding="utf-8")
    (package / "__init__.py").write_text(textwrap.dedent(f'''\
        raise ImportError(
            "{name} {VERSION} is a name placeholder with no code. Install a real "
            "release of {name}; see {info['repository']}"
        )
        '''), encoding="utf-8")


def run(command: list[str], **kwargs) -> None:
    subprocess.run(command, check=True, **kwargs)


def main() -> int:
    parser = argparse.ArgumentParser(description="Claim PyPI names with placeholder releases.")
    parser.add_argument("--project", action="append", choices=sorted(PROJECTS),
                        help="project to claim (repeatable; default: all)")
    parser.add_argument("--test", action="store_true", help="upload to TestPyPI instead of PyPI")
    parser.add_argument("--dry-run", action="store_true",
                        help="build and check the distributions; upload nothing")
    parser.add_argument("--out", type=Path, default=ROOT / "build" / "pypi-placeholders",
                        help="where the placeholder projects and distributions are written")
    args = parser.parse_args()

    for module in ("build", "twine"):
        if subprocess.run([sys.executable, "-c", f"import {module}"],
                          capture_output=True).returncode != 0:
            print(f"missing '{module}': run  {sys.executable} -m pip install build twine")
            return 2

    upload_url, index_url = INDEXES["testpypi" if args.test else "pypi"]
    names = args.project or sorted(PROJECTS)

    to_upload = []
    for name in names:
        if exists_on(index_url, name):
            print(f"{name}: already a project on {index_url}; skipped")
            continue
        source = args.out / name
        dist = args.out / "dist" / name
        write_placeholder(source, name, PROJECTS[name])
        if dist.exists():
            shutil.rmtree(dist)
        run([sys.executable, "-m", "build", "--sdist", "--wheel", "--outdir", str(dist), str(source)],
            stdout=subprocess.DEVNULL)
        files = sorted(str(p) for p in dist.iterdir())
        run([sys.executable, "-m", "twine", "check", "--strict", *files])
        to_upload.append((name, files))
        print(f"{name}: built {', '.join(Path(f).name for f in files)}")

    if not to_upload:
        return 0
    if args.dry_run:
        print(f"dry run: nothing uploaded; distributions are under {args.out / 'dist'}")
        return 0

    token = os.environ.get("PYPI_API_TOKEN") or getpass.getpass(
        f"API token for {index_url} (input hidden): ")
    if not token.startswith("pypi-"):
        print("that does not look like a PyPI API token (they start with 'pypi-'); nothing uploaded")
        return 2
    env = dict(os.environ, TWINE_USERNAME="__token__", TWINE_PASSWORD=token,
               TWINE_NON_INTERACTIVE="1")
    for name, files in to_upload:
        run([sys.executable, "-m", "twine", "upload", "--repository-url", upload_url, *files], env=env)
        print(f"{name}: {index_url}/project/{name}/")

    print(textwrap.dedent(f"""
        Next, for each project on {index_url}: Manage project -> Publishing -> add a
        GitHub trusted publisher (owner HydroCouple; repository HydroCouple or Meshkit;
        workflow release_wheels.yml; environment pypi). Then revoke the token you used
        here. After a project's first real release, yank {VERSION} (Manage -> Releases)."""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
