# GENESIS V0.1 — C++20 + Web

## What is included
- `core/genesis.cpp`: C++20 controlled hidden-law benchmark, least-squares model fitting, candidate feature comparison, and holdout validation.
- `web/index.html`: responsive browser dashboard and JSON report download UI.

## Compile the C++ engine to WebAssembly
Use Emscripten (emcc) in a Linux/macOS terminal or GitHub Codespaces after installing/activating the Emscripten SDK:

```bash
emcc core/genesis.cpp -O3 -std=c++20 \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS='["_genesis_version","_genesis_run"]' \
  -s EXPORTED_RUNTIME_METHODS='["UTF8ToString"]' \
  -s MODULARIZE=1 -s EXPORT_NAME=createGenesisModule \
  -o web/genesis.js
```

Then add this script before the inline app script in `web/index.html`:

```html
<script src="genesis.js"></script>
```

Serve the `web/` directory over HTTP (not `file://`) and press **RUN V0.1 EXPERIMENT**. The browser loads `genesis.js` and `genesis.wasm` and runs the C++ engine locally.

## Deploy web UI
Publish the `web/` folder with a static host such as Cloudflare Pages. The generated `genesis.js` and `genesis.wasm` must be present in that folder for the experiment button to execute.

## Current limitations
- This is a controlled synthetic benchmark, not a general scientific discovery engine.
- Candidate features are predefined (`1`, `x`, `x²`, `sin(1.7x)`, `cos(1.7x)`).
- No user accounts, cloud compute, database, desktop installer, or mobile package yet.
