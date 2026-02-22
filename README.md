# Chambisonics

Chambisonics is an Ambisonics package for the ChucK audiovisual programming language. All chugins support orders 1 - 7 and use ACN ordering and SN3D normalization.

Chambisonics contains the following chugins:

1. `AmbiBin`:
Basic binaural decoders with fixed order.

2. `AmbiEnc`:
Encoders with fixed order. Useful for high concurrency of voices. Simple interface that supports changing azimuth and elevation values.

3. `AmbiPan`:
An ambisonics panner with variable order. Supports additional functionality such as movement through a path over time and setting velocity values to change azimuth and elevation values automatically.
