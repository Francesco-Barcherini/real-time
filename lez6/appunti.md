# Lez. 5 - 30/10/2024
# Cache
Su multi-core ho interferenza nella cache da task eseguiti su altri processori.

`lstopo` per vedere la topologia della cache

## Programma che disturba la cache
`sudo perf record -e cache-misses ./cyclictest`

## alsa
`sudo apt install libasound2-dev`