#!/usr/bin/bash

set -e

while [ 1 ]
do
	/usr/local/bin/upgrade-solarcontrol-core.sh
	/usr/local/bin/upgrade-solarcontrol-front.sh

	sleep 21600
done
