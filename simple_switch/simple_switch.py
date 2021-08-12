# Mininet topologies for simple_switch
# Inspired by https://github.com/p4lang/tutorials/tree/master/utils/mininet
# Mininet documentation: https://github.com/mininet/mininet/wiki/Documentation

import argparse
from functools import partial

from mininet.cli import CLI
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Host, Switch
from mininet.log import info, setLogLevel


class SimpleSwitch(Switch):
    """Instances of bmv2 simple_switch."""

    def __init__(self, name,
        config=None,
        sw_path = "simple_switch",
        device_id = 0,
        log_file = None,
        log_level = "info",
        thrift_port = 9090,
        debugger = False,
        **kwargs):
        """
        :param config: Path to the JSON configuration file. Starting without a configuration will
            drop all traffic.
        :param sw_path: Switch executeable.
        :param device_id: Device ID of the switch. (--device-id)
        :param log_file: Path to a log file. `None` disabled logging. (--log-file)
        :param log_level: One of 'trace', 'debug', 'info', 'warn', 'error', 'off'. (--log-level)
        :param thrift_port: TCP port the thrift server will listen on. (--thrift-port)
        :param debugger: Enable the debugger. (--debugger)
        """
        Switch.__init__(self, name, **kwargs)

        self.config = config
        self.sw_path = sw_path
        self.device_id = device_id
        self.log_file = log_file
        self.log_level = log_level
        self.thrift_port = thrift_port
        self.debugger = debugger

    def start(self, _controllers):
        """Start the switch.
        :param _controllers_: Unused.
        """
        cmd = [self.sw_path]

        # Only include interfaces without IP addresses to make sure the loopback interface is
        # excluded. Port numbers start at zero.
        for port, intf in enumerate(intf for intf in self.intfs.values() if not intf.IP()):
            cmd.append("-i {}@{}".format(port, intf.name))

        if self.config is not None:
            cmd.append(self.config)
        else:
            cmd.append("--no-p4")

        self._add_cmd_args(cmd)

        info("Starting switch with command line: \"{}\".".format(" ".join(cmd)))
        self.cmd(cmd + ["&"])

    def stop(self):
        """Terminate the switch."""
        self.cmd("kill %" + self.sw_path)
        self.cmd("wait")
        super(SimpleSwitch, self).stop()

    def _add_cmd_args(self, cmd):
        cmd.append("--device-id {}".format(self.device_id))
        cmd.append("--thrift-port {}".format(self.thrift_port))
        if self.debugger:
            cmd.append("--debugger")

        if self.log_file is not None:
            cmd.append("--log-file {}".format(self.log_file))
            cmd.append("--log-level {}".format(self.log_level))
            if self.log_level in ["info", "warn", "error"]:
                # Flush log after every message on less verbose log levels.
                cmd.append("--log-flush")


class SimpleSwitchGrpc(SimpleSwitch):
    """Instances of bmv2 simple_switch_grpc."""

    def __init__(self, name,
        sw_path="simple_switch_grpc",
        grpc_addr="localhost:9559",
        cpu_port=None,
        **kwargs):
        """
        :param grpc_addr: Address of the gRPC server. (--grpc-server-addr)
        :param cpu_port: Index of the CPU port. (--cpu-port)
        """
        super(SimpleSwitchGrpc, self).__init__(name, sw_path=sw_path, **kwargs)

        self.grpc_addr = grpc_addr
        self.cpu_port = cpu_port

    def _add_cmd_args(self, cmd):
        super(SimpleSwitchGrpc, self)._add_cmd_args(cmd)
        cmd.append("--")
        cmd.append("--grpc-server-addr {}".format(self.grpc_addr))
        if self.cpu_port is not None:
            cmd.append("--cpu-port {}".format(self.cpu_port))


class SimpleSwitchHost(Host):
    """Special host for SimpleSwitchTopo topologies."""

    def __init__(self, name, **kwargs):
        super().__init__(name, **kwargs)

        # Make sure UDP/TCP checksums are computed on virtual interfaces.
        self.cmd("iptables -t mangle -A POSTROUTING \\! -o lo -p udp -m udp -j CHECKSUM --checksum-fill")
        self.cmd("iptables -t mangle -A POSTROUTING \\! -o lo -p tcp -m tcp -j CHECKSUM --checksum-fill")


class SingleSwitchTopo(Topo):
    """Topology consisting of a single switch and `n` hosts."""
    def build(self, n=2, config=None, log_file=None, log_level="info"):
        switch = self.addSwitch("s1",
            config=config,
            log_file=log_file,
            log_level=log_level
        )

        for i in range(n):
            host = self.addHost("h%s" % (i + 1))
            self.addLink(host, switch)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", help="Switch configuration file.")
    parser.add_argument("--hosts", default=3,
        help="Number of hosts connected to the switch. (Default: 3)")
    parser.add_argument("--log-file", help="Switch log file.")
    parser.add_argument("--log-level", default="info", help="Switch log level. (Default: 'info')")
    parser.add_argument("--grpc", default=None,
        help="Address of the gRPC server. (Default: Disabled)")
    parser.add_argument("--cpu-port", default=None,
        help="Numeric value of the CPU port. (Default: None)")
    args = parser.parse_args()

    topo = SingleSwitchTopo(n=args.hosts, config=args.config, log_file=args.log_file,
        log_level=args.log_level)

    if args.grpc is None:
        switch = SimpleSwitch
    else:
        switch = partial(SimpleSwitchGrpc, grpc_addr=args.grpc, cpu_port=args.cpu_port)

    net = Mininet(topo=topo, host=SimpleSwitchHost, switch=switch, controller=None)

    net.start()
    CLI(net)
    net.stop()


if __name__ == '__main__':
    setLogLevel('info')
    main()


# Enable use as "--custom" file, e.g., "mn --custom simple_switch.py --topo single,2".
topos = {'single': SingleSwitchTopo}
switches = {'simple_switch': SimpleSwitch, 'simple_switch_grpc': SimpleSwitchGrpc}
hosts = { 'simple_switch_host': SimpleSwitchHost }
