#!/bin/bash

if [ ! -d ./.venv ]; then
    python -m venv ./.venv
fi

source ./.venv/bin/activate

FILE_NAME=lambda.zip
rm -rf dist
rm -rf $FILE_NAME
mkdir dist
pip install -r requirements.txt -t dist 
cp -r src/* dist
pushd dist || exit
zip -r ../$FILE_NAME ./*
popd || exit
