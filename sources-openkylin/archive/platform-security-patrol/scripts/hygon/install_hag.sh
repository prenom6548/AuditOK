#!/bin/bash

# Default installation path
INSTALL_PATH="/opt/hygon"

echo "Installing hag to $INSTALL_PATH/bin"
mkdir -p -- "$INSTALL_PATH/bin" || { 
    echo "Failed to create directory: $INSTALL_PATH/bin" >&2
    exit 1
}

# Download the hag file using the wget command
rm -rf "$INSTALL_PATH/bin/hag"
wget -c https://download.sourcefind.cn:65024/directlink/8/c86-devkit/previous-release/bin/hag -O "$INSTALL_PATH/bin/hag" || {
    echo "Failed to download hag file."
    exit 1
}

# Set executable permission for the hag binary
chmod +x "$INSTALL_PATH/bin/hag"
echo "hag installed to $INSTALL_PATH/bin successfully!"