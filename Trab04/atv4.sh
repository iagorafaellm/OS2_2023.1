#!/bin/bash

if [ $# -ne 1 ]; then # Se o numero de argumentos for diferente de 1
    echo "Uso: $0 <Arquivo>"
    exit 1
fi

if [ ! -f $1 ]; then # Se o arquivo $1 nao existir
    echo "Arquivo $1 não existe"
    exit 1
fi

echo "Arquivo $1 existe"