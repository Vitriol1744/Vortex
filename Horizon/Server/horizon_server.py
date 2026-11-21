#!/usr/bin/env python3
#
# Created by v1tr10l7 on 21.11.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
"""Horizon's test release server.

Hosts a release manifest plus release archives that Horizon (the Vortex
launcher) downloads, verifies and installs. Everything lives in a plain
directory (Horizon/Server/www by default), so it's served the same way real
static hosting would serve it.

  horizon_server.py package [--version 0.2.0]   pack bin/Nexus into a release
  horizon_server.py fake --version 0.0.1        a tiny stub release, for testing
  horizon_server.py list                        releases in the manifest
  horizon_server.py remove 0.0.1                delete a release
  horizon_server.py verify                      re-check sizes and hashes
  horizon_server.py serve [--port 8000]         serve it on localhost

Horizon reads http://localhost:8000/manifest.json by default.
"""

from __future__ import annotations

import argparse
import datetime as dt
import functools
import hashlib
import http.server
import json
import os
import platform
import re
import shutil
import socketserver
import stat
import subprocess
import sys
import tarfile
import tempfile
import time
from pathlib import Path

SCHEMA = 1
SERVER_DIR = Path(__file__).resolve().parent
REPO_ROOT = SERVER_DIR.parent.parent
DEFAULT_ROOT = SERVER_DIR / "www"


# -- Manifest ----------------------------------------------------------------


def host_platform() -> str:
    """Matches Horizon::HostPlatform()."""
    system = {"linux": "linux", "win32": "windows", "darwin": "macos"}.get(
        sys.platform, sys.platform
    )
    machine = platform.machine().lower()
    arch = {"x86_64": "x86_64", "amd64": "x86_64", "aarch64": "arm64", "arm64": "arm64"}
    return f"{system}-{arch.get(machine, 'unknown')}"


def version_key(version: str):
    """Same order as Horizon::CompareVersions: numeric parts, and a
    pre-release suffix ("0.2.0-nightly") before the plain version."""
    core, _, suffix = version.partition("-")
    parts = [int(p) if p.isdigit() else 0 for p in core.split(".")]
    while parts and parts[-1] == 0:
        parts.pop()
    return (parts, 0 if suffix else 1, suffix)


def load_manifest(root: Path) -> dict:
    path = root / "manifest.json"
    if not path.exists():
        return {"schema": SCHEMA, "releases": []}
    manifest = json.loads(path.read_text())
    manifest.setdefault("releases", [])
    return manifest


def save_manifest(root: Path, manifest: dict) -> None:
    manifest["schema"] = SCHEMA
    manifest["releases"].sort(key=lambda r: version_key(r["version"]), reverse=True)
    root.mkdir(parents=True, exist_ok=True)
    temporary = root / "manifest.json.tmp"
    temporary.write_text(json.dumps(manifest, indent=2) + "\n")
    temporary.replace(root / "manifest.json")


def next_version(manifest: dict) -> str:
    """One patch above the newest plain release, or 0.1.0."""
    plain = [r["version"] for r in manifest["releases"] if "-" not in r["version"]]
    if not plain:
        return "0.1.0"
    newest = max(plain, key=version_key)
    parts = [int(p) for p in newest.split(".")] + [0, 0, 0]
    return f"{parts[0]}.{parts[1]}.{parts[2] + 1}"


def sha256_of(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def git_commit() -> str:
    try:
        return subprocess.run(
            ["git", "-C", str(REPO_ROOT), "rev-parse", "--short", "HEAD"],
            capture_output=True, text=True, check=True,
        ).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def publish(root: Path, version: str, channel: str, notes: str, stage: Path,
            components: dict | None = None) -> dict:
    """Packs `stage` (one top-level directory) into
    releases/<version>/vortex-<version>-<platform>.tar.gz and records it."""
    manifest = load_manifest(root)
    if not re.fullmatch(r"\d+(\.\d+)*(-[0-9A-Za-z.]+)?", version):
        sys.exit(f"'{version}' isn't a version like 0.2.0 or 0.3.0-nightly")

    target = host_platform()
    name = f"vortex-{version}-{target}.tar.gz"
    out_dir = root / "releases" / version
    out_dir.mkdir(parents=True, exist_ok=True)
    archive = out_dir / name

    print(f"  packing {archive.relative_to(root)}")
    tar = shutil.which("tar")
    if tar:
        # pigz makes the 100+ MB editor archives a lot faster when present.
        compress = ["--use-compress-program=pigz"] if shutil.which("pigz") else ["-z"]
        subprocess.run([tar, "-c", *compress, "-f", str(archive), "-C",
                        str(stage.parent), stage.name], check=True)
    else:
        with tarfile.open(archive, "w:gz") as tf:
            tf.add(stage, arcname=stage.name)

    artifact = {
        "platform": target,
        "url": f"releases/{version}/{name}",
        "size": archive.stat().st_size,
        "sha256": sha256_of(archive),
        "executable": "Nexus.exe" if target.startswith("windows") else "Nexus",
    }
    release: dict | None = next((r for r in manifest["releases"] if r["version"] == version), None)
    if release is None:
        release = {"version": version}
        manifest["releases"].append(release)
    release.update({
        "channel": channel,
        "date": dt.date.today().isoformat(),
        "notes": notes,
        "components": components or {"nexus": version, "vortex": version},
    })
    # One artifact per platform: re-packing replaces this platform's.
    release["artifacts"] = [a for a in release.get("artifacts", [])
                            if a.get("platform") != target] + [artifact]
    save_manifest(root, manifest)
    print(f"  {version} ({channel}): {artifact['size'] / 1e6:.1f} MB, "
          f"sha256 {artifact['sha256'][:16]}...")
    return release


# -- package: a real release from the dev tree -------------------------------


def shared_dependencies(binary: Path) -> list[Path]:
    """The libraries `binary` loads from outside the system directories,
    i.e. the ones a release must ship (Vortex-*.so, assimp, imgui...)."""
    output = subprocess.run(["ldd", str(binary)], capture_output=True, text=True,
                            check=True).stdout
    libraries = []
    for line in output.splitlines():
        match = re.search(r"=>\s+(\S+)\s+\(", line)
        if "not found" in line:
            sys.exit(f"{binary.name} can't find a library: {line.strip()}")
        if not match:
            continue
        path = Path(match.group(1))
        if not str(path).startswith(("/usr/", "/lib", "/opt/", "/nix/")):
            libraries.append(path)
    return libraries


def copy_tree(source: Path, destination: Path) -> None:
    shutil.copytree(source, destination, symlinks=False, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns("*.pdb", ".DS_Store"))


def cmd_package(args) -> None:
    root = Path(args.root)
    manifest = load_manifest(root)
    version = args.version or next_version(manifest)
    editor = Path(args.editor).resolve()
    if not editor.exists():
        sys.exit(f"No editor at {editor}; build it first (./build.sh -t Nexus)")
    if not shutil.which("patchelf"):
        print("  warning: patchelf isn't installed; the release keeps the build's "
              "RUNPATH and relies on Horizon setting LD_LIBRARY_PATH")

    with tempfile.TemporaryDirectory(prefix="horizon-stage-", dir=root.parent) as tmp:
        stage = Path(tmp) / f"vortex-{version}"
        (stage / "lib").mkdir(parents=True)

        print(f"Packaging Vortex {version} from {editor}")
        shutil.copy2(editor, stage / editor.name)
        libraries = shared_dependencies(editor)
        for library in libraries:
            # Copy the real file under the soname the binary asked for.
            shutil.copy2(library.resolve(), stage / "lib" / library.name)
        print(f"  {len(libraries)} shared libraries")

        binaries = [stage / editor.name, *(stage / "lib").iterdir()]
        for binary in binaries:
            binary.chmod(binary.stat().st_mode | stat.S_IWUSR)
            if not args.keep_symbols and shutil.which("strip"):
                subprocess.run(["strip", "--strip-debug", str(binary)], check=True)
            if shutil.which("patchelf"):
                rpath = "$ORIGIN/lib" if binary.parent == stage else "$ORIGIN"
                subprocess.run(["patchelf", "--set-rpath", rpath, str(binary)], check=True)

        # Nexus loads these relative to its working directory, which Horizon
        # sets to the release root.
        for directory in ("assets", "DotNet"):
            source = REPO_ROOT / directory
            if source.exists():
                copy_tree(source, stage / directory)
        (stage / "VERSION").write_text(f"{version}\n{git_commit()}\n")

        notes = args.notes or f"Built from {git_commit() or 'the working tree'}."
        publish(root, version, args.channel, notes, stage)
    print(f"Done. Serve it with: {Path(sys.argv[0]).name} serve")


# -- fake: a stub release to test the download/install/launch flow ----------


STUB_NEXUS = """#!/bin/sh
# Stub editor from `horizon_server.py fake`. Records how Horizon started it.
log="${{XDG_DATA_HOME:-$HOME/.local/share}}/vortex/stub-launches.log"
mkdir -p "$(dirname "$log")"
echo "$(date -Is) Vortex {version} stub, cwd=$(pwd), args: $*" >> "$log"
echo "Nexus stub {version} started with: $*"
"""


def cmd_fake(args) -> None:
    root = Path(args.root)
    version = args.version or next_version(load_manifest(root))
    with tempfile.TemporaryDirectory(prefix="horizon-stage-", dir=root.parent) as tmp:
        stage = Path(tmp) / f"vortex-{version}"
        (stage / "lib").mkdir(parents=True)
        (stage / "assets").mkdir()
        nexus = stage / "Nexus"
        nexus.write_text(STUB_NEXUS.format(version=version))
        nexus.chmod(0o755)
        (stage / "VERSION").write_text(f"{version}\nstub\n")
        if args.size_mb:
            # Random bytes don't compress, so the archive really is this
            # big and the progress bar has something to show.
            with (stage / "assets" / "padding.bin").open("wb") as file:
                for _ in range(args.size_mb):
                    file.write(os.urandom(1 << 20))
        print(f"Packaging stub Vortex {version}")
        publish(root, version, args.channel, args.notes or "Stub release for testing Horizon.",
                stage)


# -- list / remove / verify --------------------------------------------------


def cmd_list(args) -> None:
    manifest = load_manifest(Path(args.root))
    if not manifest["releases"]:
        print("No releases yet. Try: horizon_server.py fake --version 0.0.1")
        return
    for release in manifest["releases"]:
        platforms = ", ".join(
            f"{a['platform']} {a.get('size', 0) / 1e6:.1f} MB" for a in release.get("artifacts", [])
        )
        print(f"{release['version']:<16} {release.get('channel', 'stable'):<9} "
              f"{release.get('date', ''):<11} {platforms}")


def cmd_remove(args) -> None:
    root = Path(args.root)
    manifest = load_manifest(root)
    before = len(manifest["releases"])
    manifest["releases"] = [r for r in manifest["releases"] if r["version"] != args.version]
    if len(manifest["releases"]) == before:
        sys.exit(f"No release {args.version}")
    shutil.rmtree(root / "releases" / args.version, ignore_errors=True)
    save_manifest(root, manifest)
    print(f"Removed {args.version}")


def cmd_verify(args) -> None:
    root = Path(args.root)
    bad = 0
    for release in load_manifest(root)["releases"]:
        for artifact in release.get("artifacts", []):
            path = root / artifact["url"]
            if not path.exists():
                problem = "missing"
            elif path.stat().st_size != artifact.get("size"):
                problem = f"size {path.stat().st_size} != {artifact.get('size')}"
            elif sha256_of(path) != artifact.get("sha256"):
                problem = "sha256 mismatch"
            else:
                problem = ""
            bad += bool(problem)
            print(f"{release['version']:<16} {artifact['platform']:<16} {problem or 'ok'}")
    sys.exit(1 if bad else 0)


# -- serve ---------------------------------------------------------------------


class ReleaseHandler(http.server.SimpleHTTPRequestHandler):
    throttle = 0  # bytes per second; 0 = unlimited

    def end_headers(self):
        # The manifest changes whenever a release is published.
        if self.path.split("?")[0].endswith(".json"):
            self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def guess_type(self, path):
        if str(path).endswith(".tar.gz"):
            return "application/gzip"
        return super().guess_type(path)

    def copyfile(self, source, outputfile):
        if not self.throttle:
            return super().copyfile(source, outputfile)
        # Slow downloads make the launcher's progress and cancel testable.
        chunk = max(4096, self.throttle // 20)
        while data := source.read(chunk):
            outputfile.write(data)
            time.sleep(len(data) / self.throttle)

    def log_message(self, format, *args):
        sys.stderr.write(f"{self.log_date_time_string()}  {format % args}\n")


class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True
    allow_reuse_address = True


def cmd_serve(args) -> None:
    root = Path(args.root).resolve()
    root.mkdir(parents=True, exist_ok=True)
    if not (root / "manifest.json").exists():
        save_manifest(root, {"releases": []})
    ReleaseHandler.throttle = int(args.throttle * 1024)
    handler = functools.partial(ReleaseHandler, directory=str(root))
    with Server((args.bind, args.port), handler) as server:
        host = "localhost" if args.bind in ("127.0.0.1", "0.0.0.0", "") else args.bind
        print(f"Serving {root}")
        print(f"Manifest: http://{host}:{args.port}/manifest.json"
              + (f"  (throttled to {args.throttle:g} KB/s)" if args.throttle else ""),
              flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print()


# -- CLI -----------------------------------------------------------------------


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", default=str(DEFAULT_ROOT),
                        help="directory holding manifest.json and releases/ "
                             "(default: %(default)s)")
    commands = parser.add_subparsers(dest="command", required=True)

    package = commands.add_parser("package", help="pack the built editor into a release")
    package.add_argument("--version", help="default: one patch above the newest release")
    package.add_argument("--channel", default="stable", help="stable, preview, nightly...")
    package.add_argument("--notes", default="", help="release notes shown in Horizon")
    package.add_argument("--editor", default=str(REPO_ROOT / "bin" / "Nexus"),
                         help="the Nexus executable (default: %(default)s)")
    package.add_argument("--keep-symbols", action="store_true",
                         help="don't strip debug info (the archive gets much bigger)")
    package.set_defaults(func=cmd_package)

    fake = commands.add_parser("fake", help="publish a stub release for testing")
    fake.add_argument("--version", help="default: one patch above the newest release")
    fake.add_argument("--channel", default="stable")
    fake.add_argument("--notes", default="")
    fake.add_argument("--size-mb", type=int, default=0,
                      help="pad the archive to roughly this size")
    fake.set_defaults(func=cmd_fake)

    commands.add_parser("list", help="list releases").set_defaults(func=cmd_list)

    remove = commands.add_parser("remove", help="delete a release")
    remove.add_argument("version")
    remove.set_defaults(func=cmd_remove)

    commands.add_parser("verify", help="re-check archive sizes and hashes"
                        ).set_defaults(func=cmd_verify)

    serve = commands.add_parser("serve", help="serve the releases over HTTP")
    serve.add_argument("--port", type=int, default=8000)
    serve.add_argument("--bind", default="127.0.0.1",
                       help="address to listen on; 0.0.0.0 for the whole network")
    serve.add_argument("--throttle", type=float, default=0, metavar="KB/S",
                       help="limit each download's speed, to watch progress")
    serve.set_defaults(func=cmd_serve)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
