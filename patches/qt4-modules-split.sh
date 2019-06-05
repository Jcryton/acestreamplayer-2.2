#!/bin/bash
awk '/diff -ru/{close(f); f="00" ++c "-qt4-modules.patch";next} {print>f;}' 20-modules-qt4.patch
