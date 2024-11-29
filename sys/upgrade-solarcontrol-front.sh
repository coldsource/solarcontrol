#!/usr/bin/bash

set -e

BIN_DIR=/usr/local/bin

if [ -f /var/www/html/version ]
then
	CUR_VERSION=$(head -n1 /var/www/html/version)
else
	CUR_VERSION=0
fi

# Check if we have a newer version
LAST_VERSION=$(git ls-remote --tags https://github.com/coldsource/solarcontrol-react.git | egrep -o 'tags/[0-9\.]+' | egrep -o '[0-9\.]+' | sort -Vr | head -n1)

if [ $CUR_VERSION == $LAST_VERSION ]
then
	exit
fi

echo "Upgrading front to v$LAST_VERSION"

# Clean build path
rm -rf /tmp/solarcontrol-react

# Clone repo
cd /tmp
git clone https://github.com/coldsource/solarcontrol-react
cd /tmp/solarcontrol-react
git checkout --quiet $LAST_VERSION

npm install
npm run pack

mkdir htdocs/css/dist
sassc htdocs/css/src/main.scss htdocs/css/dist/main.css

rsync -r htdocs/ /var/www/html

# Clean
rm -rf /tmp/solarcontrol-react
