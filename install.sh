#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    exec sudo "$0" "$@"
fi

echo "Compiling furoxc..."
cd compiler
make clean
make

echo "Installing furoxc..."
cp ./bin/debug/furoxc /usr/local/bin/
cd ..

mkdir -p /usr/local/lib/furox/
rm -rf /usr/local/lib/furox/stdlib/
echo "Installing stdlib..."
cp -r ./stdlib/ /usr/local/lib/furox/

if [ ! -e "/usr/local/lib/furox/std" ]; then
    ln -s /usr/local/lib/furox/stdlib/ /usr/local/lib/furox/std
fi

echo "Successfully installed furoxc and stdlib"
