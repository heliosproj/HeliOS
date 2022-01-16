
![HeliOS](/extras/HeliOS_OG_Logo.png)


[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/MannyPeterson/HeliOS/blob/master/LICENSE.md) ![GitHub last commit](https://img.shields.io/github/last-commit/MannyPeterson/HeliOS) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/MannyPeterson/HeliOS) [![arduino-library-badge](https://www.ardu-badge.com/badge/HeliOS.svg?)](https://www.ardu-badge.com/HeliOS) ![GitHub stars](https://img.shields.io/github/stars/MannyPeterson/HeliOS?style=social) ![GitHub watchers](https://img.shields.io/github/watchers/MannyPeterson/HeliOS?style=social)
# Another Example
This example contains code that run without HeliOS and an equivalent example using HeliOS.

There is also code to run a simulated version of a Arduino Nano, so you can see
the difference between the two methods and see why HeliOS works better.

Granted, you can achieve a similar result without HeliOS for such a simple
example but when things get more complicated you need an RTOS solution.

Note also that the standard code does not compile when it is in the HeliOS
directory. It will fail to find the correct libraries.
