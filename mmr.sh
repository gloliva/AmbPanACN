#!/bin/bash
make mac && sudo mv AmbPanACN.chug /usr/local/lib/chuck && chuck --srate:48000 $1