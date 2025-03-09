#!/bin/bash

echo "Compiling furoxc..."
cd compiler
make clean
make

echo "Running tests..."
python3 tests.py
if [ $? -ne 0 ]; then
    echo "Failed to install furoxc and stdlib"
    exit 1
fi
cd ..

echo "Installing furoxc..."
sudo cp ./compiler/bin/release/furoxc /usr/local/bin/

sudo mkdir -p /usr/local/lib/furox/
sudo rm -rf /usr/local/lib/furox/stdlib/
echo "Installing stdlib..."
sudo cp -r ./stdlib/ /usr/local/lib/furox/

if [ ! -e "/usr/local/lib/furox/std" ]; then
    sudo ln -s /usr/local/lib/furox/stdlib/ /usr/local/lib/furox/std
fi

echo "Successfully installed furoxc and stdlib"
