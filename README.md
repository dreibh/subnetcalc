<h1 align="center">
 SubNetCalc<br />
 <span style="font-size: 75%">An IPv4/IPv6 Subnet Calculator</span><br />
 <a href="https://www.nntb.no/~dreibh/subnetcalc/">
  <span style="font-size: 75%;">https://www.nntb.no/~dreibh/subnetcalc</span>
 </a>
</h1>


# 💡 What is SubNetCalc?

SubNetCalc is an IPv4/IPv6 subnet address calculator. For given IPv4 or IPv6 address and netmask or prefix length, it calculates network address, broadcast address, maximum number of hosts and host address range. Also, it prints the addresses in binary format for better understandability. Furthermore, it prints useful information on specific address types (e.g. type, scope, interface ID, etc.).


# 😀 Examples

* My host has IP 132.252.150.154 and netmask 255.255.255.240. What are the details of its network?

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 132.252.150.154 255.255.255.240</strong>
  Address        = 132.252.150.154
                      <span style="color: red;">10000100 . 11111100 . 10010110 . 1001</span><span style="color: blue;">1010</span>
  Network        = 132.252.150.144 / 28
  Netmask        = 255.255.255.240
  Broadcast      = 132.252.150.159
  Wildcard Mask  = 0.0.0.15
  Hex. Address   = 84FC969A
  Hosts Bits     = 4
  Max. Hosts     = 14   (2^4 - 2)
  Host Range     = { 132.252.150.145 - 132.252.150.158 }
  Properties     =
     - 132.252.150.154 is a HOST address in 132.252.150.144/28
     - Class B
  GeoIP Country  = Germany (DE)
  DNS Hostname   = lupo.iem.uni-due.de
  </code></pre>

* Consider host [www.heise.de](https://www.heise.de) uses a 64-bit prefix length. What are the details of its network?

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> www.heise.de 64</strong>
  Address        = 2a02:2e0:3fe:1001:7777:772e:2:85
                      2a02 = <span style="color: red;">00101010 00000010</span>
                      02e0 = <span style="color: red;">00000010 11100000</span>
                      03fe = <span style="color: red;">00000011 11111110</span>
                      1001 = <span style="color: red;">00010000 00000001</span>
                      7777 = <span style="color: blue;">01110111 01110111</span>
                      772e = <span style="color: blue;">01110111 00101110</span>
                      0002 = <span style="color: blue;">00000000 00000010</span>
                      0085 = <span style="color: blue;">00000000 10000101</span>
  Network        = 2a02:2e0:3fe:1001:: / 64
  Netmask        = ffff:ffff:ffff:ffff::
  Wildcard Mask  = ::ffff:ffff:ffff:ffff
  Hosts Bits     = 64
  Max. Hosts     = 18446744073709551615   (2^64 - 1)
  Host Range     = { 2a02:2e0:3fe:1001::1 - 2a02:2e0:3fe:1001:ffff:ffff:ffff:ffff }
  Properties     =
     - 2a02:2e0:3fe:1001:7777:772e:2:85 is a HOST address in 2a02:2e0:3fe:1001::/64
     - Global Unicast Properties:
        + Interface ID                     = 7777:772e:0002:0085
        + Solicited Node Multicast Address = <span style="color: green">ff02::1:ff</span>02:0085
        + 6-to-4 Address                   = 2.224.3.254
  GeoIP Country  = Germany (DE)
  DNS Hostname   = www.heise.de
  </code></pre>

* My new host should use Interface ID 0x100 and Subnet ID 0x1234. Generate a Unique Local IPv6 prefix (40-bit Global ID) for my intranet, according to [RFC&nbsp;4193](https://www.rfc-editor.org/rfc/rfc4193), using high quality random numbers!

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 0:0:0:1234::100 64 -uniquelocalhq</strong>
  Generating Unique Local IPv6 address (using /dev/random) ...
  Address        = fd22:6892:1a2d:1234::100
                      fd22 = <span style="color: red;">11111101 00100010</span>
                      6892 = <span style="color: red;">01101000 10010010</span>
                      1a2d = <span style="color: red;">00011010 00101101</span>
                      1234 = <span style="color: red;">00010010 00110100</span>
                      0000 = <span style="color: blue;">00000000 00000000</span>
                      0000 = <span style="color: blue;">00000000 00000000</span>
                      0000 = <span style="color: blue;">00000000 00000000</span>
                      0100 = <span style="color: blue;">00000001 00000000</span>
  Network        = fd22:6892:1a2d:1234:: / 64
  Netmask        = ffff:ffff:ffff:ffff::
  Wildcard Mask  = ::ffff:ffff:ffff:ffff
  Hosts Bits     = 64
  Max. Hosts     = 18446744073709551615   (2^64 - 1)
  Host Range     = { fd22:6892:1a2d:1234::1 - fd22:6892:1a2d:1234:ffff:ffff:ffff:ffff }
  Properties     =
     - fd22:6892:1a2d:1234::100 is a HOST address in fd22:6892:1a2d:1234::/64
     - Unique Local Unicast Properties:
        + Locally chosen
        + Global ID                        = <strong>2268921a2d</strong>
        + Subnet ID                        = 1234
        + Interface ID                     = 0000:0000:0000:0100
        + Solicited Node Multicast Address = <span style="color: green">ff02::1:ff</span>00:0100
  GeoIP Country  = Unknown (??)
  DNS Hostname   = (Name or service not known)
  </code></pre>

* Which are DNS reverse lookup name and geo-location country of IP 2401:3800:c001::68?

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 2401:3800:c001::68</strong>
  Address        = 2401:3800:c001::68
                      2401 = <span style="color: red;">00100100 00000001</span>
                      3800 = <span style="color: red;">00111000 00000000</span>
                      c001 = <span style="color: red;">11000000 00000001</span>
                      0000 = <span style="color: red;">00000000 00000000</span>
                      0000 = <span style="color: red;">00000000 00000000</span>
                      0000 = <span style="color: red;">00000000 00000000</span>
                      0000 = <span style="color: red;">00000000 00000000</span>
                      0068 = <span style="color: red;">00000000 01101000</span>
  Network        = 2401:3800:c001::68 / 128
  Netmask        = ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
  Wildcard Mask  = ::
  Hosts Bits     = 0
  Max. Hosts     = 0   (2^0 - 1)
  Host Range     = { 2401:3800:c001::69 - 2401:3800:c001::68 }
  Properties     =
     - 2401:3800:c001::68 is a NETWORK address
   - Global Unicast Properties:
      + Interface ID                     = 0000:0000:0000:0068
      + Solicited Node Multicast Address = ff02::1:ff00:0068
  GeoIP Country = <strong>China (CN)</strong>
  DNS Hostname  = <strong>bg-in-x68.1e100.net</strong>
  </code></pre>

* Which are the MAC address and Solicited Node Multicast address of 2001:638:501:4ef8:223:aeff:fea4:8ca9/64?

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 2001:638:501:4ef8:223:aeff:fea4:8ca9/64</strong>
  Address        = 2001:638:501:4ef8:223:aeff:fea4:8ca9
                     2001 = <span style="color: red;">00100000 00000001</span>
                     0638 = <span style="color: red;">00000110 00111000</span>
                     0501 = <span style="color: red;">00000101 00000001</span>
                     4ef8 = <span style="color: red;">01001110 11111000</span>
                     0223 = <span style="color: blue;">00000010 00100011</span>
                     aeff = <span style="color: blue;">10101110 11111111</span>
                     fea4 = <span style="color: blue;">11111110 10100100</span>
                     8ca9 = <span style="color: blue;">10001100 10101001</span>
  Network        = 2001:638:501:4ef8:: / 64
  Netmask        = ffff:ffff:ffff:ffff::
  Wildcard Mask  = ::ffff:ffff:ffff:ffff
  Hosts Bits     = 64
  Max. Hosts     = 18446744073709551615   (2^64 - 1)
  Host Range     = { 2001:638:501:4ef8::1 - 2001:638:501:4ef8:ffff:ffff:ffff:ffff }
  Properties     =
     - 2001:638:501:4ef8:223:aeff:fea4:8ca9 is a HOST address in 2001:638:501:4ef8::/64
     - Global Unicast Properties:
        + Interface ID                     = <span style="color: cyan;">0223:ae</span><span style="color: gray">ff:fe</span>a4:8ca9
        + MAC Address                      = <strong><span style="color: cyan;">00:23:ae</span>:a4:8c:a9</strong>
        + Solicited Node Multicast Address = <strong><span style="color: green">ff02::1:ff</span>a4:8ca9</strong>
  GeoIP Country  = Germany (DE)
  DNS Hostname   = (Name or service not known)
  </code></pre>

* Take a look into the manual page of SubNetCalc for further information and options:

  ```bash
  man subnetcalc
  ```


# 📦 Binary Package Installation

Please use the issue tracker at [https://github.com/dreibh/subnetcalc/issues](https://github.com/dreibh/subnetcalc/issues) to report bugs and issues!

## Ubuntu Linux

For ready-to-install Ubuntu Linux packages of SubNetCalc, see [Launchpad PPA for Thomas Dreibholz](https://launchpad.net/~dreibh/+archive/ubuntu/ppa/+packages?field.name_filter=subnetcalc&field.status_filter=published&field.series_filter=)!

```bash
sudo apt-add-repository -sy ppa:dreibh/ppa
sudo apt-get update
sudo apt-get install subnetcalc
```

## Fedora Linux

For ready-to-install Fedora Linux packages of SubNetCalc, see [COPR PPA for Thomas Dreibholz](https://copr.fedorainfracloud.org/coprs/dreibh/ppa/package/subnetcalc/)!

```bash
sudo dnf copr enable -y dreibh/ppa
sudo dnf install subnetcalc
```

## FreeBSD

For ready-to-install FreeBSD packages of SubNetCalc, it is included in the ports collection, see [FreeBSD ports tree index of net/subnetcalc/](https://cgit.freebsd.org/ports/tree/net/subnetcalc/)!

```bash
sudo pkg install subnetcalc
```

Alternatively, to compile it from the ports sources:

```bash
cd /usr/ports/net/subnetcalc
make
sudo make install
```


# 💾 Build from Sources

SubNetCalc is released under the [GNU General Public Licence&nbsp;(GPL)](https://www.gnu.org/licenses/gpl-3.0.en.html#license-text).

Please use the issue tracker at [https://github.com/dreibh/subnetcalc/issues](https://github.com/dreibh/subnetcalc/issues) to report bugs and issues!

## Development Version

The Git repository of the SubNetCalc sources can be found at [https://github.com/dreibh/subnetcalc](https://github.com/dreibh/subnetcalc):

```bash
git clone https://github.com/dreibh/subnetcalc
cd subnetcalc
sudo ci/get-dependencies --install
cmake .
make
```

Optionally, for installation to the standard paths (usually under `/usr/local`):

```bash
sudo make install
```

Note: The script [`ci/get-dependencies`](https://github.com/dreibh/subnetcalc/blob/master/ci/get-dependencies) automatically  installs the build dependencies under Debian/Ubuntu Linux, Fedora Linux, and FreeBSD. For manual handling of the build dependencies, see the packaging configuration in [`debian/control`](https://github.com/dreibh/subnetcalc/blob/master/debian/control) (Debian/Ubuntu Linux), [`subnetcalc.spec`](https://github.com/dreibh/subnetcalc/blob/master/rpm/subnetcalc.spec) (Fedora Linux), and [`Makefile`](https://github.com/dreibh/subnetcalc/blob/master/freebsd/subnetcalc/Makefile) FreeBSD.

Contributions:

* Issue tracker: [https://github.com/dreibh/subnetcalc/issues](https://github.com/dreibh/subnetcalc/issues).
  Please submit bug reports, issues, questions, etc. in the issue tracker!

* Pull Requests for SubNetCalc: [https://github.com/dreibh/subnetcalc/pulls](https://github.com/dreibh/subnetcalc/pulls).
  Your contributions to SubNetCalc are always welcome!

* CI build tests of SubNetCalc: [https://github.com/dreibh/subnetcalc/actions](https://github.com/dreibh/subnetcalc/actions).

* Coverity Scan analysis of SubNetCalc: [https://scan.coverity.com/projects/dreibh-subnetcalc](https://scan.coverity.com/projects/dreibh-subnetcalc).

## Release Versions

See [https://www.nntb.no/~dreibh/subnetcalc/#current-stable-release](https://www.nntb.no/~dreibh/subnetcalc/#current-stable-release) for the release packages!


# 🔗 Useful Links

* [NetPerfMeter – A TCP/MPTCP/UDP/SCTP/DCCP Network Performance Meter Tool](https://www.nntb.no/~dreibh/netperfmeter/)
* [HiPerConTracer – High-Performance Connectivity Tracer](https://www.nntb.no/~dreibh/hipercontracer/)
* [System-Tools – Tools for Basic System Management](https://www.nntb.no/~dreibh/system-tools/)
* [Wireshark](https://www.wireshark.org/)
* [Linux Weekly News](https://lwn.net)
* [Linux.org](https://www.linux.org)
