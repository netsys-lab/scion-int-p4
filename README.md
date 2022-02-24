Experiments with P4, SCION and In-Band Network Telemetry
========================================================


Development Environment Setup
-----------------------------
Install Docker ([install_docker.sh](VM/install_docker.sh)) and run all commands in
[setup.sh](VM/setup.sh) on a fresh installation of Ubuntu Server 20.04.2 LTS.

Alternatively, a development VM can be created using [Vagrant](https://www.vagrantup.com/).
Customize the [Vagrantfile](VM/Vagrantfile) to your needs and run `vagrant up` in the `VM` directory.


Contents of this Repository
---------------------------
- [Data planes and controllers for the BMv2 simple_switch](simple_switch/)
- [Dockerized SCION topologies with BMv2 switches](scion/)
- [In-band Network Telemetry report format for SCION](telemetry/kafka)
