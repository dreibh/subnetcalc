<h1 align="center">
 SubNetCalc<br />
 <span style="font-size: 75%">An IPv4/IPv6 Subnet Calculator</span><br />
 <a href="https://www.nntb.no/~dreibh/subnetcalc/">
  <span style="font-size: 75%;">https://www.nntb.no/~dreibh/subnetcalc</span>
 </a>
</h1>


# 💡 What is SubNetCalc?

SubNetCalc is an IPv4/IPv6 subnet address calculator. For given IPv4 or IPv6 address and netmask or prefix length, it calculates network address, broadcast address, maximum number of hosts and host address range. Also, it prints the addresses in binary format for better understandability. Furthermore, it prints useful information on specific address types (e.g. type, scope, interface ID, etc.). In addition to standard formatted console output, SubNetCalc supports machine-readable output formats (JSON, YAML, and XML).

Internationalisation&nbsp;(i18n) is supported using [GNU gettext](https://www.gnu.org/software/gettext/). That is, translation files for the output of the programs are supported. You can support the project by contributing translations for your language. See [Internationalisation&nbsp;(I18N)](#internationalisation) for details!

Address parsing, subnet calculation, property classification, and generation features are implemented according to the following RFCs:

## IPv4 Addressing and Subnetting

* [RFC 791](https://www.rfc-editor.org/rfc/rfc791) (*Internet Protocol*): Classful network identification (`Class A`, `Class B`, `Class C`, `Class D (Multicast)`, and invalid/reserved `Class E`), network address calculation, and 32-bit hexadecimal address formatting.
* [RFC 919](https://www.rfc-editor.org/rfc/rfc919) / [RFC 922](https://www.rfc-editor.org/rfc/rfc922) (*Broadcasting Internet Datagrams in the Presence of Subnets*): Directed subnet broadcast address calculation and identification.
* [RFC 950](https://www.rfc-editor.org/rfc/rfc950) / [RFC 4632](https://www.rfc-editor.org/rfc/rfc4632) (*Subnetting and Classless Inter-Domain Routing (CIDR)*): Arbitrary prefix lengths (`/0`–`/32`), subnet masks, wildcard (inverted) masks, host bit counts, maximum usable host counts, and usable host address ranges.
* [RFC 1122](https://www.rfc-editor.org/rfc/rfc1122) / [RFC 6890](https://www.rfc-editor.org/rfc/rfc6890) (*Host Requirements and Special-Purpose IP Address Registries*): Identification of the IPv4 loopback host (`127.0.0.1`) and loopback network (`127.0.0.0/8`), default route (`0.0.0.0/0`), and limited broadcast (`255.255.255.255`).
* [RFC 1918](https://www.rfc-editor.org/rfc/rfc1918) (*Address Allocation for Private Internets*): Identification of IPv4 private address spaces (`10.0.0.0/8`, `172.16.0.0/12`, and `192.168.0.0/16`).
* [RFC 3021](https://www.rfc-editor.org/rfc/rfc3021) (*Using 31-Bit Prefixes on IPv4 Point-to-Point Links*): Point-to-point `/31` handling (as well as `/32` host routes), omitting the broadcast address and treating both endpoints as usable hosts (`2^1 - 0 = 2`).
* [RFC 3927](https://www.rfc-editor.org/rfc/rfc3927) (*Dynamic Configuration of IPv4 Link-Local Addresses*): Identification of IPv4 link-local addresses in `169.254.0.0/16`.

## IPv6 Addressing and Subnetting

* [RFC 4291](https://www.rfc-editor.org/rfc/rfc4291) (*IP Version 6 Addressing Architecture*):
  * Arbitrary CIDR prefix lengths (`/0`–`/128`), zero-padded expanded address formatting, binary bit breakdown, and host range calculation.
  * Special address identification: Unspecified (`::`), Loopback (`::1`), IPv4-compatible (`::x.x.x.x`), and IPv4-mapped (`::ffff:x.x.x.x`).
  * Unicast scope classification: Link-Local Unicast (`fe80::/10`), Site-Local Unicast (`fec0::/10`, deprecated by [RFC 3879](https://www.rfc-editor.org/rfc/rfc3879)), and Global Unicast (`2000::/3`).
  * Modified EUI-64 Interface Identifier decoding (`ff:fe` detection and universal/local bit inversion) to extract the underlying 48-bit IEEE 802 MAC address.
  * Solicited-Node Multicast Address (`ff02::1:ffXX:XXXX`) derivation for unicast addresses, and target node mask (`xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxXX:XXXX`) extraction when inspecting `ff02::1:ff00:0/104`.
* [RFC 5952](https://www.rfc-editor.org/rfc/rfc5952) (*A Recommendation for IPv6 Address Text Representation*): Canonical IPv6 text compression for addresses and masks (shortening the longest run of consecutive 16-bit zero fields while never compressing a single zero field to `::`).
* [RFC 4193](https://www.rfc-editor.org/rfc/rfc4193) (*Unique Local IPv6 Unicast Addresses (ULA)*):
  * Classification of `fc00::/7` addresses into locally chosen (`fd00::/8`, `L=1`) and globally assigned (`fc00::/8`, `L=0`).
  * Extraction and display of the 40-bit Global ID and 16-bit Subnet ID.
  * Random ULA `/48` prefix generation (`-u` / `--uniquelocal` via `/dev/urandom` and `-U` / `--uniquelocalhq` via `/dev/random`).
* [RFC 6164](https://www.rfc-editor.org/rfc/rfc6164) (*Using 127-Bit IPv6 Prefixes on Inter-Router Links*): Point-to-point `/127` handling (as well as `/128` host routes), disabling Subnet-Router anycast reservation so both endpoints are usable hosts (`2^1 - 0 = 2`).

## Multicast Addressing

* [RFC 1112](https://www.rfc-editor.org/rfc/rfc1112) / [RFC 2365](https://www.rfc-editor.org/rfc/rfc2365) / [RFC 5771](https://www.rfc-editor.org/rfc/rfc5771) (*IPv4 Multicast Addressing and Administrative Scoping*): Classification of IPv4 Class D multicast (`224.0.0.0/4`) scopes (`link-local` in `224.0.0.0/8`, `organization-local` in `239.192.0.0/14`, `site-local` in `239.252.0.0/14`, and `global`) and mapping of the lower 23 bits to Ethernet multicast MAC addresses (`01:00:5e:xx:xx:xx`).
* [RFC 4291](https://www.rfc-editor.org/rfc/rfc4291) / [RFC 7346](https://www.rfc-editor.org/rfc/rfc7346) (*IPv6 Multicast Addressing and Scopes*): Classification of IPv6 multicast (`ff00::/8`) scopes (`node-local`, `link-local`, `site-local`, `organization-local`, and `global`) and temporarily-allocated (transient) flag detection (`ff1x::/16`).
* [RFC 2464](https://www.rfc-editor.org/rfc/rfc2464) (*Transmission of IPv6 Packets over Ethernet Networks*): Mapping of the lower 32 bits of an IPv6 multicast address to its Ethernet multicast MAC address (`33:33:xx:xx:xx:xx`).
* [RFC 3306](https://www.rfc-editor.org/rfc/rfc3306) / [RFC 4607](https://www.rfc-editor.org/rfc/rfc4607) (*Source-Specific Multicast (SSM)*): Identification of IPv4 SSM (`232.0.0.0/8`) and IPv6 SSM (`ff3x::/32`) addresses.

## Transition and Translation Mechanisms

* [RFC 3056](https://www.rfc-editor.org/rfc/rfc3056) (*Connection of IPv6 Domains via IPv4 Clouds — 6to4*): Detection of `2002::/16` addresses and extraction of the embedded 32-bit IPv4 gateway address.
* [RFC 6052](https://www.rfc-editor.org/rfc/rfc6052) / [RFC 8215](https://www.rfc-editor.org/rfc/rfc8215) (*IPv6 Addressing of IPv4/IPv6 Translators — NAT64*): Recognition of IPv4-embedded IPv6 addresses under the Well-Known Prefix (`64:ff9b::/96`) and Local-Use translation prefix (`64:ff9b:1::/48`), formatting the embedded lower 32 bits in dotted-decimal IPv4 notation.

## Hostnames, IDN and Literal Address Syntax

* [RFC 2732](https://www.rfc-editor.org/rfc/rfc2732) (*Format for Literal IPv6 Addresses in URLs*): Parsing of bracketed IPv6 literal addresses (`[address]`).
* [RFC 1035](https://www.rfc-editor.org/rfc/rfc1035) / [RFC 3596](https://www.rfc-editor.org/rfc/rfc3596) / [RFC 5890](https://www.rfc-editor.org/rfc/rfc5890) / [RFC 5891](https://www.rfc-editor.org/rfc/rfc5891) (*DNS and Internationalized Domain Names — IDNA2008*): Forward (`A` and `AAAA`) and reverse (`PTR`) DNS resolution with Unicode IDN and Punycode (`xn--`) conversion.


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
  Host Bits      = 4
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
  Exp. Address   = 2a02:02e0:03fe:1001:7777:772e:0002:0085
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
  Host Bits      = 64
  Max. Hosts     = 18446744073709551615   (2^64 - 1)
  Host Range     = { 2a02:2e0:3fe:1001::1 - 2a02:2e0:3fe:1001:ffff:ffff:ffff:ffff }
  Properties     =
     - 2a02:2e0:3fe:1001:7777:772e:2:85 is a HOST address in 2a02:2e0:3fe:1001::/64
     - Global Unicast Properties:
        + Interface ID                     = 7777:772e:0002:0085
        + Solicited Node Multicast Address = <span style="color: green">ff02::1:ff</span>02:0085
        + 6-to-4 Address                   = 2.224.3.254
  GeoIP AS Info  = Plus.line AG (AS12306)
  GeoIP Country  = Germany (DE)
  DNS Hostname   = www.heise.de
  </code></pre>

* My new host should use Interface ID 0x100 and Subnet ID 0x1234. Generate a Unique Local IPv6 prefix (40-bit Global ID) for my intranet, according to [RFC&nbsp;4193](https://www.rfc-editor.org/rfc/rfc4193), using high quality random numbers!

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 0:0:0:1234::100 64 -uniquelocalhq</strong>
  Generating Unique Local IPv6 address (using /dev/random) ...
  Address        = fd22:6892:1a2d:1234::100
  Exp. Address   = fd22:6892:1a2d:1234:0000:0000:0000:0100
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
  Host Bits      = 64
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
  Exp. Address   = 2401:3800:c001:0000:0000:0000:0000:0068
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
  Host Bits      = 0
  Max. Hosts     = 0   (2^0 - 1)
  Host Range     = { 2401:3800:c001::69 - 2401:3800:c001::68 }
  Properties     =
     - 2401:3800:c001::68 is a NETWORK address
     - Global Unicast Properties:
        + Interface ID                     = 0000:0000:0000:0068
        + Solicited Node Multicast Address = ff02::1:ff00:0068
  GeoIP Country  = <strong>China (CN)</strong>
  DNS Hostname   = <strong>bg-in-x68.1e100.net</strong>
  </code></pre>

* Which are the MAC address and Solicited Node Multicast address of 2001:638:501:4ef8:223:aeff:fea4:8ca9/64?

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 2001:638:501:4ef8:223:aeff:fea4:8ca9/64</strong>
  Address        = 2001:638:501:4ef8:223:aeff:fea4:8ca9
  Exp. Address   = 2001:0638:0501:4ef8:0223:aeff:fea4:8ca9
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
  Host Bits      = 64
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

* Need machine-readable output for scripts or pipelines? Use `-F json` to get clean JSON output:

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 192.168.1.1/24 -F json -n -g</strong>
  {
    "address": "192.168.1.1",
    "family": "IPv4",
    "network": "192.168.1.0",
    "prefix": 24,
    "netmask": "255.255.255.0",
    "broadcast": "192.168.1.255",
    "wildcard": "0.0.0.255",
    "hex": "C0A80101",
    "host_bits": 8,
    "max_hosts": 254,
    "host_range": {
      "min": "192.168.1.1",
      "max": "192.168.1.254"
    },
    "properties": {
      "type": "host",
      "description": "192.168.1.1 is a HOST address in 192.168.1.0/24",
      "class": "Class C",
      "flags": [
        "Private"
      ]
    }
  }
  </code></pre>

* JSON output is also available for IPv6 with expanded address, GeoIP ASN details, and full property breakdown:

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 2a02:2e0:3fe:1001:7777:772e:2:85 64 -F json</strong>
  {
    "address": "2a02:2e0:3fe:1001:7777:772e:2:85",
    "expanded_address": "2a02:02e0:03fe:1001:7777:772e:0002:0085",
    "family": "IPv6",
    "network": "2a02:2e0:3fe:1001::",
    "prefix": 64,
    "netmask": "ffff:ffff:ffff:ffff::",
    "wildcard": "::ffff:ffff:ffff:ffff",
    "host_bits": 64,
    "max_hosts": 18446744073709551615,
    "host_range": {
      "min": "2a02:2e0:3fe:1001::1",
      "max": "2a02:2e0:3fe:1001:ffff:ffff:ffff:ffff"
    },
    "properties": {
      "type": "host",
      "description": "2a02:2e0:3fe:1001:7777:772e:2:85 is a HOST address in 2a02:2e0:3fe:1001::/64",
      "unicast_type": "Global Unicast",
      "interface_id": "7777:772e:0002:0085",
      "solicited_node_multicast": "ff02::1:ff02:0085",
      "six_to_four": "2.224.3.254"
    },
    "geoip": {
      "asn": 12306,
      "as_organization": "Plus.line AG",
      "country": "Germany",
      "country_code": "DE",
      "postal_code": "63263",
      "city": "Neu-Isenburg",
      "region": "Hesse",
      "timezone": "Europe/Berlin",
      "latitude": 50.0596,
      "longitude": 8.6862
    },
    "dns": {
      "hostname": "www.heise.de"
    }
  }
  </code></pre>

* YAML (`-F yaml`) and XML (`-F xml`) are also supported:

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 2001:db8::1/64 -F yaml -n -g</strong>
  address: "2001:db8::1"
  expanded_address: "2001:0db8:0000:0000:0000:0000:0000:0001"
  family: "IPv6"
  network: "2001:db8::"
  prefix: 64
  netmask: "ffff:ffff:ffff:ffff::"
  wildcard: "::ffff:ffff:ffff:ffff"
  host_bits: 64
  max_hosts: 18446744073709551615
  host_range:
    min: "2001:db8::1"
    max: "2001:db8::ffff:ffff:ffff:ffff"
  properties:
    type: "host"
    description: "2001:db8::1 is a HOST address in 2001:db8::/64"
    unicast_type: "Global Unicast"
    interface_id: "0000:0000:0000:0001"
    solicited_node_multicast: "ff02::1:ff00:0001"
  </code></pre>

  <pre><code><span style="color:green;">user@host</span><span style="color:blue;">:~</span><span style="color:gray;">$</span> <strong><span class="fu">subnetcalc</span> 2001:db8::1/64 -F xml -n -g</strong>
  &lt;?xml version="1.0" encoding="UTF-8"?&gt;
  &lt;subnetcalc&gt;
    &lt;address&gt;2001:db8::1&lt;/address&gt;
    &lt;expanded_address&gt;2001:0db8:0000:0000:0000:0000:0000:0001&lt;/expanded_address&gt;
    &lt;family&gt;IPv6&lt;/family&gt;
    &lt;network&gt;2001:db8::&lt;/network&gt;
    &lt;prefix&gt;64&lt;/prefix&gt;
    &lt;netmask&gt;ffff:ffff:ffff:ffff::&lt;/netmask&gt;
    &lt;wildcard&gt;::ffff:ffff:ffff:ffff&lt;/wildcard&gt;
    &lt;host_bits&gt;64&lt;/host_bits&gt;
    &lt;max_hosts&gt;18446744073709551615&lt;/max_hosts&gt;
    &lt;host_range&gt;
      &lt;min&gt;2001:db8::1&lt;/min&gt;
      &lt;max&gt;2001:db8::ffff:ffff:ffff:ffff&lt;/max&gt;
    &lt;/host_range&gt;
    &lt;properties&gt;
      &lt;type&gt;host&lt;/type&gt;
      &lt;description&gt;2001:db8::1 is a HOST address in 2001:db8::/64&lt;/description&gt;
      &lt;unicast_type&gt;Global Unicast&lt;/unicast_type&gt;
      &lt;interface_id&gt;0000:0000:0000:0001&lt;/interface_id&gt;
      &lt;solicited_node_multicast&gt;ff02::1:ff00:0001&lt;/solicited_node_multicast&gt;
    &lt;/properties&gt;
  &lt;/subnetcalc&gt;
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
git clone [https://github.com/dreibh/subnetcalc](https://github.com/dreibh/subnetcalc)
cd subnetcalc
sudo ci/get-dependencies --install
cmake .
make
```

Optionally, for installation to the standard paths (usually under `/usr/local`):

```bash
sudo make install
```

Note: The script [`ci/get-dependencies`](https://github.com/dreibh/subnetcalc/blob/master/ci/get-dependencies) automatically installs the build dependencies under Debian/Ubuntu Linux, Fedora Linux, and FreeBSD. For manual handling of the build dependencies, see the packaging configuration in [`debian/control`](https://github.com/dreibh/subnetcalc/blob/master/debian/control) (Debian/Ubuntu Linux), [`subnetcalc.spec`](https://github.com/dreibh/subnetcalc/blob/master/rpm/subnetcalc.spec) (Fedora Linux), and [`Makefile`](https://github.com/dreibh/subnetcalc/blob/master/freebsd/subnetcalc/Makefile) FreeBSD.

Contributions:

* Issue tracker: [https://github.com/dreibh/subnetcalc/issues](https://github.com/dreibh/subnetcalc/issues).
  Please submit bug reports, issues, questions, etc. in the issue tracker!

* Pull Requests for SubNetCalc: [https://github.com/dreibh/subnetcalc/pulls](https://github.com/dreibh/subnetcalc/pulls).
  Your contributions to SubNetCalc are always welcome!

* CI build tests of SubNetCalc: [https://github.com/dreibh/subnetcalc/actions](https://github.com/dreibh/subnetcalc/actions).

* Coverity Scan analysis of SubNetCalc: [https://scan.coverity.com/projects/dreibh-subnetcalc](https://scan.coverity.com/projects/dreibh-subnetcalc).

## Release Versions

See [https://www.nntb.no/~dreibh/subnetcalc/#current-stable-release](https://www.nntb.no/~dreibh/subnetcalc/#current-stable-release) for the release packages!


# 🌏 Internationalisation

To provide a translation of one or more components of NetPerfMeter into your language, apply the following steps:

1. Build NetPerfMeter from the Git sources (see [Development Version](#development-version)), i.e., use the "master" branch with the latest development version. The build will create `.pot` (translation template files) under [`po`](po).

2. Create a new Git branch for your translations, e.g., `my_username/translations_language_XX` (where `XX` is the language code for your language, e.g., `da` for Danish):

   ```bash
   git branch my_username/translations_language_XX
   git checkout my_username/translations_language_XX
   ```

3. Take a look at the existing `.po` files (translation files) in [`po/de`](po/de) (German) and [`po/nb`](po/nb) (Bokmål) as examples, e.g., [`po/de/netperfmeter.po`](po/de/netperfmeter.po) or [`po/nb/netperfmeter.po`](po/nb/netperfmeter.po) for netperfmeter. Then, prepare a translation for program `PROGRAM` (e.g., netperfmeter; see the name of the `.pot` translation template file) for your language `XX` under [`po`](po):

   ```bash
   mkdir -p XX
   msginit --input PROGRAM.pot \
           --locale XX.UTF-8 \
           --output-file XX/PROGRAM.po
   git add XX/PROGRAM.po
   ```

4. Add your translations into `XX/PROGRAM.po`, with a text editor or PO file editor like [Poedit](https://poedit.com/).

5. Test, commit and push your changes.

6. Finally, make a pull request: [https://github.com/dreibh/netperfmeter/pulls](https://github.com/dreibh/netperfmeter/pulls).


# 🔗 Useful Links

* [NetPerfMeter – A TCP/MPTCP/UDP/SCTP/DCCP Network Performance Meter Tool](https://www.nntb.no/~dreibh/netperfmeter/)
* [HiPerConTracer – High-Performance Connectivity Tracer](https://www.nntb.no/~dreibh/hipercontracer/)
* [System-Tools – Tools for Basic System Management](https://www.nntb.no/~dreibh/system-tools/)
* [Wireshark](https://www.wireshark.org/)
* [Linux Weekly News](https://lwn.net)
* [Linux.org](https://www.linux.org)
