# cameracontroller
A simple Arduino project for controlling camera aperture for incompatible lens.

This is a project for reviving an old film camera. The camera can do metering with the lens but cannot actuate the aperture driver motor. This system activates the aperture motor to the correct aperture automatically whenever the shutter is pressed, although the aperture values has to be set on the arduino system and not the camera.

Basic functionality. Code still needs low power optimization to increase battery life, as it is still draining battery quicker than wanted.
[30-11-2017]: Implemented sleep functionality. But still buggy so disabled for now.
