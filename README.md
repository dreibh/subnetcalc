# SubNetCalc
An IPv4/IPv6 Subnet Calculator

## Description

SubNetCalc is an IPv4/IPv6 subnet address calculator. For given IPv4 or IPv6
address and netmask or prefix length, it calculates network address, broadcast
address, maximum number of hosts and host address range. Also, it prints the
addresses in binary format for better understandability. Furthermore, it prints
useful information on specific address types (e.g. type, scope, interface ID,
etc.).

See the manpage of subnetcalc for details!

## Usage Examples

```
subnetcalc 172.31.255.254 255.255.255.0
subnetcalc 172.31.255.254 24
subnetcalc fec0:2345:6789:1111:221:6aff:fe0b:2674 56
subnetcalc www.iem.uni-due.de 24
subnetcalc fd00:: 64 -uniquelocal
subnetcalc fd00::9876:256:7bff:fe1b:3255 56 -uniquelocalhq
```

My host has IP 132.252.150.254 and netmask 255.255.255.240. What are the details of its network?

```
user@host:~$ subnetcalc 132.252.150.254 255.255.255.240
Address       = 132.252.150.254
                   10000100 . 11111100 . 10010110 . 11111110
Network       = 132.252.150.240 / 28
Netmask       = 255.255.255.240
Broadcast     = 132.252.150.255
Wildcard Mask = 0.0.0.15
Hosts Bits    = 4
Max. Hosts    = 14   (2^4 - 2)
Host Range    = { 132.252.150.241 - 132.252.150.254 }
Properties    =
   - 132.252.150.254 is a HOST address in 132.252.150.240/28
   - Class B
GeoIP Country = Germany (DE)
DNS Hostname  = dummy.iem.uni-duisburg-essen.de
```

Consider host www.six.heise.de uses a 64-bit prefix length. What are the details of its network?

```
user@host:~$ subnetcalc www.six.heise.de 64
Address       = 2a02:2e0:3fe:100::6
                   2a02 = 00101010 00000010
                   02e0 = 00000010 11100000
                   03fe = 00000011 11111110
                   0100 = 00000001 00000000
                   0000 = 00000000 00000000
                   0000 = 00000000 00000000
                   0000 = 00000000 00000000
                   0006 = 00000000 00000110
Network       = 2a02:2e0:3fe:100:: / 64
Netmask       = ffff:ffff:ffff:ffff::
Wildcard Mask = ::ffff:ffff:ffff:ffff
Hosts Bits    = 64
Max. Hosts    = 18446744073709551616   (2^64 - 1)
Host Range    = { 2a02:2e0:3fe:100::1 - 2a02:2e0:3fe:100:ffff:ffff:ffff:ffff }
Properties    =
   - 2a02:2e0:3fe:100::6 is a HOST address in 2a02:2e0:3fe:100::/64
   - Global Unicast Properties:
      + Interface ID = 0000:0000:0000:0006
      + Sol. Node MC = ff02::1:ff00:0006
GeoIP Country = Germany (DE)
DNS Hostname  = www.six.heise.de
```

My new host should use Interface ID 0x100 and Subnet ID 0x1234. Generate a Unique Local IPv6 prefix (40-bit Global ID) for my intranet, using high-quality random numbers!

```
user@host:~$ subnetcalc 0:0:0:1234::1 56 -uniquelocalhq
Address       = fded:30d0:1072:1234::1
                   fded = 11111101 11101101
                   30d0 = 00110000 11010000
                   1072 = 00010000 01110010
                   1234 = 00010010 00110100
                   0000 = 00000000 00000000
                   0000 = 00000000 00000000
                   0000 = 00000000 00000000
                   0001 = 00000000 00000001
Network       = fded:30d0:1072:1200:: / 56
Netmask       = ffff:ffff:ffff:ff00::
Wildcard Mask = ::ff:ffff:ffff:ffff:ffff
Hosts Bits    = 72
Max. Hosts    = 4722366482869645213696   (2^72 - 1)
Host Range    = { fded:30d0:1072:1200::1 - fded:30d0:1072:12ff:ffff:ffff:ffff:ffff }
Properties    =
   - fded:30d0:1072:1234::1 is a HOST address in fded:30d0:1072:1200::/56
   - Unique Local Unicast Properties:
      + Locally chose
      + Global ID    = ed30d01072
      + Subnet ID    = 1234
      + Interface ID = 0000:0000:0000:0001
      + Sol. Node MC = ff02::1:ff00:0001
GeoIP Country = Unknown (??)
DNS Hostname  = (Name or service not known)
```

Which are DNS reverse lookup name and geo location country of IP 2401:3800:c001::68?

```
user@host:~$ subnetcalc 2401:3800:c001::68
Address       = 2401:3800:c001::68
                   2401 = 00100100 00000001
                   3800 = 00111000 00000000
                   c001 = 11000000 00000001
                   0000 = 00000000 00000000
                   0000 = 00000000 00000000
                   0000 = 00000000 00000000
                   0000 = 00000000 00000000
                   0068 = 00000000 01101000
Network       = 2401:3800:c001::68 / 128
Netmask       = ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
Wildcard Mask = ::
Hosts Bits    = 0
Max. Hosts    = 0   (2^0 - 1)
Host Range    = { 2401:3800:c001::69 - 2401:3800:c001::68 }
Properties    =
   - 2401:3800:c001::68 is a NETWORK address
   - Global Unicast Properties:
      + Interface ID = 0000:0000:0000:0068
      + Sol. Node MC = ff02::1:ff00:0068
GeoIP Country = China (CN)
DNS Hostname  = bg-in-x68.1e100.net
```

Which are the MAC address and Solicited Node Multicast address of 2001:638:501:4ef8:223:aeff:fea4:8ca9/64?

```
user@host:~$ subnetcalc 2001:638:501:4ef8:223:aeff:fea4:8ca9/64
Address       = 2001:638:501:4ef8:223:aeff:fea4:8ca9
                   2001 = 00100000 00000001
                   0638 = 00000110 00111000
                   0501 = 00000101 00000001
                   4ef8 = 01001110 11111000
                   0223 = 00000010 00100011
                   aeff = 10101110 11111111
                   fea4 = 11111110 10100100
                   8ca9 = 10001100 10101001
Network       = 2001:638:501:4ef8:: / 64
Netmask       = ffff:ffff:ffff:ffff::
Wildcard Mask = ::ffff:ffff:ffff:ffff
Hosts Bits    = 64
Max. Hosts    = 18446744073709551616   (2^64 - 1)
Host Range    = { 2001:638:501:4ef8::1 - 2001:638:501:4ef8:ffff:ffff:ffff:ffff }
Properties    =
   - 2001:638:501:4ef8:223:aeff:fea4:8ca9 is a HOST address in 2001:638:501:4ef8::/64
   - Global Unicast Properties:
      + Interface ID = 0223:aeff:fea4:8ca9
      + MAC address  = 00:23:ae:a4:8c:a9
      + Sol. Node MC = ff02::1:ffa4:8ca9
GeoIP Country = Germany (DE)
DNS Hostname  = (Name or service not known)
```

## Binary Package Installation

Please use the issue tracker at https://github.com/dreibh/subnetcalc/issues to report bugs and issues!

### Ubuntu Linux

For ready-to-install Ubuntu Linux packages of SubNetCalc, see Launchpad PPA for Thomas Dreibholz!

```
sudo apt-add-repository -sy ppa:dreibh/ppa
sudo apt-get update
sudo apt-get install subnetcalc
```

### Fedora Linux

For ready-to-install Fedora Linux packages of SubNetCalc, see COPR PPA for Thomas Dreibholz!

```
sudo dnf copr enable -y dreibh/ppa
sudo dnf install subnetcalc
```

### FreeBSD

For ready-to-install FreeBSD packages of SubNetCalc, it is included in the ports collection, see Index of /head/net/subnetcalc/!

   pkg install subnetcalc

Alternatively, to compile it from the ports sources:

```
cd /usr/ports/net/subnetcalc
make
make install
```

## Sources Download

SubNetCalc is released under the GNU General Public Licence (GPL).

Please use the issue tracker at https://github.com/dreibh/subnetcalc/issues to report bugs and issues!

### Development Version

The Git repository of the SubNetCalc sources can be found at https://github.com/dreibh/subnetcalc:

- Issue tracker: https://github.com/dreibh/subnetcalc/issues.
  Please submit bug reports, issues, questions, etc. in the issue tracker!

- Pull Requests for SubNetCalc: https://github.com/dreibh/subnetcalc/pulls.
  Your contributions to SubNetCalc are always welcome!

- Travis CI automated build tests of SubNetCalc: https://travis-ci.org/dreibh/subnetcalc.

- Coverity Scan analysis of SubNetCalc: https://scan.coverity.com/projects/dreibh-subnetcalc.

### Current Stable Release

The tarball has been signed with my GnuPG key DF605BB0760F2D65. Its authenticity and integrity can be verified by:

gpg --keyserver hkp://keyserver.ubuntu.com --recv-keys DF605BB0760F2D65
gpg --verify subnetcalc-VERSION.tar.xz.asc subnetcalc-VERSION.tar.xz

- [subnetcalc-2.4.14.tar.xz (Tar/XZ file)](https://www.uni-due.de/~be0001/subnetcalc/download/subnetcalc-2.4.14.tar.xz)
- [subnetcalc-2.4.14.tar.xz.asc (Signature)](https://www.uni-due.de/~be0001/subnetcalc/download/subnetcalc-2.4.14.tar.xz.asc)

## Requirements

- Linux or FreeBSD (other OS should work too), GeoIP library and a C++ compiler

