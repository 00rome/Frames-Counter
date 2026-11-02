# FrameForge

An independent Geometry Dash / Geode input-recording and timing-analysis project. Target: Windows, Geometry Dash 2.2081, Geode SDK 5.11.2.

## What this starter build currently implements

- Hooks the game update loop and input handler.
- Records press/release events with game-update tick, player, and button.
- Saves a CSV recording in the mod save folder when the level is quit (if auto-save is enabled).
- Includes Geode settings for tick rate, release capture, and display preferences reserved for the next UI milestone.

## Important status / accuracy note

This is **not yet a feature-complete replacement** for FB Frame Counter. A true click-window scan needs deterministic replay and controlled input-time shifting, then repeated simulations through the level. That engine, playback UI, click-circle overlay, counter rows, manual-count editor, CBF sub-frame timing, Pathfinder, hitbox tools, and video export are not implemented in this first source milestone. The CSV records input events; it does not claim to measure the actual early/late survival window yet.

The configured tick rate is metadata/configuration only at this stage. The mod does not change Geometry Dash's physics rate. For a non-default rate, use a compatible TPS/physics bypass set to the same rate.

## Build (Windows)

1. Install Geode SDK 5.11.2 for Geometry Dash 2.2081.
2. Install Visual Studio 2022/2026 C++ build tools and CMake. Ninja is optional when using the Visual Studio generator.
3. Set `GEODE_SDK` to your SDK directory, e.g. `C:\Users\YourName\GeodeSDK`.
4. Open PowerShell in this folder and run:

   ```powershell
   cmake -S . -B build -A x64
   cmake --build build --config Release
   geode package
   ```

If your Geode SDK uses a different generator setup, use the build command recommended by `geode sdk install` for your installation.

## Recording location

`<Geode save directory>/rome.frameforge/recordings/`

Each CSV starts with metadata comments and then `tick,action,player,button` rows.

## Next implementation milestones

1. Deterministic macro playback and import.
2. Scan runner that moves one input earlier/later and replays to determine valid timing bounds.
3. Window-size classification, configurable counter rows, and click-circle overlay.
4. Recording/playback/analyze/manual-count menu.
5. Advanced scan modes, CBF precision, setup/dependent-click logic, Pathfinder, hitboxes, and export tools.

## GitHub Actions build

This repository includes `.github/workflows/build.yml`. GitHub automatically builds a Windows x64 `.geode` package when you push to a branch, open/update a pull request, or manually run the workflow from the **Actions** tab. It uses the Geode SDK version declared in `mod.json` (`5.11.2`).

### Downloading a build

1. Push the project to a GitHub repository.
2. Open the repository's **Actions** tab and select **Build FrameForge**.
3. Open the latest successful workflow run.
4. Download the `FrameForge-Windows-x64` artifact near the bottom of the run page.
5. Extract the artifact; it contains the `.geode` package to install in Geode.

Pushing a tag beginning with `v` (for example, `v0.1.0`) also creates/updates a GitHub Release and attaches the built `.geode` file.

The workflow builds the current source; it does not add features that are not yet implemented.
