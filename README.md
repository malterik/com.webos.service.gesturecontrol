Summary
-------
Gesture controlled volume control for webos

Description
-----------
Gesture controlled volume control inspired by this quote from the Hitchhiker's Guide to the Galaxy :D

A loud clatter of gunk music flooded through the Heart of Gold cabin as Zaphod searched the sub-etha radio wave bands for news of himself.
The machine was rather difficult to operate. For years radios had been operated by means of pressing buttons and turning dials;
then as the technology became more sophisticated the controls were made touch-sensitive--you merely had to brush the panels with your fingers;
now all you had to do was wave your hand in the general direction of the components and hope.
It saved a lot of muscular expenditure, of course, but meant that you had to sit infuriatingly still if you wanted to keep listening to the same program.

Zaphod waved a hand and the channel switched again.

How to Build on Linux
---------------------

## Dependencies

Below are the tools and libraries (and their minimum versions) required to build sample program:

* cmake (version required by cmake-modules-webos)
* gcc
* glib-2.0
* make
* cmake-modules-webos

## Building

    $ cd build-webos
    $ source oe-init-build-env
    $ bitbake com.example.service.native

Copyright and License Information
=================================
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

SPDX-License-Identifier: Apache-2.0

