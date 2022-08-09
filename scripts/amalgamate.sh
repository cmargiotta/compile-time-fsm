#!/bin/sh

if command -v quom &> /dev/null
then
    echo "Amalgamating headers"
	quom src/fsm/fsm.hpp -I src/ ./single-include/fsm.hpp
    exit
fi

echo "quom not found, cannot amalgamate headers."