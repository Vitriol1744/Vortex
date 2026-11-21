# Horizon test release server

`horizon_server.py` hosts releases that Horizon (the Vortex launcher) can
download, verify and install. Everything is a static file under `www/`
(gitignored), the same layout real hosting would serve.

```sh
# Pack the built editor (bin/Nexus, its shared libraries, assets/, DotNet/)
# as the next version, or a given one:
python3 Horizon/Server/horizon_server.py package --notes "What changed"
python3 Horizon/Server/horizon_server.py package --version 0.3.0-nightly --channel nightly

# A tiny stub release, for testing the launcher without a 100 MB archive:
python3 Horizon/Server/horizon_server.py fake --version 0.0.1 --size-mb 5

python3 Horizon/Server/horizon_server.py list
python3 Horizon/Server/horizon_server.py verify     # re-check sizes and sha256
python3 Horizon/Server/horizon_server.py remove 0.0.1

# Serve on http://localhost:8000/manifest.json (Horizon's default URL).
# --throttle (KB/s) slows downloads so the progress bar and Cancel can be seen.
python3 Horizon/Server/horizon_server.py serve --throttle 2048
```

Then open Horizon's Engines page, or install from a terminal:

```sh
./bin/Horizon --list-releases
./bin/Horizon --install latest
./bin/Horizon --manifest http://localhost:9000/manifest.json --install 0.2.0
```

## Manifest

```json
{
  "schema": 1,
  "releases": [{
    "version": "0.2.0", "channel": "stable", "date": "2026-11-21",
    "notes": "...",
    "components": { "nexus": "0.2.0", "vortex": "0.2.0" },
    "artifacts": [{
      "platform": "linux-x86_64",
      "url": "releases/0.2.0/vortex-0.2.0-linux-x86_64.tar.gz",
      "size": 107374182, "sha256": "...", "executable": "Nexus"
    }]
  }]
}
```

URLs are relative to the manifest, so the same `www/` works from this server,
from `file://`, or from any static host. Horizon refuses an archive whose
sha256 doesn't match, and one without a sha256 unless the manifest is a local
`file://`.

A packaged release is one top-level directory (`vortex-<version>/`) holding
`Nexus`, `lib/` (its shared libraries, found through `$ORIGIN/lib`), `assets/`
and `DotNet/`. Horizon unpacks it into its engines directory and runs Nexus
from there.
