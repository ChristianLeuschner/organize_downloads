#!/bin/bash

set -e  # sofort beenden bei Fehlern

# Variablen

APP_NAME="filesorter"
BUILD_DIR="build"
INSTALL_BIN_DIR="/usr/local/bin"
EXECUTABLE="$INSTALL_BIN_DIR/$APP_NAME"
CONFIG_DIR="$HOME/.config/$APP_NAME"
SERVICE_DIR="$HOME/.config/systemd/user"

# --- 1. Projekt kompilieren ---

echo "Installing the project..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
cd ..

# Binary installieren (sudo nur hier)

echo "Copying executable to $INSTALL_BIN_DIR..."
sudo cp "$BUILD_DIR/$APP_NAME" "$EXECUTABLE"

# --- 2. Config-Dateien erstellen ---

echo "Creating default configuration..."
mkdir -p "$CONFIG_DIR"
if [ ! -f "$CONFIG_DIR/rules.json" ]; then
if [ -f "./config/rules.json" ]; then
cp ./config/rules.json "$CONFIG_DIR/rules.json"
echo "Default rules copied to $CONFIG_DIR/rules.json"
else
echo "WARNING: rules.json not found in project root. Please create it manually."
fi
else
echo "Config already exists at $CONFIG_DIR/rules.json, skipping."
fi

# --- 3. Systemd User-Service erzeugen ---

echo "Generating systemd user service..."
mkdir -p "$SERVICE_DIR"
cat <<EOF > "$SERVICE_DIR/$APP_NAME.service"
[Unit]
Description=Daemon service to sort files according to their extensions
After=network.target

[Service]
Type=simple
ExecStart=$EXECUTABLE $CONFIG_DIR/rules.json
Restart=on-failure

[Install]
WantedBy=default.target
EOF
echo "Systemd user service created at $SERVICE_DIR/$APP_NAME.service"

# --- 4. User Service starten ---

echo "Starting user service..."
systemctl --user daemon-reload
systemctl --user enable "$APP_NAME.service"
systemctl --user restart "$APP_NAME.service"

echo "---"
echo "Installation finished. Check service log with:"
echo "journalctl --user -u $APP_NAME.service -f"
