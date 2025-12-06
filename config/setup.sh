#!/bin/bash

set -e # Exit immediately if a command exits with a non-zero status

# Variables 
APP_NAME="filesorter"
BUILD_DIR="build"
INSTALL_BIN_DIR="/usr/local/bin"
EXECUTABLE="$INSTALL_BIN_DIR/$APP_NAME"
CONFIG_DIR="$HOME/.config/$APP_NAME"
SERVICE_DIR="$HOME/.config/systemd/user"

# 1. ---compile project---
echo "Installing the project..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release .. # configure CMake 
make -j$(nproc)
cd ..

mkdir -p "$INSTALL_BIN_DIR"
cp "$BUILD_DIR/$APP_NAME" "$EXECUTABLE" # copy executable to install directory

# 2. create config file
echo "Creating default configuration..."
mkdir -p "$CONFIG_DIR"
cp -i ./rules.json "$CONFIG_DIR/rules.json" # copy default rules
echo "Rules are configured in $CONFIG_DIR/rules.json."

# 3. generate systemd service
echo "Generating systemd service..."
mkdir -p "$SERVICE_DIR"
cat <<EOF > "$SERVICE_DIR/$APP_NAME.service"
[Unit]
Description=Daemon service to fort files regarding their file extensions.
After=network.target

[Service]
Type=simple

# ExecStart ruft die installierte Binary mit der Config-Datei als Argument auf
ExecStart=$EXECUTABLE $CONFIG_DIR/rules.json

# Sorgt dafür, dass der Service bei einem Absturz neu gestartet wird
Restart=on-failure

[Install]
# Stellt sicher, dass der Service beim Login des Benutzers gestartet wird
WantedBy=default.target
EOF
echo "Systemd service created at $SERVICE_DIR/$APP_NAME.service."

# 4. start user service
echo "Starting user service..."
systemctl --user daemon-reload
systemctl --user enable "$APP_NAME.service"
systemctl --user restart "$APP_NAME.service"

echo "---"
echo "Installation finished. Test status with:"
echo "journalctl --user -u $APP_NAME.service -f"