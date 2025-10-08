# Solar Control

## What is it

Solar Control is photovoltaic controller aimed to help the best possible auto consumption of solar energy. It monitors production and consumption of whole house as well as specific devices and can switch on/off specific devices when enough solar only power is available (offload)

Is supports Hot Water System (HWS) through solar router, Heaters, Coolers, Controlled Mecanical Ventilation (CMV) as well as generic domestic devices.

It can be associated with hygrometer and thermometers to allow intelligent control of these devices

## How it works

Solar Control is free software (GPL v3) so you can use it as you want. It's designed to be installed on small devices such as Raspberry PI. It's design in C++ ensures very low resource consumption.

It features a [Web front-end](https://github.com/coldsource/solarcontrol-react) that can be used from a computer or phone (it's specifically designed for mobile use).

## What hardware do I need

Solar Control is designed to work with a wide variety of Shelly devices to monitor power and control devices.

It supports Shelly Pro EM (and 3EM), Pro, Plus, H&T (both wifi and bluetooth).

Devices can be auto-detected so configuration is pretty easy.

The controller itself can run on any computer running Linux with at least 1GB of memory. I recommand using Raspberry PI 3 (or higher) to run it as it's proven to work perfectly and ensures low power consumption.

## What it looks like

As images are always better than long descriptions, here are some screenshots of the interface :

<p align="center">
  <img src="https://raw.githubusercontent.com/coldsource/solarcontrol/refs/heads/main/doc/screenshots/Home.png" alt="Home screen"/>
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/coldsource/solarcontrol/refs/heads/main/doc/screenshots/Stats.png" alt="Montly statistics"/>
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/coldsource/solarcontrol/refs/heads/main/doc/screenshots/HWS.png" alt="Hot Water System"/>
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/coldsource/solarcontrol/refs/heads/main/doc/screenshots/DeviceConfig.png" alt="Device configuration"/>
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/coldsource/solarcontrol/refs/heads/main/doc/screenshots/Settings.png" alt="Settings"/>
</p>
