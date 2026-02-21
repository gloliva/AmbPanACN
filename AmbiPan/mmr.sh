#!/bin/bash
make mac && sudo mv AmbiPan.chug /usr/local/lib/chuck && chuck --srate:48000 $1