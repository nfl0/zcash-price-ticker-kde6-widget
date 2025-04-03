#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
# Default to user installation (~/.local)
INSTALL_PREFIX="${HOME}/.local"
BUILD_DIR="build"
INSTALL_TYPE="user" # 'user' or 'system'

# --- Argument Parsing ---
# Simple check for a '--system' flag
if [[ "$1" == "--system" ]]; then
  INSTALL_PREFIX="/usr"
  INSTALL_TYPE="system"
  echo "Selected system-wide installation (requires sudo)."
else
  echo "Selected user-local installation (to ${INSTALL_PREFIX})."
  # Ensure the target directories exist for user install
  mkdir -p "${INSTALL_PREFIX}/lib64/qt6/plugins/" # Adjust lib64 if needed for your distro (e.g. lib)
  mkdir -p "${INSTALL_PREFIX}/share/plasma/applets/"
fi

# --- Dependency Checks (Basic) ---
echo "Checking for basic build tools..."
command -v cmake >/dev/null 2>&1 || { echo >&2 "Error: cmake is required but not found. Aborting."; exit 1; }
command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || { echo >&2 "Error: C++ compiler (g++ or clang++) not found. Aborting."; exit 1; }
echo "Basic tools found."

echo ""
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "!! Reminder: This script DOES NOT install library           !!"
echo "!! dependencies (Qt6, KF6, ECM). Please ensure you have   !!"
echo "!! installed the required development packages for your     !!"
echo "!! distribution using dnf, apt, pacman, zypper, etc.      !!"
echo "!! (e.g., qt6-base-devel, kf6-plasma-devel, extra-cmake-modules)"
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo ""
read -p "Press Enter to continue if dependencies are installed, or Ctrl+C to cancel..."

# --- Build Steps ---
echo "[1/3] Configuring project with CMake..."
cmake -B "${BUILD_DIR}" -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" -DCMAKE_BUILD_TYPE=Release .

echo "[2/3] Building project..."
cmake --build "${BUILD_DIR}"

echo "[3/3] Installing project..."
if [[ "${INSTALL_TYPE}" == "system" ]]; then
  echo "System installation requires root privileges."
  sudo cmake --install "${BUILD_DIR}"
else
  cmake --install "${BUILD_DIR}"
fi

echo ""
echo "--------------------------------------------------"
echo " Installation Complete!"
echo "--------------------------------------------------"
echo ""
echo "Widget installed to: ${INSTALL_PREFIX}"
if [[ "${INSTALL_TYPE}" == "user" ]]; then
  echo "Note: Ensure Plasma can find user-installed widgets."
  echo "This usually works by default if ~/.local/share is in your XDG_DATA_DIRS environment variable."
fi
echo ""

# --- Plasma Reload (Optional) ---
read -p "Do you want to try reloading PlasmaShell now? (y/N): " -n 1 -r
echo # Move to a new line

if [[ $REPLY =~ ^[Yy]$ ]]; then
  echo "Reloading PlasmaShell..."
  command -v kquitapp6 >/dev/null 2>&1 && command -v plasmashell >/dev/null 2>&1 || { echo >&2 "Warning: kquitapp6 or plasmashell not found in PATH. Cannot reload automatically."; exit 0; }

  kquitapp6 plasmashell || echo "kquitapp6 command failed (maybe Plasma wasn't running?)"
  sleep 2 # Give it a moment to shut down
  # Start plasmashell in the background
  (plasmashell > /dev/null 2>&1 &)
  echo "PlasmaShell restart initiated."
else
  echo "Please reload PlasmaShell manually to see the widget:"
  echo "  Press Alt+F2, type 'kquitapp6 plasmashell', Enter."
  echo "  Press Alt+F2, type 'plasmashell', Enter."
fi

echo ""
echo "Done. You should now be able to add 'ZEC Price Ticker (C++)' as a widget."