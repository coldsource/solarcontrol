#!/usr/bin/bash

set -e

# Install all required packages
apt-get install -y sudo build-essential cmake git mariadb-server libmariadb-dev mosquitto libmosquitto-dev libwebsockets-dev libcurl4-gnutls-dev nginx npm sassc

# Configure mosquitto
echo -e "listener 1883 0.0.0.0\nallow_anonymous true" >/etc/mosquitto/conf.d/default.conf
systemctl restart mosquitto

# Configure database
mysql <prepare-db.sql

# Create user for solarcontrol
mkdir /home/solarcontrol
groupadd --system solarcontrol
useradd --system -g solarcontrol -d /home/solarcontrol solarcontrol
chown solarcontrol:solarcontrol /home/solarcontrol

echo "solarcontrol ALL = (root) NOPASSWD: /usr/bin/mv /tmp/solarcontrol/build/solarcontrol /usr/local/bin/solarcontrol" >/etc/sudoers.d/solarcontrol
echo "solarcontrol ALL = (root) NOPASSWD: /usr/bin/systemctl restart solarcontrol" >>/etc/sudoers.d/solarcontrol

# Prepare solarcontrol service
cp solarcontrol.conf /etc

cp solarcontrol.service /lib/systemd/system
systemctl daemon-reload
systemctl enable solarcontrol

# Prepare solarcontrol-upgrade service
cp upgrade-solarcontrol.sh /usr/local/bin
cp upgrade-solarcontrol-core.sh /usr/local/bin
cp upgrade-solarcontrol-front.sh /usr/local/bin
cp solarcontrol-upgrade.service /lib/systemd/system

systemctl daemon-reload
systemctl enable solarcontrol-upgrade
systemctl restart solarcontrol-upgrade

# Prepre nginx
rm -f /var/www/html/*
chown solarcontrol:solarcontrol /var/www/html

echo
echo
echo "Solar control is now installed"
