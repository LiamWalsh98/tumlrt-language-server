#!/bin/sh
(cd org.xtext.example.mydsl.parent && ./gradlew build shadowJar)
npm install -g vsce
(cd vscode-extension && npm install && vsce package)
(cd vscode-extension-self-contained && npm install && vsce package)